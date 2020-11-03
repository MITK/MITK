/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryWorkbenchTestable.h"

#include "berryDisplay.h"
#include "berryIWorkbench.h"

#include <Poco/Thread.h>

namespace berry
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

  void run() override
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
