/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkManualStorageThreadDispatcher_h
#define mitkManualStorageThreadDispatcher_h

#include <mitkStorageThreadDispatcherBase.h>

#include <atomic>
#include <deque>
#include <functional>
#include <future>
#include <mutex>
#include <thread>
#include <utility>

namespace mitk
{
  /**
   * \brief Stands in for the dispatcher the Workbench installs, in tests.
   *
   * The thread that owns the data is whichever thread constructs this, and it runs what is
   * handed over or posted to it only when Drain() is called, the way the Qt implementation
   * lets its event loop do it.
   *
   * Running the tasks anywhere else would not model the real thing at all: a mutation that
   * is handed over re-enters the very method that handed it over, and only arriving on the
   * owning thread stops it handing itself over again.
   */
  class ManualStorageThreadDispatcher : public StorageThreadDispatcherBase
  {
  public:
    mitkClassMacro(ManualStorageThreadDispatcher, StorageThreadDispatcherBase);
    itkFactorylessNewMacro(Self);

    /** How often anything was handed over to the owning thread. */
    std::atomic<int> HandedOver{ 0 };

    /** How often anything was posted to the owning thread. */
    std::atomic<int> Posted{ 0 };

    /** Whether a hand-over can be delivered at all. */
    std::atomic<bool> Deliverable{ true };

    bool IsDispatchThread() const override
    {
      return std::this_thread::get_id() == m_OwningThread;
    }

    void Post(std::function<void()> task) override
    {
      ++Posted;

      std::lock_guard<std::mutex> locked(m_Mutex);
      m_Queue.push_back(std::move(task));
    }

    /** \brief Run whatever has been handed over or posted, as an event loop would. */
    void Drain()
    {
      for (;;)
      {
        std::function<void()> task;

        {
          std::lock_guard<std::mutex> locked(m_Mutex);

          if (m_Queue.empty())
            return;

          task = std::move(m_Queue.front());
          m_Queue.pop_front();
        }

        task();
      }
    }

  protected:
    ManualStorageThreadDispatcher()
      : m_OwningThread(std::this_thread::get_id())
    {
    }

    ~ManualStorageThreadDispatcher() override = default;

    bool ExecuteDispatched(std::function<void()> task) override
    {
      ++HandedOver;

      if (!Deliverable)
        return false;

      std::promise<void> ran;
      auto done = ran.get_future();

      // Swallowed rather than let out, so that a throwing task cannot leave the wait below on
      // a promise nobody fulfils. Nothing is lost: whatever hands work over here has already
      // taken the exception off the task.
      this->Post([&task, &ran]()
        {
          try
          {
            task();
          }
          catch (...)
          {
          }

          ran.set_value();
        });

      done.wait();

      return true;
    }

  private:
    std::thread::id m_OwningThread;

    std::mutex m_Mutex;
    std::deque<std::function<void()>> m_Queue;
  };
}

#endif
