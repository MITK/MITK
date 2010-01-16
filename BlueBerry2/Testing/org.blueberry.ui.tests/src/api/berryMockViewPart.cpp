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

#include "berryMockViewPart.h"

namespace berry
{

const std::string MockViewPart::ID = "org.blueberry.ui.tests.api.MockViewPart";
const std::string MockViewPart::ID2 = MockViewPart::ID + "2";
const std::string MockViewPart::ID3 = MockViewPart::ID + "3";
const std::string MockViewPart::ID4 = MockViewPart::ID + "4";
const std::string MockViewPart::IDMULT = MockViewPart::ID + "Mult";
const std::string MockViewPart::NAME = "Mock View 1";

IViewSite::Pointer MockViewPart::GetViewSite()
{
  return GetSite().Cast<IViewSite> ();
}

void MockViewPart::Init(IViewSite::Pointer site, IMemento::Pointer memento)
    throw (PartInitException)
{
  SetSite(site);
  callTrace->Add("Init");
  SetSiteInitialized();
  //AddToolbarContributionItem();
}

void MockViewPart::CreatePartControl(void* parent)
{
  MockWorkbenchPart::CreatePartControl(parent);

  //        Button addAction = new Button(parent, SWT.PUSH);
  //        addAction.setText("Add Action to Tool Bar");
  //        addAction.addSelectionListener(new SelectionAdapter() {
  //      public void widgetSelected(SelectionEvent e) {
  //        IActionBars bars = getViewSite().getActionBars();
  //        bars.getToolBarManager().add(new DummyAction());
  //        bars.updateActionBars();
  //      }
  //    });
  //
  //        Button removeAction = new Button(parent, SWT.PUSH);
  //        removeAction.setText("Remove Action from Tool Bar");
  //        removeAction.addSelectionListener(new SelectionAdapter() {
  //      public void widgetSelected(SelectionEvent e) {
  //        IActionBars bars = getViewSite().getActionBars();
  //        IToolBarManager tbm = bars.getToolBarManager();
  //        IContributionItem[] items = tbm.getItems();
  //        if (items.length > 0) {
  //          IContributionItem item = items[items.length-1];
  //          if (item instanceof ActionContributionItem) {
  //            if (((ActionContributionItem) item).getAction() instanceof DummyAction) {
  //              tbm.remove(item);
  //              bars.updateActionBars();
  //            }
  //          }
  //        }
  //      }
  //    });
}

MockViewPart::~MockViewPart()
{
  //    // Test for bug 94457: The contribution items must still be in the toolbar manager at the
  //    // time the part is disposed. (Changing this behavior would be a breaking change for some
  //    // clients).
  //    Assert.assertTrue(
  //        "Contribution items should not be removed from the site until after the part is disposed",
  //        getViewSite().getActionBars().getToolBarManager().find(
  //            toolbarItem.getId()) == toolbarItem);
}

void MockViewPart::ToolbarContributionItemWidgetDisposed()
{
  callTrace->Add("ToolbarContributionItemWidgetDisposed");
}

void MockViewPart::ToolbarContributionItemDisposed()
{
  callTrace->Add("ToolbarContributionItemDisposed");
}

void MockViewPart::SaveState(IMemento::Pointer memento)
{
  // do nothing
}

IWorkbenchPartSite::Pointer MockViewPart::GetSite() const
{
  return MockWorkbenchPart::GetSite();
}

std::string MockViewPart::GetPartName() const
{
  return MockWorkbenchPart::GetPartName();
}

std::string MockViewPart::GetContentDescription() const
{
  return MockWorkbenchPart::GetContentDescription();
}

std::string MockViewPart::GetTitleToolTip() const
{
  return MockWorkbenchPart::GetTitleToolTip();
}

std::string MockViewPart::GetPartProperty(const std::string& key) const
{
  return MockWorkbenchPart::GetPartProperty(key);
}

void MockViewPart::SetPartProperty(const std::string& key,
    const std::string& value)
{
  MockWorkbenchPart::SetPartProperty(key, value);
}

const std::map<std::string, std::string>& MockViewPart::GetPartProperties() const
{
  return MockWorkbenchPart::GetPartProperties();
}

}
