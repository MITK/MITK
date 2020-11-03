/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
    const QString& partId) :
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
  page->RemoveSelectionListener(GetPartId(), selListener.data());
  page->RemovePostSelectionListener(GetPartId(), postSelListener.data());
}

void WindowPartSelectionTracker::PageOpened(SmartPointer<IWorkbenchPage> page)
{
  page->AddSelectionListener(GetPartId(), selListener.data());
  page->AddPostSelectionListener(GetPartId(), postSelListener.data());
}

ISelection::ConstPointer WindowPartSelectionTracker::GetSelection()
{
  IWorkbenchPage::Pointer page = GetWindow()->GetActivePage();
  if (page)
  {
    return page->GetSelection(GetPartId());
  }
  return ISelection::ConstPointer(nullptr);
}

SmartPointer<IWorkbenchWindow> WindowPartSelectionTracker::GetWindow()
{
  return IWorkbenchWindow::Pointer(fWindow);
}

}
