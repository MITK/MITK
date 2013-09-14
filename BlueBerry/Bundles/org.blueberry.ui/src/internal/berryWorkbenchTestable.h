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

  bool oldAutomatedMode;

  bool oldIgnoreErrors;

  Poco::Thread thread;

  class WorkbenchTestRunnable: public Poco::Runnable
  {
  public:

    WorkbenchTestRunnable(TestableObject* testable);

    void run();

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
  void TestingStarting();

  /**
   * The <code>WorkbenchTestable</code> implementation of this
   * <code>TestableObject</code> method flushes the event queue,
   * runs the test in a <code>syncExec</code>, then flushes the
   * event queue again.
   */
  void RunTest(Poco::Runnable* testRunnable);

  /**
   * The <code>WorkbenchTestable</code> implementation of this
   * <code>TestableObject</code> method flushes the event queue,
   * then closes the workbench.
   */
  void TestingFinished();

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
