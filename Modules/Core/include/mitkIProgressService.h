/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkIProgressService_h
#define mitkIProgressService_h

#include <mitkIProgressListener.h>
#include <mitkServiceInterface.h>

#include <memory>
#include <vector>

namespace mitk
{
  /**
   * \brief Tracks the progress of concurrently running, named tasks.
   *
   * Every task is independent: reporting progress for one never affects
   * another, and a task that is abandoned on an early return or an exception
   * cleans up after itself. This is what distinguishes the service from a
   * single shared progress bar.
   *
   * Producers do not use this interface directly. They create a ProgressTask,
   * which acquires the service, keeps the bookkeeping local, and ends the task
   * when it goes out of scope.
   *
   * Consumers (typically one per application window) implement
   * IProgressListener and register with AddListener().
   *
   * All methods are safe to call from any thread.
   *
   * \sa ProgressTask
   * \sa IProgressListener
   * \sa CoreServices::GetProgressService()
   *
   * \ingroup MicroServices_Interfaces
   */
  class MITKCORE_EXPORT IProgressService
  {
  public:
    virtual ~IProgressService();

    /**
     * \brief Add a listener that is notified of all task snapshots.
     *
     * A newly added listener immediately receives one snapshot per task that
     * is currently running, so that it does not miss operations that started
     * before it existed.
     *
     * \param listener A valid pointer to a listener. Adding it twice has no
     *        additional effect.
     *
     * \return True if the listener is registered, false if it was \c nullptr.
     */
    virtual bool AddListener(IProgressListener* listener) = 0;

    /**
     * \brief Remove a previously added listener.
     *
     * Returns only once no notification of that listener is in progress, so it
     * is safe to destroy the listener right afterwards.
     *
     * \param listener A pointer to an already added listener.
     *
     * \return True if the listener was found and removed.
     */
    virtual bool RemoveListener(const IProgressListener* listener) = 0;

    /** \brief Get a snapshot of every task that is currently running. */
    virtual std::vector<ProgressTaskInfo> GetActiveTasks() const = 0;

    /**
     * \brief Ask a task to stop.
     *
     * Only a request: the task decides when to honor it. Tasks that were not
     * started as cancelable ignore it entirely, as do unknown ids.
     *
     * \param id The task to cancel.
     */
    virtual void RequestCancel(ProgressTaskId id) = 0;

    /**
     * \brief Start a new task.
     *
     * Prefer ProgressTask over calling this directly; a task started here must
     * be finished by an explicit FinishTask(), including on error paths.
     *
     * \param steps Total number of steps, or 0 for indeterminate progress.
     * \param cancelable Whether the task polls the returned cancel flag.
     *
     * \return The shared state of the new task, never \c nullptr.
     */
    virtual std::shared_ptr<ProgressTaskState> StartTask(const std::string& name,
                                                         unsigned int steps,
                                                         bool cancelable) = 0;

    /**
     * \brief Publish the current state of a running task.
     *
     * Absolute rather than incremental, so that a lost or reordered update
     * cannot accumulate an error. Unknown ids are ignored.
     *
     * \param progress Steps completed; clamped to \c steps.
     */
    virtual void UpdateTask(ProgressTaskId id,
                            const std::string& name,
                            unsigned int steps,
                            unsigned int progress) = 0;

    /**
     * \brief End a task and publish its final snapshot.
     *
     * Unknown ids are ignored, so finishing twice is harmless.
     */
    virtual void FinishTask(ProgressTaskId id) = 0;
  };
}

MITK_DECLARE_SERVICE_INTERFACE(mitk::IProgressService, "org.mitk.IProgressService")

#endif
