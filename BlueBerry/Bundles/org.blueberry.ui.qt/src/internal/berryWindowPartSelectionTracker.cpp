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

#include "berryWindowPartSelectionTracker.h"

#include "berryIWorkbenchWindow.h"
#include "berryIWorkbenchPage.h"

namespace berry
{

void WindowPartSelectionTracker::SetWindow(
    IWorkbenchWindow* window)
{
  fWindow = window;
}

WindowPartSelectionTracker::WindowPartSelectionTracker(IWorkbenchWindow* window,
    const std::string& partId) :
  AbstractPartSelectionTracker(partId), selListener(
      new NullSelectionChangedAdapter<WindowPartSelectionTracker> (this,
          &WindowPartSelectionTracker::FireSelection)), postSelListener(
      new NullSelectionChangedAdapter<WindowPartSelectionTracker> (this,
          &WindowPartSelectionTracker::FirePostSelection))
{
  SetWindow(window);
  //window.addPageListener(this);
  IWorkbenchPage::Pointer page = window->GetActivePage();
  if (page)
  {
    PageOpened(page);
  }
}

void WindowPartSelectionTracker::PageActivated(
    SmartPointer<IWorkbenchPage>  /*page*/)
{
}

void WindowPartSelectionTracker::PageClosed(SmartPointer<IWorkbenchPage> page)
{
  page->RemoveSelectionListener(GetPartId(), selListener);
  page->RemovePostSelectionListener(GetPartId(), postSelListener);
}

void WindowPartSelectionTracker::PageOpened(SmartPointer<IWorkbenchPage> page)
{
  page->AddSelectionListener(GetPartId(), selListener);
  page->AddPostSelectionListener(GetPartId(), postSelListener);
}

ISelection::ConstPointer WindowPartSelectionTracker::GetSelection()
{
  IWorkbenchPage::Pointer page = GetWindow()->GetActivePage();
  if (page)
  {
    return page->GetSelection(GetPartId());
  }
  return ISelection::ConstPointer(0);
}

SmartPointer<IWorkbenchWindow> WindowPartSelectionTracker::GetWindow()
{
  return IWorkbenchWindow::Pointer(fWindow);
}

}
