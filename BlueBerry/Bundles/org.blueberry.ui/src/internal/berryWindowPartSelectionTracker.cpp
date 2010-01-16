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

#include "berryWindowPartSelectionTracker.h"

#include "../berryIWorkbenchWindow.h"
#include "../berryIWorkbenchPage.h"

namespace berry
{

void WindowPartSelectionTracker::SetWindow(
    SmartPointer<IWorkbenchWindow> window)
{
  fWindow = window;
}

WindowPartSelectionTracker::WindowPartSelectionTracker(SmartPointer<IWorkbenchWindow> window,
    const std::string& partId) :
  AbstractPartSelectionTracker(partId), selListener(
      new SelectionChangedAdapter<WindowPartSelectionTracker> (this,
          &WindowPartSelectionTracker::FireSelection)), postSelListener(
      new SelectionChangedAdapter<WindowPartSelectionTracker> (this,
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
  return fWindow;
}

}
