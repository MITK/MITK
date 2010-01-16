/*=========================================================================
 
 Program:   BlueBerry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$
 
 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.
 
 =========================================================================*/

#include "berrySafeRunnable.h"

#include <typeinfo>
#include <berrySolsticeExceptions.h>

namespace berry
{

bool SafeRunnable::ignoreErrors = false;
ISafeRunnableRunner::Pointer SafeRunnable::runner;

class DefaultSafeRunnableRunner: public ISafeRunnableRunner
{
public:

  void Run(ISafeRunnable::Pointer code)
  {
    try
    {
      code->Run();
    } catch (const std::exception& e)
    {
      HandleException(code, e);
    } catch (...)
    {
      HandleException(code);
    }
  }

private:

  void HandleException(ISafeRunnable::Pointer code, const std::exception& e =
      std::exception())
  {
    try
    {
      dynamic_cast<const OperationCanceledException&> (e);
    } catch (const std::bad_cast&)
    {
      // TODO logging
      try
      {
        //            Policy.getLog()
        //                .log(
        //                    new Status(IStatus.ERROR, Policy.JFACE,
        //                        IStatus.ERROR,
        //                        "Exception occurred", e)); //$NON-NLS-1$
      } catch (...)
      {
        //e.printStackTrace();
        std::cerr << "Exception occurred" << std::endl;
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

SafeRunnable::SafeRunnable(const std::string& message) :
  message(message)
{

}

void SafeRunnable::HandleException(const std::exception&  /*e*/)
{
  // Workaround to avoid interactive error dialogs during
  // automated testing
  if (ignoreErrors)
    return;

  if (message.empty())
    message = "An error has occurred. See error log for more details.";

  // TODO status bar
  //    Policy.getStatusHandler().show(
  //        new Status(IStatus.ERROR, Policy.JFACE, message, e),
  //        JFaceResources.getString("SafeRunnable.errorMessage")); //$NON-NLS-1$
  std::cerr << message << std::endl;
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
