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

#include "berryUITestCase.h"

#include <berryPlatformUI.h>
#include <berryUIException.h>

#include "util/berryEmptyPerspective.h"

#ifdef BLUEBERRY_DEBUG_SMARTPOINTER
#include <berryDebugUtil.h>
#endif

namespace berry
{

IAdaptable* UITestCase::GetPageInput()
{
  return 0;
}

UITestCase::UITestCase(const QString& testName) :
  TestCase(testName)
{
  //    ErrorDialog.NO_UI = true;
  fWorkbench = PlatformUI::GetWorkbench();
}

void UITestCase::failexc(const QString& message, const std::exception& e,
    long /*lineNumber*/, const QString& /*fileName*/)
{
  //TODO IStatus
  // If the exception is a CoreException with a multistatus
  // then print out the multistatus so we can see all the info.
  //    if (e instanceof CoreException) {
  //      IStatus status = ((CoreException) e).getStatus();
  //      write(status, 0);
  //    } else
  //      e.printStackTrace();
  CPPUNIT_FAIL(message.toStdString() + ": " + e.what());
}

IWorkbenchWindow::Pointer UITestCase::OpenTestWindow()
{
  return OpenTestWindow(EmptyPerspective::PERSP_ID);
}

IWorkbenchWindow::Pointer UITestCase::OpenTestWindow(
    const QString& perspectiveId)
{
  try
  {
    IWorkbenchWindow::Pointer window = fWorkbench->OpenWorkbenchWindow(
        perspectiveId, GetPageInput());
    WaitOnShell(window->GetShell());
    return window;
  } catch (const WorkbenchException& e)
  {
    CPPUNIT_FAIL(e.what());
    return IWorkbenchWindow::Pointer(0);
  }
}

void UITestCase::CloseAllTestWindows()
{
  while (!testWindows.empty())
  {
    testWindows.back()->Close();
    testWindows.pop_back();
  }
}

IWorkbenchPage::Pointer UITestCase::OpenTestPage(const IWorkbenchWindow::Pointer& /*win*/)
{
  //        IWorkbenchPage[] pages = openTestPage(win, 1);
  //        if (pages != null)
  //            return pages[0];
  //        else
  return IWorkbenchPage::Pointer(0);
}

QList<IWorkbenchPage::Pointer> UITestCase::OpenTestPage(
    const IWorkbenchWindow::Pointer&  /*win*/, int  /*pageTotal*/)
{
  //        try {
  //            IWorkbenchPage[] pages = new IWorkbenchPage[pageTotal];
  //            IAdaptable input = getPageInput();
  //
  //            for (int i = 0; i < pageTotal; i++) {
  //                pages[i] = win.openPage(EmptyPerspective.PERSP_ID, input);
  //            }
  //            return pages;
  //        } catch (WorkbenchException e) {
  //            fail();
  //            return null;
  //        }
  return QList<IWorkbenchPage::Pointer>();
}

void UITestCase::CloseAllPages(const IWorkbenchWindow::Pointer& /*window*/)
{
  //        IWorkbenchPage[] pages = window.getPages();
  //        for (int i = 0; i < pages.length; i++)
  //            pages[i].close();
}

void UITestCase::Trace(const QString& msg)
{
  qDebug() << msg;
}

void UITestCase::setUp()
{
  QString name = QString::fromStdString(this->getName());
  Trace("----- " + name);
  Trace(name + ": setUp...");
  AddWindowListener();
  berry::TestCase::setUp();
}

void UITestCase::DoSetUp()
{
  // do nothing.
}

void UITestCase::tearDown()
{
  Trace(QString::fromStdString(this->getName()) + ": tearDown...\n");
  RemoveWindowListener();
  berry::TestCase::tearDown();
}

void UITestCase::DoTearDown()
{
  ProcessEvents();
  CloseAllTestWindows();
  ProcessEvents();
}

void UITestCase::ProcessEvents()
{
  //        Display display = PlatformUI.getWorkbench().getDisplay();
  //        if (display != null)
  //            while (display.readAndDispatch())
  //                ;
}

void UITestCase::ManageWindows(bool manage)
{
  windowListener->SetEnabled(manage);
}

IWorkbench* UITestCase::GetWorkbench()
{
  return fWorkbench;
}

UITestCase::TestWindowListener::TestWindowListener(
    QList<IWorkbenchWindow::Pointer>& testWindows)
  : enabled(true), testWindows(testWindows)
{
}

void UITestCase::TestWindowListener::SetEnabled(bool enabled)
{
  this->enabled = enabled;
}

void UITestCase::TestWindowListener::WindowActivated(
    const IWorkbenchWindow::Pointer&  /*window*/)
{
  // do nothing
}

void UITestCase::TestWindowListener::WindowDeactivated(
    const IWorkbenchWindow::Pointer&  /*window*/)
{
  // do nothing
}

void UITestCase::TestWindowListener::WindowClosed(
    const IWorkbenchWindow::Pointer& window)
{
  if (enabled)
    testWindows.removeAll(window);
}

void UITestCase::TestWindowListener::WindowOpened(
    const IWorkbenchWindow::Pointer& window)
{
  if (enabled)
    testWindows.push_back(window);
}

void UITestCase::Indent(std::ostream& output, unsigned int indent)
{
  for (unsigned int i = 0; i < indent; i++)
    output << "  ";
}

//   void UITestCase::Write(IStatus status, unsigned int indent) {
//    PrintStream output = System.out;
//    indent(output, indent);
//    output.println("Severity: " + status.getSeverity());
//
//    indent(output, indent);
//    output.println("Plugin ID: " + status.getPlugin());
//
//    indent(output, indent);
//    output.println("Code: " + status.getCode());
//
//    indent(output, indent);
//    output.println("Message: " + status.getMessage());
//
//    if (status.getException() != null) {
//      indent(output, indent);
//      output.print("Exception: ");
//      status.getException().printStackTrace(output);
//    }
//
//    if (status.isMultiStatus()) {
//      IStatus[] children = status.getChildren();
//      for (int i = 0; i < children.length; i++)
//        write(children[i], indent + 1);
//    }
//  }

void UITestCase::AddWindowListener()
{
  windowListener.reset(new TestWindowListener(testWindows));
  fWorkbench->AddWindowListener(windowListener.data());
}

void UITestCase::RemoveWindowListener()
{
  if (windowListener)
  {
    fWorkbench->RemoveWindowListener(windowListener.data());
  }
}

void UITestCase::WaitOnShell(Shell::Pointer  /*shell*/)
{

  ProcessEvents();
  //    long endTime = System.currentTimeMillis() + 5000;
  //
  //    while (shell.getDisplay().getActiveShell() != shell
  //        && System.currentTimeMillis() < endTime) {
  //      processEvents();
  //    }
}

}
