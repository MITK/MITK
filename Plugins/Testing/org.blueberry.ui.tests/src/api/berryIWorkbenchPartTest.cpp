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

#include "berryIWorkbenchPartTest.h"

namespace berry
{

void IWorkbenchPartTest::DoSetUp()
{
  UITestCase::DoSetUp();
  fWindow = OpenTestWindow();
  fPage = fWindow->GetActivePage();
}

void IWorkbenchPartTest::DoTearDown()
{
  fPage = 0;
  fWindow = 0;
  UITestCase::DoTearDown();
}

IWorkbenchPartTest::IWorkbenchPartTest(const std::string& testName) :
  UITestCase(testName)
{

}

void IWorkbenchPartTest::TestOpenAndClose()
{
  this->IgnoreLeakingObjects();

  CallHistory::Pointer history;
  std::vector<std::string> callOrder;
  {
    // Open a part.
    MockWorkbenchPart::Pointer part = OpenPart(fPage);
    assert(part->IsSiteInitialized());
    history = part->GetCallHistory();

    callOrder.push_back("SetInitializationData");
    callOrder.push_back("Init");
    callOrder.push_back("CreatePartControl");
    callOrder.push_back("SetFocus");

    assert(history->VerifyOrder(callOrder));

    // Close the part.
    ClosePart(fPage, part);
  }

  callOrder.push_back("WidgetDisposed");
  callOrder.push_back("PartDestructor");
  assert(history->VerifyOrder(callOrder));
}

void IWorkbenchPartTest::TestOpenAndWindowClose()
{
  this->IgnoreLeakingObjects();

  CallHistory::Pointer history;
  std::vector<std::string> callOrder;
  {
    // Open a new window
    IWorkbenchWindow::Pointer newWindow = OpenTestWindow();

    // Open a part.
    MockWorkbenchPart::Pointer part = OpenPart(newWindow->GetActivePage());
    assert(part->IsSiteInitialized());
    history = part->GetCallHistory();

    callOrder.push_back("SetInitializationData");
    callOrder.push_back("Init");
    callOrder.push_back("CreatePartControl");
    callOrder.push_back("SetFocus");

    assert(history->VerifyOrder(callOrder));

    // Close the window.
    newWindow->Close();
  }

  // The Qt widget for the window is deleted
  // asynchronously, so the "WidgetDisposed"
  // call may come after the part destructor
  //callOrder.push_back("WidgetDisposed");
  callOrder.push_back("PartDestructor");
  assert(history->VerifyOrder(callOrder));
}

}
