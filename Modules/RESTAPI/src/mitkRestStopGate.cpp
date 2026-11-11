/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkRestStopGate.h"

#include <mitkErrorResponse.h>
#include <mitkRestRoute.h>

#include <exception>

void mitk::RestStopGate::SetStopping(bool stopping)
{
  {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Stopping = stopping;
  }

  m_Changed.notify_all();
}

bool mitk::RestStopGate::IsStopping() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return m_Stopping;
}

void mitk::RestStopGate::Run(const std::shared_ptr<RestStopGate>& gateOrNull, StorageThreadDispatcherBase* dispatcher, const std::function<void()>& task)
{
  if (dispatcher == nullptr || dispatcher->IsDispatchThread())
  {
    task();
    return;
  }

  const auto gate = gateOrNull != nullptr ? gateOrNull : std::make_shared<RestStopGate>();

  struct Call
  {
    bool Started = false;
    bool Done = false;
    bool Abandoned = false;
    std::exception_ptr Error;
  };

  const auto call = std::make_shared<Call>();

  std::unique_lock<std::mutex> lock(gate->m_Mutex);

  if (!gate->m_Stopping)
  {
    // The task is referenced: it outlives the call unless the call is abandoned, and then it does not run.
    dispatcher->Post([gate, call, &task]() {
      {
        std::lock_guard<std::mutex> callLock(gate->m_Mutex);

        if (call->Abandoned)
          return;

        call->Started = true;
      }

      std::exception_ptr error;

      try
      {
        task();
      }
      catch (...)
      {
        error = std::current_exception();
      }

      {
        std::lock_guard<std::mutex> callLock(gate->m_Mutex);
        call->Error = error;
        call->Done = true;
      }

      gate->m_Changed.notify_all();
    });

    gate->m_Changed.wait(lock, [&gate, &call]() { return call->Done || (gate->m_Stopping && !call->Started); });
  }

  if (!call->Done)
  {
    call->Abandoned = true;
    throw RestException(ErrorResponse::Create("SERVER_STOPPING", "Service Unavailable", "The server is stopping.", 503, ""));
  }

  if (call->Error)
    std::rethrow_exception(call->Error);
}
