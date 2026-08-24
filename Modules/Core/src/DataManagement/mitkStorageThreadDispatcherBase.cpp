/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkStorageThreadDispatcherBase.h>

#include <mitkCoreServices.h>
#include <mitkIDataStorageService.h>
#include <mitkLog.h>

#include <exception>

bool mitk::DispatchToStorageThread(const std::function<void()> &task)
{
  // Held for the duration of the call, so that the dispatcher it owns cannot go
  // away underneath the hand-over below.
  CoreServicePointer<IDataStorageService> service(CoreServices::GetDataStorageService());

  auto *dispatcher = service
    ? service->GetDispatcher()
    : nullptr;

  if (nullptr == dispatcher || dispatcher->IsDispatchThread())
    return false;

  std::exception_ptr exception;

  dispatcher->Execute([&task, &exception]()
    {
      try
      {
        task();
      }
      catch (...)
      {
        exception = std::current_exception();
      }
    });

  if (exception)
    std::rethrow_exception(exception);

  return true;
}

void mitk::RunWhereTheDataLives(const std::function<void()> &task)
{
  if (!DispatchToStorageThread(task))
    task();
}

void mitk::WarnIfOffStorageThread(const char *what)
{
  try
  {
    CoreServicePointer<IDataStorageService> service(CoreServices::GetDataStorageService());

    auto *dispatcher = service
      ? service->GetDispatcher()
      : nullptr;

    // No dispatcher means nothing owns the data, which is the ordinary situation
    // in a command line tool or a test. There is no other thread to race with
    // there, and warning would be noise.
    if (nullptr == dispatcher || dispatcher->IsDispatchThread())
      return;

    MITK_WARN << what << " off the thread that owns the data storage. Whatever "
                 "reads it there may see it half written. Do it on that thread, or "
                 "prepare the data before handing the work over.";
  }
  catch (...)
  {
    // A diagnostic must never be the reason an operation fails.
  }
}
