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

#ifndef BERRYUITESTCASE_H_
#define BERRYUITESTCASE_H_

#include <berryTestCase.h>

#include <berryIWindowListener.h>
#include <berryIWorkbench.h>
#include <berryIWorkbenchPage.h>

#include <org_blueberry_uitest_Export.h>

#include <list>

namespace berry
{

/**
 * <code>UITestCase</code> is a useful super class for most
 * UI tests cases.  It contains methods to create new windows
 * and pages.  It will also automatically close the test
 * windows when the tearDown method is called.
 */
class BERRY_UITEST_EXPORT UITestCase: public TestCase
{

public:

  /**
   * Returns the workbench page input to use for newly created windows.
   *
   * @return the page input to use for newly created windows
   * @since 3.1
   */
  static IAdaptable* GetPageInput();

  UITestCase(const QString& testName);

  /**
   * Sets up the fixture, for example, open a network connection.
   * This method is called before a test is executed.
   * The default implementation does nothing.
   * Subclasses may extend.
   */
  virtual void DoSetUp();

  /**
   * Tears down the fixture, for example, close a network connection.
   * This method is called after a test is executed.
   * The default implementation closes all test windows, processing events both before
   * and after doing so.
   * Subclasses may extend.
   */
  virtual void DoTearDown();

  /**
   * Fails the test due to the given throwable.
   */
  void failexc(const QString& message, const std::exception& e,
               long lineNumber = -1,
               const QString& fileName = "unknown");

  /**
   * Open a test window with the empty perspective.
   */
  IWorkbenchWindow::Pointer OpenTestWindow();

  /**
   * Open a test window with the provided perspective.
   */
  IWorkbenchWindow::Pointer OpenTestWindow(const QString& perspectiveId);

  /**
   * Close all test windows.
   */
  void CloseAllTestWindows();

  /**
   * Open a test page with the empty perspective in a window.
   */
  IWorkbenchPage::Pointer OpenTestPage(const IWorkbenchWindow::Pointer& win);

  /**
   * Open "n" test pages with the empty perspective in a window.
   */
  QList<IWorkbenchPage::Pointer> OpenTestPage(
      const IWorkbenchWindow::Pointer& win, int pageTotal);

  /**
   * Close all pages within a window.
   */
  void CloseAllPages(const IWorkbenchWindow::Pointer& window);

protected:

  /**
   * Outputs a trace message to the trace output device, if enabled.
   * By default, trace messages are sent to <code>System.out</code>.
   *
   * @param msg the trace message
   */
  void Trace(const QString& msg);

  /**
   * Simple implementation of setUp. Subclasses are prevented
   * from overriding this method to maintain logging consistency.
   * DoSetUp() should be overriden instead.
   */
  void setUp();

  /**
   * Simple implementation of tearDown.  Subclasses are prevented
   * from overriding this method to maintain logging consistency.
   * DoTearDown() should be overriden instead.
   */
  void tearDown();

  static void ProcessEvents();

  /**
   * Set whether the window listener will manage opening and closing of created windows.
   */
  void ManageWindows(bool manage);

  /**
   * Returns the workbench.
   *
   * @return the workbench
   * @since 3.1
   */
  IWorkbench* GetWorkbench();

  class TestWindowListener: public IWindowListener
  {

  private:
    bool enabled;
    QList<IWorkbenchWindow::Pointer>& testWindows;

  public:

    berryObjectMacro(TestWindowListener)

    TestWindowListener(QList<IWorkbenchWindow::Pointer>& testWindows);

    void SetEnabled(bool enabled);

    void WindowActivated(const IWorkbenchWindow::Pointer& window);

    void WindowDeactivated(const IWorkbenchWindow::Pointer& window);

    void WindowClosed(const IWorkbenchWindow::Pointer& window);

    void WindowOpened(const IWorkbenchWindow::Pointer& window);
  };

  IWorkbench* fWorkbench;

private:

  QList<IWorkbenchWindow::Pointer> testWindows;

  QScopedPointer<TestWindowListener> windowListener;

  static void Indent(std::ostream& output, unsigned int indent);

  //static void Write(IStatus status, unsigned int indent);

  /**
   * Adds a window listener to the workbench to keep track of
   * opened test windows.
   */
  void AddWindowListener();

  /**
   * Removes the listener added by <code>addWindowListener</code>.
   */
  void RemoveWindowListener();

  /**
   * Try and process events until the new shell is the active shell. This may
   * never happen, so time out after a suitable period.
   *
   * @param shell
   *            the shell to wait on
   * @since 3.2
   */
  void WaitOnShell(Shell::Pointer shell);

};

#define failuimsg(msg, exc) \
  (this->failexc(msg, exc, __LINE__, __FILE__))

}

#endif /* BERRYUITESTCASE_H_ */
