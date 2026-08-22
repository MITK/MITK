/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkProgressTask_h
#define mitkProgressTask_h

#include <mitkProgressTaskInfo.h>

#include <memory>
#include <string>

namespace mitk
{
  class IProgressService;

  /**
   * \brief Reports the progress of one named, long-running operation.
   *
   * A task exists for as long as its handle does. Destroying the handle ends
   * the task, so an early return or an exception cannot leave progress
   * reporting in a bogus state:
   *
   * \code
   * mitk::ProgressTask task("Loading files", 2 * filesToRead);
   *
   * for (const auto& file : files)
   * {
   *   Read(file);
   *   task.Progress(2);
   * }
   * \endcode
   *
   * Tasks are independent of each other, so concurrent operations report
   * without interfering. A handle belongs to a single thread; it is move-only
   * and must not be shared, but different threads may each own one.
   *
   * Reporting is silently skipped when no IProgressService is available, which
   * is the normal situation in command-line tools and tests.
   *
   * \sa IProgressService
   */
  class MITKCORE_EXPORT ProgressTask final
  {
  public:
    /** \brief Step count of a task whose extent is unknown in advance. */
    static constexpr unsigned int Indeterminate = 0;

    /**
     * \brief Start a task and show it to the user.
     *
     * \param name What the user sees. Name the operation, not the class, for
     *        example "Loading files" rather than "IOUtil".
     * \param steps Total number of steps, or Indeterminate.
     * \param cancelable Whether this task polls IsCancelRequested(). Claiming
     *        it without polling offers the user a cancel that never happens.
     */
    explicit ProgressTask(const std::string& name,
                          unsigned int steps = Indeterminate,
                          bool cancelable = false);

    /** \brief Ends the task. */
    ~ProgressTask();

    ProgressTask(ProgressTask&& other) noexcept;
    ProgressTask& operator=(ProgressTask&& other) noexcept;

    ProgressTask(const ProgressTask&) = delete;
    ProgressTask& operator=(const ProgressTask&) = delete;

    /** \brief Get the id of this task, or 0 once it has finished. */
    ProgressTaskId GetId() const;

    /** \brief Rename the task, for example when it enters another phase. */
    void SetName(const std::string& name);

    /** \brief Increase the total number of steps of a running task. */
    void AddStepsToDo(unsigned int steps);

    /** \brief Advance by the given number of steps, clamped to the total. */
    void Progress(unsigned int steps = 1);

    /**
     * \brief Set the number of completed steps, clamped to the total.
     *
     * Use this to forward a fractional progress value without accumulating
     * rounding errors, for example
     * \c task.SetProgress(fraction * task.GetStepsToDo()).
     */
    void SetProgress(unsigned int progress);

    /** \brief Get the total number of steps, or Indeterminate. */
    unsigned int GetStepsToDo() const;

    /**
     * \brief Check whether the user asked for this task to stop.
     *
     * A lock-free read, so polling it in a tight loop is free. Always false
     * unless the task was started as cancelable.
     */
    bool IsCancelRequested() const;

    /**
     * \brief End the task before the handle goes out of scope.
     *
     * Calling this more than once has no effect.
     */
    void Finish() noexcept;

  private:
    void Publish();

    IProgressService* m_Service;
    std::shared_ptr<ProgressTaskState> m_State;
    std::string m_Name;
    unsigned int m_StepsToDo;
    unsigned int m_Progress;
  };
}

#endif
