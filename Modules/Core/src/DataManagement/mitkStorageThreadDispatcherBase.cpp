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
