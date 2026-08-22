/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkProgressTaskInfo_h
#define mitkProgressTaskInfo_h

#include <MitkCoreExports.h>

#include <atomic>
#include <cstdint>
#include <string>

namespace mitk
{
  /** \brief Identifies a single task of the IProgressService. Never reused. */
  using ProgressTaskId = std::uint64_t;

  /**
   * \brief Complete snapshot of one task, as delivered to IProgressListener.
   *
   * A snapshot is self-contained on purpose: listeners are notified from
   * arbitrary threads and may apply a snapshot long after it was taken, so
   * they must never read back from the service to fill in the gaps.
   *
   * \sa IProgressListener
   * \sa IProgressService
   */
  struct ProgressTaskInfo
  {
    /** \brief Identifies the task this snapshot belongs to. */
    ProgressTaskId Id = 0;

    /**
     * \brief Increases strictly monotonically over all snapshots of a service.
     *
     * Snapshots of one task can reach a listener out of order, because a task
     * updated from a worker thread and finished from the GUI thread may be
     * delivered through different paths. Listeners must therefore discard any
     * snapshot whose sequence number does not exceed the last one they applied
     * for that task.
     */
    std::uint64_t Sequence = 0;

    /** \brief Human-readable name of the operation, shown to the user. */
    std::string Name;

    /** \brief Total number of steps, or 0 if the task is indeterminate. */
    unsigned int StepsToDo = 0;

    /** \brief Steps completed so far, never greater than StepsToDo. */
    unsigned int Progress = 0;

    /** \brief Whether the task polls IsCancelRequested() and can be cancelled. */
    bool Cancelable = false;

    /** \brief Whether cancellation was requested but the task is still running. */
    bool CancelRequested = false;

    /** \brief Whether this is the final snapshot of the task. */
    bool Finished = false;
  };

  /**
   * \brief State a running task shares with the IProgressService.
   *
   * Holding the cancel flag here rather than in the service keeps
   * ProgressTask::IsCancelRequested() a lock-free load, so a task can poll it
   * in a tight loop without contending on the service.
   *
   * \sa ProgressTask
   */
  struct ProgressTaskState
  {
    explicit ProgressTaskState(ProgressTaskId id)
      : Id(id),
        CancelRequested(false)
    {
    }

    const ProgressTaskId Id;
    std::atomic<bool> CancelRequested;
  };
}

#endif
