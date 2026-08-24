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
      // A hand-over that could not be delivered still has to run: dropping it
      // would turn a data storage mutation into a silent no-op.
      if (this->IsDispatchThread() || !this->ExecuteDispatched(task))
        task();
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
     * \return True if the task was delivered and has run. False if it could not
     *         be delivered, in which case Execute() runs it here instead.
     */
    virtual bool ExecuteDispatched(std::function<void()> task) = 0;

    StorageThreadDispatcherBase() = default;
    ~StorageThreadDispatcherBase() override = default;
  };

  /**
   * \brief Run a task on the thread that owns the data storage.
   *
   * Mutating the storage notifies observers synchronously, and those observers
   * are rendering and user interface code that belongs to one thread. So work
   * running elsewhere hands the whole operation over instead of doing it where
   * it happens to be.
   *
   * Handing over blocks until the task has run, so the owning thread must be
   * able to reach its event loop. Waiting for a worker without one, as a bare
   * QFuture::waitForFinished() does, deadlocks instead.
   *
   * An exception the task throws is carried back and rethrown here. Left to
   * itself it would unwind the owning thread's event loop rather than reach the
   * caller, which is where the error belongs.
   *
   * The dispatcher is resolved on every call rather than cached. It belongs to
   * the data storage service and dies with the plugin that installed it, which
   * a caller outliving that plugin has no way of being told about.
   *
   * \param[in] task The operation to run on the owning thread.
   * \return True if the task was handed over and has already run. False if
   *         there is nothing to hand over to, as in a command line tool, or
   *         this is already the owning thread; the task has *not* run then and
   *         the caller carries on itself.
   */
  MITKCORE_EXPORT bool DispatchToStorageThread(const std::function<void()> &task);

  /**
   * \brief Run a task on the thread that owns the data storage, or here if this
   *        already is that thread or there is no such thread.
   *
   * The unconditional form of DispatchToStorageThread(), for writing to data
   * that is already on display: it has to happen where everything else reads
   * it, and the caller does not care which thread that turns out to be.
   *
   * \param[in] task The operation to run.
   */
  MITKCORE_EXPORT void RunWhereTheDataLives(const std::function<void()> &task);

  /**
   * \brief Warn when data the storage thread reads is built somewhere else.
   *
   * A mitk::Image or mitk::Surface builds its VTK representation on first
   * access, on whatever thread asks. Doing that on a worker while the mappers
   * on the owning thread read the same object is a race that nothing else
   * catches: the work is supposed to be done on the owning thread before the
   * data is handed over, and forgetting it fails only sometimes.
   *
   * Silent where there is no thread that owns the data, as in a command line
   * tool or a test, and silent on that thread itself.
   *
   * \param[in] what Names the action, for the message, for example
   *        "Building the VTK representation of an image".
   */
  MITKCORE_EXPORT void WarnIfOffStorageThread(const char *what);
}

#endif
