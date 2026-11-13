/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkRestJob.h>

#include <mitkErrorResponse.h>
#include <mitkLexicalCast.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <limits>

namespace
{
  constexpr double MaxWaitSeconds = 120.0;
  constexpr int MaxConcurrentWaits = 4;

  std::atomic<int> WaitingRequests{ 0 };

  /** Takes one of the places to wait for a job, if one is free, and gives it back at the end of its life. */
  class WaitPlace
  {
  public:
    WaitPlace() : m_Taken(WaitingRequests.fetch_add(1) < MaxConcurrentWaits)
    {
      if (!m_Taken)
        WaitingRequests.fetch_sub(1);
    }

    ~WaitPlace()
    {
      if (m_Taken)
        WaitingRequests.fetch_sub(1);
    }

    WaitPlace(const WaitPlace&) = delete;
    WaitPlace& operator=(const WaitPlace&) = delete;

    bool IsTaken() const
    {
      return m_Taken;
    }

  private:
    const bool m_Taken;
  };

  std::string GetStateName(mitk::RestJob::State state)
  {
    switch (state)
    {
      case mitk::RestJob::State::Running:
        return "running";
      case mitk::RestJob::State::Succeeded:
        return "succeeded";
      case mitk::RestJob::State::Failed:
        return "failed";
      case mitk::RestJob::State::Canceled:
        return "canceled";
    }

    return "unknown";
  }
}

mitk::RestJob::RestJob(const std::string& id, const std::string& description)
  : m_Id(id),
    m_Description(description),
    m_Start(std::chrono::steady_clock::now()),
    m_State(State::Running),
    m_WaitsInterrupted(false)
{
}

std::string mitk::RestJob::GetId() const
{
  return m_Id;
}

mitk::RestJob::State mitk::RestJob::GetState() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return m_State;
}

bool mitk::RestJob::HasEnded() const
{
  return this->GetState() != State::Running;
}

void mitk::RestJob::Succeed(const nlohmann::json& result)
{
  this->End(State::Succeeded, result, std::string());
}

void mitk::RestJob::Fail(const std::string& message)
{
  this->End(State::Failed, nlohmann::json(), message);
}

void mitk::RestJob::SetCancelHandler(std::function<void()> handler)
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_CancelHandler = std::move(handler);
}

bool mitk::RestJob::Cancel()
{
  std::function<void()> handler;

  {
    std::lock_guard<std::mutex> lock(m_Mutex);

    if (m_State != State::Running || !m_CancelHandler)
      return false;

    handler = m_CancelHandler;
  }

  // Outside the lock, as the handler may end the job itself.
  handler();

  this->End(State::Canceled, nlohmann::json(), std::string());
  return true;
}

bool mitk::RestJob::WaitUntilEnded(std::chrono::milliseconds timeout) const
{
  std::unique_lock<std::mutex> lock(m_Mutex);
  m_Ended.wait_for(lock, timeout, [this]() { return m_State != State::Running || m_WaitsInterrupted; });
  return m_State != State::Running;
}

void mitk::RestJob::SetWaitsInterrupted(bool interrupted)
{
  {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_WaitsInterrupted = interrupted;
  }

  m_Ended.notify_all();
}

nlohmann::json mitk::RestJob::ToJson() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);

  const auto end = m_State == State::Running ? std::chrono::steady_clock::now() : m_End;
  const auto seconds = std::chrono::duration<double>(end - m_Start).count();

  nlohmann::json json = {
    { "id", m_Id },
    { "description", m_Description },
    { "state", GetStateName(m_State) },
    { "seconds", std::round(seconds * 10.0) / 10.0 }
  };

  if (m_State == State::Running)
  {
    json["cancelable"] = static_cast<bool>(m_CancelHandler);
  }
  else if (m_State == State::Succeeded)
  {
    json["result"] = m_Result;
  }
  else if (m_State == State::Failed)
  {
    json["error"] = m_Error;
  }

  return json;
}

mitk::RestResponse mitk::RestJob::Respond(const RestRequest& request, const std::shared_ptr<RestJob>& job)
{
  double wait = 0.0;

  if (const auto value = request.GetQueryParam("wait"); value.has_value())
  {
    try
    {
      // Not std::stod(), which reads the decimal separator of the C locale.
      wait = LexicalCast<double>(*value);
    }
    catch (const BadLexicalCast&)
    {
      wait = std::numeric_limits<double>::quiet_NaN();
    }

    if (!std::isfinite(wait))
      throw RestException(ErrorResponse::InvalidRequest("The query parameter \"wait\" must be a number of seconds.", request.Path));

    wait = std::clamp(wait, 0.0, MaxWaitSeconds);
  }

  bool ended = job->HasEnded();

  if (!ended && wait > 0.0)
  {
    const WaitPlace place;

    if (place.IsTaken())
      ended = job->WaitUntilEnded(std::chrono::milliseconds(static_cast<long long>(wait * 1000.0)));
  }

  auto response = RestResponse::FromData(job->ToJson(), ended ? 200 : 202);

  if (!ended)
    response.Headers["Location"] = "/api/v1/jobs/" + job->GetId();

  return response;
}

bool mitk::RestJob::End(State state, const nlohmann::json& result, const std::string& error)
{
  {
    std::lock_guard<std::mutex> lock(m_Mutex);

    if (m_State != State::Running)
      return false;

    m_State = state;
    m_Result = result;
    m_Error = error;
    m_End = std::chrono::steady_clock::now();
    m_CancelHandler = nullptr;
  }

  m_Ended.notify_all();
  return true;
}
