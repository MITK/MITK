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

#include "cherryUITestCase.h"

#include <cherryPlatformUI.h>
#include <cherryUIException.h>

#include "../util/cherryEmptyPerspective.h"

namespace cherry {

 IAdaptable* UITestCase::GetPageInput() {
    return 0;
  }

   UITestCase::UITestCase(const std::string& testName)
   : CppUnit::TestCase(testName)
   {
        //    ErrorDialog.NO_UI = true;
        fWorkbench = PlatformUI::GetWorkbench();
    }

   void UITestCase::failexc(const std::string& message, const std::exception& e,
      long lineNumber, const std::string& fileName ) {
     //TODO IStatus
    // If the exception is a CoreException with a multistatus
    // then print out the multistatus so we can see all the info.
//    if (e instanceof CoreException) {
//      IStatus status = ((CoreException) e).getStatus();
//      write(status, 0);
//    } else
//      e.printStackTrace();
    fail(message + ": " + e.what(), lineNumber, fileName);
  }

    IWorkbenchWindow::Pointer UITestCase::OpenTestWindow() {
        return OpenTestWindow(EmptyPerspective::PERSP_ID);
    }

    IWorkbenchWindow::Pointer UITestCase::OpenTestWindow(const std::string& perspectiveId) {
    try {
      IWorkbenchWindow::Pointer window = fWorkbench->OpenWorkbenchWindow(
          perspectiveId, GetPageInput());
      WaitOnShell(window->GetShell());
      return window;
    } catch (WorkbenchException& e) {
      failmsg(e.displayText());
      return IWorkbenchWindow::Pointer(0);
    }
  }

    void UITestCase::CloseAllTestWindows() {
//        Iterator iter = new ArrayList(testWindows).iterator();
//        while (iter.hasNext()) {
//            IWorkbenchWindow win = (IWorkbenchWindow) iter.next();
//            win.close();
//        }
//        testWindows.clear();
    }

    IWorkbenchPage::Pointer UITestCase::OpenTestPage(IWorkbenchWindow::Pointer win) {
//        IWorkbenchPage[] pages = openTestPage(win, 1);
//        if (pages != null)
//            return pages[0];
//        else
            return IWorkbenchPage::Pointer(0);
    }

    std::vector<IWorkbenchPage::Pointer> UITestCase::OpenTestPage(IWorkbenchWindow::Pointer win, int pageTotal) {
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
      return std::vector<IWorkbenchPage::Pointer>();
    }

    void UITestCase::CloseAllPages(IWorkbenchWindow::Pointer window) {
//        IWorkbenchPage[] pages = window.getPages();
//        for (int i = 0; i < pages.length; i++)
//            pages[i].close();
    }

    void UITestCase::Trace(const std::string& msg) {
        std::cerr << msg << std::endl;
    }

    void UITestCase::setUp() {
      CppUnit::TestCase::setUp();
      Trace("----- " + this->name());
        Trace(this->name() + ": setUp...");
        AddWindowListener();
        DoSetUp();

    }

    void UITestCase::DoSetUp() {
        // do nothing.
    }

    void UITestCase::tearDown() {
        CppUnit::TestCase::tearDown();
        Trace(this->name() + ": tearDown...\n");
        RemoveWindowListener();
        DoTearDown();
    }

    void UITestCase::DoTearDown() {
        ProcessEvents();
        CloseAllTestWindows();
        ProcessEvents();
    }

     void UITestCase::ProcessEvents() {
//        Display display = PlatformUI.getWorkbench().getDisplay();
//        if (display != null)
//            while (display.readAndDispatch())
//                ;
    }

    void UITestCase::ManageWindows(bool manage) {
        windowListener->SetEnabled(manage);
    }

    IWorkbench* UITestCase::GetWorkbench() {
        return fWorkbench;
    }

    UITestCase::TestWindowListener::TestWindowListener(std::list<IWorkbenchWindow::Pointer>& testWindows)
    : enabled(true), testWindows(testWindows)
    {}

    void UITestCase::TestWindowListener::SetEnabled(bool enabled) {
            this->enabled = enabled;
        }

        void UITestCase::TestWindowListener::WindowActivated(IWorkbenchWindow::Pointer window) {
            // do nothing
        }

        void UITestCase::TestWindowListener::WindowDeactivated(IWorkbenchWindow::Pointer window) {
            // do nothing
        }

        void UITestCase::TestWindowListener::WindowClosed(IWorkbenchWindow::Pointer window) {
            if (enabled)
                testWindows.remove(window);
        }

        void UITestCase::TestWindowListener::WindowOpened(IWorkbenchWindow::Pointer window) {
            if (enabled)
                testWindows.push_back(window);
        }

   void UITestCase::Indent(std::ostream& output, unsigned int indent) {
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

    void UITestCase::AddWindowListener() {
        windowListener = new TestWindowListener(testWindows);
        fWorkbench->AddWindowListener(windowListener);
    }

    void UITestCase::RemoveWindowListener() {
        if (windowListener) {
            fWorkbench->RemoveWindowListener(windowListener);
        }
    }

  void UITestCase::WaitOnShell(Shell::Pointer shell) {

    ProcessEvents();
//    long endTime = System.currentTimeMillis() + 5000;
//
//    while (shell.getDisplay().getActiveShell() != shell
//        && System.currentTimeMillis() < endTime) {
//      processEvents();
//    }
  }

}
