/*=========================================================================
 
 Program:   openCherry Platform
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

#include "cherryWorkbenchTestable.h"

#include "../cherryDisplay.h"
#include "../cherryIWorkbench.h"

#include <Poco/Thread.h>

namespace cherry
{


  WorkbenchTestable::WorkbenchTestRunnable::WorkbenchTestRunnable(TestableObject* testable) :
    testable(testable)
  {
  }

  void WorkbenchTestable::WorkbenchTestRunnable::run()
  {
    //                  // Some tests (notably the startup performance tests) do not want to wait for early startup.
    //                  // Allow this to be disabled by specifying the system property: org.eclipse.ui.testsWaitForEarlyStartup=false
    //                  // For details, see bug 94129 [Workbench] Performance test regression caused by workbench harness change
    //                  if (!"false".equalsIgnoreCase(System.getProperty(PlatformUI.PLUGIN_ID + ".testsWaitForEarlyStartup"))) {  //$NON-NLS-1$ //$NON-NLS-2$
    //                    waitForEarlyStartup();
    //                  }
    testable->GetTestHarness()->RunTests();
  }

class WorkbenchCloseRunnable: public Poco::Runnable
{

public:

  WorkbenchCloseRunnable(IWorkbench* workbench) :
    workbench(workbench)
  {

  }

  void run()
  {
    poco_assert(workbench->Close());
  }

private:

  IWorkbench* workbench;

};

WorkbenchTestable::WorkbenchTestable()
 : thread("WorkbenchTestable"), testRunnable(this)
 {
  // do nothing
}

void WorkbenchTestable::Init(Display* display, IWorkbench* workbench)
{
  poco_assert(display);
  poco_assert(workbench);
  this->display = display;
  this->workbench = workbench;
  if (GetTestHarness())
  {
    // don't use a job, since tests often wait for all jobs to complete before proceeding
    thread.start(testRunnable);
  }
}

void WorkbenchTestable::TestingStarting()
{
  poco_assert(workbench);
  //        oldAutomatedMode = ErrorDialog.AUTOMATED_MODE;
  //        ErrorDialog.AUTOMATED_MODE = true;
  //        oldIgnoreErrors = SafeRunnable.getIgnoreErrors();
  //        SafeRunnable.setIgnoreErrors(true);
}

void WorkbenchTestable::RunTest(Poco::Runnable* testRunnable)
{
  poco_assert(workbench);
  display->SyncExec(testRunnable);
}

void WorkbenchTestable::TestingFinished()
{
  // force events to be processed, and ensure the close is done in the UI thread
  WorkbenchCloseRunnable runnable(workbench);
  display->SyncExec(&runnable);
  //        ErrorDialog.AUTOMATED_MODE = oldAutomatedMode;
  //        SafeRunnable.setIgnoreErrors(oldIgnoreErrors);
}

}
