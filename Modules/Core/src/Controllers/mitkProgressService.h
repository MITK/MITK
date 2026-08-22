/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkProgressService_h
#define mitkProgressService_h

#include <mitkIProgressService.h>

#include <map>
#include <mutex>

namespace mitk
{
  /**
   * \brief Implementation of the IProgressService interface.
   *
   * Keeps one record per running task and broadcasts a complete snapshot on
   * every change. Snapshots are numbered so that listeners can restore order
   * after a delivery path reordered them.
   *
   * The service mutex is never held while a listener runs: notification would
   * otherwise drag arbitrary listener code, including Qt widget code and its
   * event filters, into a lock that every reporting thread contends on.
   * Listener lifetime is covered by a per-registration mutex instead.
   *
   * \sa IProgressService
   * \sa IProgressListener
   */
  class ProgressService : public IProgressService
  {
  public:
    ProgressService();
    ~ProgressService() override;

    bool AddListener(IProgressListener* listener) override;
    bool RemoveListener(const IProgressListener* listener) override;
    std::vector<ProgressTaskInfo> GetActiveTasks() const override;
    void RequestCancel(ProgressTaskId id) override;

    std::shared_ptr<ProgressTaskState> StartTask(const std::string& name,
                                                 unsigned int steps,
                                                 bool cancelable) override;
    void UpdateTask(ProgressTaskId id,
                    const std::string& name,
                    unsigned int steps,
                    unsigned int progress) override;
    void FinishTask(ProgressTaskId id) override;

  private:
    /**
     * \brief A listener and the lock that keeps it alive while it is notified.
     *
     * Held by shared_ptr so that a notification in progress keeps the record
     * alive even after RemoveListener() dropped it from m_Registrations.
     */
    struct Registration
    {
      std::mutex Mutex;
      IProgressListener* Listener;
    };

    struct Task
    {
      std::shared_ptr<ProgressTaskState> State;
      ProgressTaskInfo Info;
    };

    using Registrations = std::vector<std::shared_ptr<Registration>>;

    /** \brief Call the listeners. Must be called with m_Mutex released. */
    static void Notify(const Registrations& registrations, const ProgressTaskInfo& info);

    mutable std::mutex m_Mutex;
    std::map<ProgressTaskId, Task> m_Tasks;
    Registrations m_Registrations;
    ProgressTaskId m_NextId;
    std::uint64_t m_NextSequence;
  };
}

#endif
