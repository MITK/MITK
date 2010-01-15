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

#ifndef CHERRYUITESTCASE_H_
#define CHERRYUITESTCASE_H_

#include <cherryTestCase.h>

#include <cherryIWindowListener.h>
#include <cherryIWorkbench.h>
#include <cherryIWorkbenchPage.h>

#include "../cherryUITestDll.h"

#include <list>

namespace cherry
{

/**
 * <code>UITestCase</code> is a useful super class for most
 * UI tests cases.  It contains methods to create new windows
 * and pages.  It will also automatically close the test
 * windows when the tearDown method is called.
 */
class CHERRY_UITEST_EXPORT UITestCase: public TestCase
{

public:

  /**
   * Returns the workbench page input to use for newly created windows.
   *
   * @return the page input to use for newly created windows
   * @since 3.1
   */
  static IAdaptable* GetPageInput();

  UITestCase(const std::string& testName);

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
  void failexc(const std::string& message, const std::exception& e,
      long lineNumber = CppUnit::CppUnitException::CPPUNIT_UNKNOWNLINENUMBER,
      const std::string& fileName =
          CppUnit::CppUnitException::CPPUNIT_UNKNOWNFILENAME);

  /**
   * Open a test window with the empty perspective.
   */
  IWorkbenchWindow::Pointer OpenTestWindow();

  /**
   * Open a test window with the provided perspective.
   */
  IWorkbenchWindow::Pointer OpenTestWindow(const std::string& perspectiveId);

  /**
   * Close all test windows.
   */
  void CloseAllTestWindows();

  /**
   * Open a test page with the empty perspective in a window.
   */
  IWorkbenchPage::Pointer OpenTestPage(IWorkbenchWindow::Pointer win);

  /**
   * Open "n" test pages with the empty perspective in a window.
   */
  std::vector<IWorkbenchPage::Pointer> OpenTestPage(
      IWorkbenchWindow::Pointer win, int pageTotal);

  /**
   * Close all pages within a window.
   */
  void CloseAllPages(IWorkbenchWindow::Pointer window);

protected:

  /**
   * Outputs a trace message to the trace output device, if enabled.
   * By default, trace messages are sent to <code>System.out</code>.
   *
   * @param msg the trace message
   */
  void Trace(const std::string& msg);

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
    std::list<IWorkbenchWindow::Pointer>& testWindows;

  public:

    cherryObjectMacro(TestWindowListener)

    TestWindowListener(std::list<IWorkbenchWindow::Pointer>& testWindows);

    void SetEnabled(bool enabled);

    void WindowActivated(IWorkbenchWindow::Pointer window);

    void WindowDeactivated(IWorkbenchWindow::Pointer window);

    void WindowClosed(IWorkbenchWindow::Pointer window);

    void WindowOpened(IWorkbenchWindow::Pointer window);
  };

  IWorkbench* fWorkbench;

private:

  std::list<IWorkbenchWindow::Pointer> testWindows;

  TestWindowListener::Pointer windowListener;

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

#endif /* CHERRYUITESTCASE_H_ */
