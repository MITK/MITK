/*=========================================================================
 
 Program:   BlueBerry Platform
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

#include "berryIViewPartTest.h"

#include "berrySaveableMockViewPart.h"

#include <berryDebugBreakpointManager.h>

#include <CppUnit/TestSuite.h>

namespace berry
{

IViewPartTest::IViewPartTest(const std::string& testName) :
  IWorkbenchPartTest(testName)
{

}

CppUnit::Test* IViewPartTest::Suite()
{
  CppUnit::TestSuite* suite = new CppUnit::TestSuite("IViewPartTest");

  CppUnit_addTest(suite, IViewPartTest, TestOpenAndCloseSaveNotNeeded);
  CppUnit_addTest(suite, IViewPartTest, TestOpenAndClose);
  CppUnit_addTest(suite, IViewPartTest, TestOpenAndWindowClose);

  return suite;
}

void IViewPartTest::TestOpenAndCloseSaveNotNeeded()
{
  this->IgnoreLeakingObjects();
  //this->LeakDetailsOn();

  CallHistory::Pointer history;

  {
    // Open a part.
    SaveableMockViewPart::Pointer part = fPage->ShowView(
        SaveableMockViewPart::ID).Cast<SaveableMockViewPart> ();

    part->SetDirty(true);
    part->SetSaveNeeded(false);
    ClosePart(fPage, part);

    history = part->GetCallHistory();
  }

  // TODO: This verifies the 3.0 disposal order. However, there may be a bug here.
  // That is, it may be necessary to change this and dispose the contribution items
  // after the view's dispose method in order to ensure that the site is never returning
  // a disposed contribution item. See bug 94457 for details.
  std::vector<std::string> callOrder;
  callOrder.push_back("SetInitializationData");
  callOrder.push_back("Init");
  callOrder.push_back("CreatePartControl");
  callOrder.push_back("SetFocus");
  callOrder.push_back("IsSaveOnCloseNeeded");
  callOrder.push_back("WidgetDisposed");
  //callOrder.push_back("ToolbarContributionItemWidgetDisposed");
  //callOrder.push_back("ToolbarContributionItemDisposed");
  callOrder.push_back("PartDestructor");
  assert(history->VerifyOrder(callOrder));
  // TODO enable assert when saveable support is complete
  //assert(history->Contains("DoSave"));
}

MockWorkbenchPart::Pointer IViewPartTest::OpenPart(IWorkbenchPage::Pointer page)
{
  return page->ShowView(MockViewPart::ID).Cast<MockWorkbenchPart> ();
}

void IViewPartTest::ClosePart(IWorkbenchPage::Pointer page,
    MockWorkbenchPart::Pointer part)
{
  page->HideView(part.Cast<IViewPart> ());
}

}
