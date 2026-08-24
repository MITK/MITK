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

#include <functional>
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

    /**
     * \brief Report to the given callback instead of raising a notification.
     *
     * For an operation whose progress is already covered by whoever called
     * it. Steps still accumulate, so that anything nested inside has a task
     * to report into, but what comes out is a fraction for the caller rather
     * than a notification of its own.
     *
     * \param report Receives how much of the work is done, from 0 to 1.
     * \param steps Total number of steps, or Indeterminate.
     */
    explicit ProgressTask(std::function<void(float)> report, unsigned int steps = Indeterminate);

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
     *
     * Never moves the task backwards: a value below what was already reported
     * is published as a repeat of the latter. Reporting phases that map onto
     * shares of one budget therefore does not need to end them in order.
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
    std::function<void(float)> m_Report;
    std::string m_Name;
    unsigned int m_StepsToDo;
    unsigned int m_Progress;
  };

  /**
   * \brief Make a task that reports into a share of another one.
   *
   * For an operation that is one part of a larger one: it reports its own
   * progress from 0 to 1, and the caller sees that arrive as the given number
   * of steps of the caller's own budget.
   *
   * Those steps have to be part of that budget already. Declare them where the
   * total is declared, or AddStepsToDo() them before the work starts; growing
   * the budget once it is under way shrinks the fraction the bar shows, and a
   * bar that falls back reads as an operation coming undone.
   *
   * Steps are handed over relative rather than absolute, so that the caller
   * keeps whatever progress it made itself.
   *
   * \param[in] task The task to report into, or nullptr to report nowhere.
   * \param[in] steps How much of that task this share accounts for.
   * \return A task to hand to the nested operation. Reports the share as done
   *         when it is destroyed.
   */
  MITKCORE_EXPORT ProgressTask MakeProgressShare(ProgressTask* task, unsigned int steps);
}

#endif
