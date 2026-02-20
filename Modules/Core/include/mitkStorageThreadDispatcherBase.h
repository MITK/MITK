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
   * @brief Abstract base class for dispatching tasks to the thread that owns DataStorage.
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
     * @brief Execute a task on the storage-owning thread. Blocks until completion.
     * Execute() does check thread affinity automatically to avoid deadlocks
     * (e.g., Qt::BlockingQueuedConnection deadlocks if called
     * from the target thread). If true, execute the task directly instead of dispatching.
     * @pre task must not be empty.
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
     * @brief Check if the current thread IS the dispatch target thread.
     *
     * Callers can check this before Execute(), but Execute() does it automatically
     * to avoid deadlocks (e.g., Qt::BlockingQueuedConnection deadlocks if called
     * from the target thread).
     */
    virtual bool IsDispatchThread() const = 0;

    /**
     * @brief Convenience template for tasks that return a value.
     * Blocks until the task completes and returns the result.
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
     * @brief Execute a task on the storage-owning thread. Blocks until completion.
     * Needs to be implemented in derived classes.
     * @pre task must not be empty.
     */
    virtual void ExecuteDispatched(std::function<void()> task) = 0;

    StorageThreadDispatcherBase() = default;
    ~StorageThreadDispatcherBase() override = default;
  };
}

#endif
