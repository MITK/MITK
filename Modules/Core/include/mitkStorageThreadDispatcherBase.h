/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkStorageThreadDispatcherBase_h
#define mitkStorageThreadDispatcherBase_h

#include <MitkCoreExports.h>
#include <mitkCommon.h>
#include <itkObject.h>
#include <functional>

namespace mitk
{
  /**
   * \brief Abstract base class for dispatching tasks to the thread that owns DataStorage.
   *
   * Implementations provide thread-marshaling to ensure DataStorage operations
   * (which trigger synchronous observer events) execute on the correct thread.
   * (e.g. the Qt implementation dispatches to the GUI main thread.)
   *
   * In headless/test scenarios, no dispatcher is needed - tasks execute directly.
   */
  class MITKCORE_EXPORT StorageThreadDispatcherBase : public itk::Object
  {
  public:
    mitkClassMacroItkParent(StorageThreadDispatcherBase, itk::Object);

    /**
     * \brief Execute a task on the storage-owning thread, blocking until completion.
     *
     * Checks thread affinity automatically to avoid deadlocks
     * (e.g., Qt::BlockingQueuedConnection deadlocks if called
     * from the target thread). If on the dispatch thread, executes the task directly.
     *
     * \pre task must not be empty.
     */
    void Execute(std::function<void()> task)
    {
      if (this->IsDispatchThread())
      {
        task();
      }
      else
      {
        this->ExecuteDispatched(task);
      }
    }

    /**
     * \brief Check if the current thread is the dispatch target thread.
     *
     * Callers can check this before Execute(), but Execute() does it automatically
     * to avoid deadlocks (e.g., Qt::BlockingQueuedConnection deadlocks if called
     * from the target thread).
     *
     * \return True if the current thread is the dispatch target thread.
     */
    virtual bool IsDispatchThread() const = 0;

    /**
     * \brief Post a task to run on the storage-owning thread, returning immediately.
     *
     * Unlike Execute(), Post() never runs the task inline: it always defers to a
     * later turn of the dispatch thread's event loop, even when called from the
     * dispatch thread itself. Callers rely on that deferral -- e.g. to escape a
     * context (such as the platform loader lock held during library/plugin load)
     * in which running the task synchronously would deadlock.
     *
     * The task's result, if any, is discarded (fire-and-forget). Ordering relative
     * to other posted tasks follows the dispatch thread's own queue semantics.
     *
     * \pre task must not be empty.
     */
    virtual void Post(std::function<void()> task) = 0;

    /**
     * \brief Convenience template for tasks that return a value.
     *
     * Blocks until the task completes and returns the result.
     *
     * \return The result of the executed task.
     */
    template <typename R>
    R ExecuteWithResult(std::function<R()> task)
    {
      R result{};
      this->Execute([&result, &task]() { result = task(); });
      return result;
    }

  protected:

    /**
     * \brief Execute a task on the storage-owning thread, blocking until completion.
     *
     * Must be implemented in derived classes.
     *
     * \param[in] task the task to execute on the dispatch thread.
     * \pre task must not be empty.
     */
    virtual void ExecuteDispatched(std::function<void()> task) = 0;

    StorageThreadDispatcherBase() = default;
    ~StorageThreadDispatcherBase() override = default;
  };
}

#endif
