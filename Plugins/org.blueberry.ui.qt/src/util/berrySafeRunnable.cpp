/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berrySafeRunnable.h"

#include <typeinfo>

#include <berryLog.h>
#include <berryOperationCanceledException.h>

namespace berry
{

bool SafeRunnable::ignoreErrors = false;
ISafeRunnableRunner::Pointer SafeRunnable::runner;

class DefaultSafeRunnableRunner: public ISafeRunnableRunner
{
public:

  void Run(ISafeRunnable::Pointer code) override
  {
    try
    {
      code->Run();
    }
    catch (const ctkException& e)
    {
      HandleException(code, e);
    }
    catch (const std::exception& e)
    {
      HandleException(code, e);
    }
    catch (...)
    {
      HandleException(code);
    }
  }

private:

  void HandleException(ISafeRunnable::Pointer code)
  {
    HandleException(code, ctkException("Unknown exception thrown"));
  }

  void HandleException(ISafeRunnable::Pointer code,
                       const std::exception& e)
  {
    HandleException(code, ctkException(e.what()));
  }

  void HandleException(ISafeRunnable::Pointer code, const ctkException& e)
  {
    try
    {
      static_cast<void>(dynamic_cast<const OperationCanceledException&> (e));
    }
    catch (const std::bad_cast&)
    {
      // TODO logging
      try
      {
        //            Policy.getLog()
        //                .log(
        //                    new Status(IStatus.ERROR, Policy.JFACE,
        //                        IStatus.ERROR,
        //                        "Exception occurred", e)); //$NON-NLS-1$
        qDebug() << e.printStackTrace();
      }
      catch (...)
      {
        //e.printStackTrace();
        BERRY_ERROR << "Exception occurred" << std::endl;
      }
    }
    code->HandleException(e);
  }
};

SmartPointer<ISafeRunnableRunner> SafeRunnable::CreateDefaultRunner()
{
  ISafeRunnableRunner::Pointer runner(new DefaultSafeRunnableRunner());
  return runner;
}

SafeRunnable::SafeRunnable(const QString& message) :
  message(message)
{

}

void SafeRunnable::HandleException(const ctkException&  /*e*/)
{
  // Workaround to avoid interactive error dialogs during
  // automated testing
  if (ignoreErrors)
    return;

  if (message.isEmpty())
    message = "An error has occurred. See error log for more details.";

  // TODO status bar
  //    Policy.getStatusHandler().show(
  //        new Status(IStatus.ERROR, Policy.JFACE, message, e),
  //        JFaceResources.getString("SafeRunnable.errorMessage")); //$NON-NLS-1$
  BERRY_ERROR << message << std::endl;
}

bool SafeRunnable::GetIgnoreErrors()
{
  return ignoreErrors;
}

void SafeRunnable::SetIgnoreErrors(bool flag)
{
  ignoreErrors = flag;
}

SmartPointer<ISafeRunnableRunner> SafeRunnable::GetRunner()
{
  if (!runner)
  {
    runner = CreateDefaultRunner();
  }
  return runner;
}

void SafeRunnable::SetRunner(SmartPointer<ISafeRunnableRunner> runner)
{
  SafeRunnable::runner = runner;
}

void SafeRunnable::Run(SmartPointer<ISafeRunnable> runnable)
{
  GetRunner()->Run(runnable);
}

}
