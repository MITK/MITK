/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkScopedProgressTask_h
#define mitkScopedProgressTask_h

namespace mitk
{
  class ProgressTask;

  /**
   * \brief Hands a task to a filter or command for as long as this exists.
   *
   * Something that reports progress does not own the task it reports into, and
   * usually outlives it by a long way: a tool keeps its filter between updates,
   * a view keeps one for as long as it is open. So the task has to be taken
   * away again on every path out of the call, including the ones an exception
   * takes, or a pointer to a dead task is left behind.
   *
   * \code
   * mitk::ProgressTask task("Registering surfaces");
   * mitk::ScopedProgressTask<mitk::AnisotropicIterativeClosestPointRegistration> scopedTask(filter, &task);
   *
   * filter->Update();
   * \endcode
   *
   * \tparam T Anything with a \c SetProgressTask(ProgressTask*) method.
   *
   * \sa ProgressTask
   */
  template <class T>
  class ScopedProgressTask final
  {
  public:
    /**
     * \brief Give the target the task until this goes out of scope.
     *
     * \param[in] target What reports into the task. Nothing happens if it is
     *        \c nullptr.
     * \param[in] task The task to report into, or \c nullptr for none.
     */
    ScopedProgressTask(T *target, ProgressTask *task)
      : m_Target(target)
    {
      if (nullptr != m_Target)
        m_Target->SetProgressTask(task);
    }

    /** \brief Takes the task away from the target again. */
    ~ScopedProgressTask()
    {
      if (nullptr != m_Target)
        m_Target->SetProgressTask(nullptr);
    }

    ScopedProgressTask(const ScopedProgressTask &) = delete;
    ScopedProgressTask &operator=(const ScopedProgressTask &) = delete;

  private:
    T *m_Target;
  };
}

#endif
