/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkRestJob_h
#define mitkRestJob_h

#include <mitkRestRoute.h>

#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>

namespace mitk
{
  /**
   * \brief Work that an endpoint started and that takes longer than a
   *        request should, like running a model.
   *
   * Clients follow jobs below /api/v1/jobs. An endpoint makes one with
   * IRestServerService::CreateJob(), answers with Respond(), and reports
   * progress and the outcome through the job. All methods are thread-safe.
   */
  class MITKRESTAPI_EXPORT RestJob
  {
  public:
    enum class State
    {
      Running,
      Succeeded,
      Failed,
      Cancelled
    };

    RestJob(const std::string& id, const std::string& description);

    RestJob(const RestJob&) = delete;
    RestJob& operator=(const RestJob&) = delete;

    std::string GetId() const;
    State GetState() const;
    bool HasEnded() const;

    /**
     * \param fraction The part of the work that is done, from 0 to 1, or
     *        negative if that is unknown.
     */
    void SetProgress(double fraction, const std::string& message = "");

    /**
     * \brief End the job with a result.
     *
     * The first of Succeed(), Fail() and Cancel() ends the job; later calls
     * change nothing.
     */
    void Succeed(const nlohmann::json& result = nlohmann::json::object());

    /** \brief End the job with an error. */
    void Fail(const std::string& message);

    /**
     * \brief Make the job cancellable.
     *
     * \param handler Stops the work. Runs on a thread of the server.
     */
    void SetCancelHandler(std::function<void()> handler);

    /**
     * \brief End the job as cancelled, after asking its handler to stop the work.
     *
     * \return False if the job has no cancel handler or has ended already.
     */
    bool Cancel();

    /** \return Whether the job has ended within the timeout. */
    bool WaitUntilEnded(std::chrono::milliseconds timeout) const;

    /**
     * \brief The job as clients see it: {id, description, state, progress,
     *        message, seconds, result | error}.
     */
    nlohmann::json ToJson() const;

    /**
     * \brief The answer of an endpoint that started a job.
     *
     * Waits up to the seconds that the query parameter "wait" of the request
     * gives, at most 120. Then answers with the job: status 200 if it has
     * ended, 202 and its location if it still runs.
     */
    static RestResponse Respond(const RestRequest& request, const std::shared_ptr<RestJob>& job);

  private:
    bool End(State state, const nlohmann::json& result, const std::string& error);

    const std::string m_Id;
    const std::string m_Description;
    const std::chrono::steady_clock::time_point m_Start;

    mutable std::mutex m_Mutex;
    mutable std::condition_variable m_Ended;
    State m_State;
    double m_Progress;
    std::string m_Message;
    nlohmann::json m_Result;
    std::string m_Error;
    std::chrono::steady_clock::time_point m_End;
    std::function<void()> m_CancelHandler;
  };
}

#endif
