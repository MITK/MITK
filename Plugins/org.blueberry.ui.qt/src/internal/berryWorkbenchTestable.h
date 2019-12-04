/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYWORKBENCHTESTABLE_H_
#define BERRYWORKBENCHTESTABLE_H_

#include "testing/berryTestableObject.h"

#include <Poco/Thread.h>

namespace berry
{

class Display;
struct IWorkbench;

/**
 * The Workbench's testable object facade to a test harness.
 *
 * @since 3.0
 */
class WorkbenchTestable: public TestableObject
{

private:

  Display* display;

  IWorkbench* workbench;

  Poco::Thread thread;

  class WorkbenchTestRunnable: public Poco::Runnable
  {
  public:

    WorkbenchTestRunnable(TestableObject* testable);

    void run() override;

  private:

    TestableObject* testable;

  };

  WorkbenchTestRunnable testRunnable;

public:

  berryObjectMacro( WorkbenchTestable);

  /**
   * Constructs a new workbench testable object.
   */
  WorkbenchTestable();

  /**
   * Initializes the workbench testable with the display and workbench,
   * and notifies all listeners that the tests can be run.
   *
   * @param display the display
   * @param workbench the workbench
   */
  void Init(Display* display, IWorkbench* workbench);

  /**
   * The <code>WorkbenchTestable</code> implementation of this
   * <code>TestableObject</code> method ensures that the workbench
   * has been set.
   */
  void TestingStarting() override;

  /**
   * The <code>WorkbenchTestable</code> implementation of this
   * <code>TestableObject</code> method flushes the event queue,
   * runs the test in a <code>syncExec</code>, then flushes the
   * event queue again.
   */
  void RunTest(Poco::Runnable* testRunnable) override;

  /**
   * The <code>WorkbenchTestable</code> implementation of this
   * <code>TestableObject</code> method flushes the event queue,
   * then closes the workbench.
   */
  void TestingFinished() override;

private:

  /**
   * Waits for the early startup job to complete.
   */
  //    void WaitForEarlyStartup() {
  //      try {
  //        Job::GetJobManager()->Join(Workbench::EARLY_STARTUP_FAMILY, 0);
  //      } catch (OperationCanceledException e) {
  //        // ignore
  //      } catch (InterruptedException e) {
  //        // ignore
  //      }
  //    }
};

}

#endif /* BERRYWORKBENCHTESTABLE_H_ */
