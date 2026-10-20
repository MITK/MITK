/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkRestJob.h>

#include <mitkErrorResponse.h>

#include <algorithm>
#include <cmath>

namespace
{
  constexpr double MaxWaitSeconds = 120.0;

  std::string ToString(mitk::RestJob::State state)
  {
    switch (state)
    {
      case mitk::RestJob::State::Running:
        return "running";
      case mitk::RestJob::State::Succeeded:
        return "succeeded";
      case mitk::RestJob::State::Failed:
        return "failed";
      case mitk::RestJob::State::Cancelled:
        return "cancelled";
    }

    return "unknown";
  }
}

mitk::RestJob::RestJob(const std::string& id, const std::string& description)
  : m_Id(id),
    m_Description(description),
    m_Start(std::chrono::steady_clock::now()),
    m_State(State::Running),
    m_Progress(-1.0)
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

void mitk::RestJob::SetProgress(double fraction, const std::string& message)
{
  std::lock_guard<std::mutex> lock(m_Mutex);

  if (m_State != State::Running)
    return;

  m_Progress = fraction < 0.0 ? -1.0 : std::min(fraction, 1.0);
  m_Message = message;
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

  this->End(State::Cancelled, nlohmann::json(), std::string());
  return true;
}

bool mitk::RestJob::WaitUntilEnded(std::chrono::milliseconds timeout) const
{
  std::unique_lock<std::mutex> lock(m_Mutex);
  return m_Ended.wait_for(lock, timeout, [this]() { return m_State != State::Running; });
}

nlohmann::json mitk::RestJob::ToJson() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);

  const auto end = m_State == State::Running ? std::chrono::steady_clock::now() : m_End;
  const auto seconds = std::chrono::duration<double>(end - m_Start).count();

  nlohmann::json json = {
    { "id", m_Id },
    { "description", m_Description },
    { "state", ToString(m_State) },
    { "seconds", std::round(seconds * 10.0) / 10.0 }
  };

  if (m_State == State::Running)
  {
    json["progress"] = m_Progress < 0.0 ? nlohmann::json() : nlohmann::json(m_Progress);

    if (!m_Message.empty())
      json["message"] = m_Message;

    json["cancellable"] = static_cast<bool>(m_CancelHandler);
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
      wait = std::clamp(std::stod(*value), 0.0, MaxWaitSeconds);
    }
    catch (const std::exception&)
    {
      throw RestException(ErrorResponse::InvalidRequest("The query parameter \"wait\" must be a number of seconds.", request.Path));
    }
  }

  const bool ended = job->WaitUntilEnded(std::chrono::milliseconds(static_cast<long long>(wait * 1000.0)));
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
