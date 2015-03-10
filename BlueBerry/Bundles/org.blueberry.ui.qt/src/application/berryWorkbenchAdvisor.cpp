/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "berryWorkbenchAdvisor.h"

#include <Poco/Exception.h>

namespace berry
{

WorkbenchAdvisor::WorkbenchAdvisor()
{
  // do nothing
}

WorkbenchAdvisor::~WorkbenchAdvisor()
{
}

void WorkbenchAdvisor::InternalBasicInitialize(IWorkbenchConfigurer::Pointer configurer)
{
  if (workbenchConfigurer.IsNotNull())
  {
    throw Poco::IllegalStateException();
  }
  this->workbenchConfigurer = configurer;
  this->Initialize(configurer);
}

void WorkbenchAdvisor::Initialize(IWorkbenchConfigurer::Pointer  /*configurer*/)
{
  // do nothing
}

IWorkbenchConfigurer::Pointer WorkbenchAdvisor::GetWorkbenchConfigurer()
{
  return workbenchConfigurer;
}

void WorkbenchAdvisor::PreStartup()
{
  // do nothing
}

void WorkbenchAdvisor::PostStartup()
{
  // do nothing
}

bool WorkbenchAdvisor::PreShutdown()
{
  return true;
}

void WorkbenchAdvisor::PostShutdown()
{
  // do nothing
}

IAdaptable* WorkbenchAdvisor::GetDefaultPageInput()
{
  // default: no input
  return 0;
}

QString WorkbenchAdvisor::GetMainPreferencePageId()
{
  // default: no opinion
  return "";
}

bool WorkbenchAdvisor::OpenWindows()
{
  //   final Display display = PlatformUI.getWorkbench().getDisplay();
  //   final boolean result [] = new boolean[1];
  //
  //   // spawn another init thread.  For API compatibility We guarantee this method is called from
  //   // the UI thread but it could take enough time to disrupt progress reporting.
  //   // spawn a new thread to do the grunt work of this initialization and spin the event loop
  //   // ourselves just like it's done in Workbench.
  //   final boolean[] initDone = new boolean[]{false};
  //   final Throwable [] error = new Throwable[1];
  //   Thread initThread = new Thread() {
  //     /* (non-Javadoc)
  //      * @see java.lang.Thread#run()
  //      */
  //       void run() {
  //       try {
  //         //declare us to be a startup thread so that our syncs will be executed
  //         UISynchronizer.startupThread.set(Boolean.TRUE);
  //         final IWorkbenchConfigurer [] myConfigurer = new IWorkbenchConfigurer[1];
  //         StartupThreading.runWithoutExceptions(new StartupRunnable() {
  //
  //             void runWithException() throws Throwable {
  //             myConfigurer[0] = getWorkbenchConfigurer();
  //
  //           }});
  //
  //         IStatus status = myConfigurer[0].restoreState();
  //         if (!status.isOK()) {
  //           if (status.getCode() == IWorkbenchConfigurer.RESTORE_CODE_EXIT) {
  //             result[0] = false;
  //             return;
  //           }
  //           if (status.getCode() == IWorkbenchConfigurer.RESTORE_CODE_RESET) {
  //             myConfigurer[0].openFirstTimeWindow();
  //           }
  //         }
  //         result[0] = true;
  //       } catch (Throwable e) {
  //         error[0] = e;
  //       }
  //       finally {
  //         initDone[0] = true;
  //         display.wake();
  //       }
  //     }};
  //     initThread.start();
  //
  //     while (true) {
  //       if (!display.readAndDispatch()) {
  //         if (initDone[0])
  //           break;
  //         display.sleep();
  //       }
  //
  //     }
  //
  //     // can only be a runtime or error
  //     if (error[0] instanceof Error)
  //       throw (Error)error[0];
  //     else if (error[0] instanceof RuntimeException)
  //       throw (RuntimeException)error[0];
  //
  //     return result[0];

  IWorkbenchConfigurer::Pointer myConfigurer = this->GetWorkbenchConfigurer();

  bool status = myConfigurer->RestoreState();
  if (!status)
  {
    myConfigurer->OpenFirstTimeWindow();
  }
  return true;
}

bool WorkbenchAdvisor::SaveState(IMemento::Pointer  /*memento*/)
{
  return true;
}

bool WorkbenchAdvisor::RestoreState(IMemento::Pointer  /*memento*/)
{
  return true;
}

}
