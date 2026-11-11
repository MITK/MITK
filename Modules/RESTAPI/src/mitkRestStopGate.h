/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkRestStopGate_h
#define mitkRestStopGate_h

#include <mitkStorageThreadDispatcherBase.h>

#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>

namespace mitk
{
  /**
   * \brief Lets requests wait for the dispatch thread in a way that a
   *        stopping server can end.
   *
   * The server waits for its requests in flight when it stops, typically on
   * the dispatch thread. A request that waits for that thread in turn would
   * never return, so it waits here instead of in the dispatcher's blocking
   * call, and gives up when the server stops.
   */
  class RestStopGate
  {
  public:
    /** \brief While set, the waiting calls of Run() give up and new ones do not start. */
    void SetStopping(bool stopping);

    bool IsStopping() const;

    /**
     * \brief Run a task on the dispatcher's thread and wait for it.
     *
     * The task runs right away without a dispatcher and on its thread.
     * What the task throws is thrown again here. Without a gate, nothing
     * ends the wait early.
     *
     * \throw RestException (503) if the server stops before the task has
     *        started. The task does not run then.
     */
    static void Run(const std::shared_ptr<RestStopGate>& gate, StorageThreadDispatcherBase* dispatcher, const std::function<void()>& task);

  private:
    mutable std::mutex m_Mutex;
    std::condition_variable m_Changed;
    bool m_Stopping = false;
  };
}

#endif
