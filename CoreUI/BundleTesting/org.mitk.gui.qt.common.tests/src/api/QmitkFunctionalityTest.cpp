/*=========================================================================

 Program:   Medical Imaging & Interaction Toolkit
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

#include "QmitkFunctionalityTest.h"

#include <CppUnit/TestSuite.h>
#include <CppUnit/TestCaller.h>

#include "QmitkMockFunctionality.h"

QmitkFunctionalityTest::QmitkFunctionalityTest(const std::string& testName)
 : berry::UITestCase(testName)
{

}

CppUnit::Test* QmitkFunctionalityTest::Suite()
{
  CppUnit::TestSuite* suite = new CppUnit::TestSuite("QmitkFunctionalityTest");

  CppUnit_addTest(suite, QmitkFunctionalityTest, TestOpenAndClose);

  return suite;
}

void QmitkFunctionalityTest::DoSetUp()
{
  UITestCase::DoSetUp();
  fWindow = OpenTestWindow();
  fPage = fWindow->GetActivePage();
}

void QmitkFunctionalityTest::DoTearDown()
{
  fPage = 0;
  fWindow = 0;
  UITestCase::DoTearDown();
}

void QmitkFunctionalityTest::TestOpenAndClose()
{
  this->IgnoreLeakingObjects();

  berry::CallHistory::Pointer history;
  std::vector<std::string> callOrder;
  {
    // Open a functionality.
    QmitkMockFunctionality::Pointer part = fPage->ShowView(QmitkMockFunctionality::ID).Cast<QmitkMockFunctionality> ();
    history = part->GetCallHistory();

    //callOrder.push_back("SetInitializationData");
    //callOrder.push_back("Init");
    callOrder.push_back("CreateQtPartControl");
    callOrder.push_back("SetFocus");

    assert(history->VerifyOrder(callOrder));

    // Close the part.
    fPage->HideView(part.Cast<berry::IViewPart>());
  }

  //callOrder.push_back("WidgetDisposed");
  callOrder.push_back("FunctionalityDestructor");
  assert(history->VerifyOrder(callOrder));
}

void QmitkFunctionalityTest::TestOpenAndWindowClose()
{
  this->IgnoreLeakingObjects();

  berry::CallHistory::Pointer history;
  std::vector<std::string> callOrder;
  {
    // Open a new window
    berry::IWorkbenchWindow::Pointer newWindow = OpenTestWindow();

    // Open a part.
    QmitkMockFunctionality::Pointer part = newWindow->GetActivePage()->ShowView(QmitkMockFunctionality::ID).Cast<QmitkMockFunctionality> ();;
    history = part->GetCallHistory();

    //callOrder.push_back("SetInitializationData");
    //callOrder.push_back("Init");
    callOrder.push_back("CreateQtPartControl");
    callOrder.push_back("SetFocus");

    assert(history->VerifyOrder(callOrder));

    // Close the window.
    newWindow->Close();
  }

  //callOrder.push_back("WidgetDisposed");
  callOrder.push_back("FunctionalityDestructor");
  assert(history->VerifyOrder(callOrder));
}
