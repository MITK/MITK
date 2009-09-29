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

#include "cherryWWinPartService.h"

#include "../cherryIWorkbenchWindow.h"
#include "cherryWorkbenchPage.h"

namespace cherry
{

struct WWinListener: public IPartListener
{ //, IPageChangedListener {

  WWinListener(WWinPartService* wwps) :
    wwps(wwps)
  {
  }

  void PartActivated(IWorkbenchPartReference::Pointer ref)
  {
    wwps->UpdateActivePart();
  }

  void PartBroughtToTop(IWorkbenchPartReference::Pointer ref)
  {
    wwps->partService.FirePartBroughtToTop(ref);
  }

  void PartClosed(IWorkbenchPartReference::Pointer ref)
  {
    wwps->partService.FirePartClosed(ref);
  }

  void PartDeactivated(IWorkbenchPartReference::Pointer ref)
  {
    wwps->UpdateActivePart();
  }

  void PartOpened(IWorkbenchPartReference::Pointer ref)
  {
    wwps->partService.FirePartOpened(ref);
  }

  void PartHidden(IWorkbenchPartReference::Pointer ref)
  {
    wwps->partService.FirePartHidden(ref);
  }

  void PartVisible(IWorkbenchPartReference::Pointer ref)
  {
    wwps->partService.FirePartVisible(ref);
  }

  void PartInputChanged(IWorkbenchPartReference::Pointer ref)
  {
    wwps->partService.FirePartInputChanged(ref);
  }

  //        void PageChanged(PageChangedEvent::Pointer event) {
  //          partService.firePageChanged(event);
  //        }

private:
  WWinPartService* wwps;
};

WWinPartService::WWinPartService(SmartPointer<IWorkbenchWindow> window) :
  partService("", ""), selectionService(window), partListener(new WWinListener(
      this))
{

}

void WWinPartService::AddPartListener(IPartListener::Pointer l)
{
  partService.AddPartListener(l);
}

void WWinPartService::RemovePartListener(IPartListener::Pointer l)
{
  partService.RemovePartListener(l);
}

IWorkbenchPart::Pointer WWinPartService::GetActivePart()
{
  return partService.GetActivePart();
}

void WWinPartService::UpdateActivePart()
{
  IWorkbenchPartReference::Pointer activeRef;
  IWorkbenchPart::Pointer activePart;

  if (activePage)
  {
    activePart = activePage->GetActivePart();
    activeRef = activePage->GetActivePartReference();
  }

  partService.SetActivePart(activeRef);
  selectionService.SetActivePart(activePart);
}

IWorkbenchPartReference::Pointer WWinPartService::GetActivePartReference()
{
  return partService.GetActivePartReference();
}

ISelectionService* WWinPartService::GetSelectionService()
{
  return &selectionService;
}

void WWinPartService::PageActivated(SmartPointer<IWorkbenchPage> newPage)
{
  // Optimize.
  if (newPage == activePage)
  {
    return;
  }

  // Fire events in the following order:

  // 1. For each open part in the new page, open it and then (if applicable) make it visible
  // 2. Deactivate old active part
  // 3. Activate the new active part
  // 4. For each open part in the old page, make it invisible then close it

  // Hook listener on the new page.
  if (newPage)
  {
    std::vector<IWorkbenchPartReference::Pointer> refs(newPage.Cast<
        WorkbenchPage> ()->GetOpenParts());

    for (std::size_t i = 0; i < refs.size(); i++)
    {
      IWorkbenchPartReference::Pointer reference = refs[i];

      partService.FirePartOpened(reference);

      IWorkbenchPart::Pointer part = reference->GetPart(false);
      if (part && newPage->IsPartVisible(part))
      {
        partService.FirePartVisible(reference);
      }
    }

    partService.SetActivePart(newPage->GetActivePartReference());
    selectionService.SetActivePart(newPage->GetActivePart());
  }
  else
  {
    partService.SetActivePart(IWorkbenchPartReference::Pointer(0));
    selectionService.SetActivePart(IWorkbenchPart::Pointer(0));
  }

  // Unhook listener from the old page.
  Reset();

  // Update active page.
  activePage = newPage;

  if (newPage)
  {
    newPage->AddPartListener(partListener);
  }

}

void WWinPartService::PageClosed(SmartPointer<IWorkbenchPage> page)
{
  // Unhook listener from the old page.
  if (page == activePage)
  {
    Reset();
  }
}

void WWinPartService::PageOpened(SmartPointer<IWorkbenchPage> page)
{
  PageActivated(page);
}

void WWinPartService::Reset()
{
  IWorkbenchPage::Pointer tempPage = activePage;
  activePage = 0;
  if (tempPage)
  {
    WorkbenchPage::Pointer page = tempPage.Cast<WorkbenchPage> ();

    std::vector<IWorkbenchPartReference::Pointer> refs(page->GetOpenParts());

    for (std::size_t i = 0; i < refs.size(); i++)
    {
      IWorkbenchPartReference::Pointer reference = refs[i];

      if (page->IsPartVisible(reference))
      {
        partService.FirePartHidden(reference);
      }

      partService.FirePartClosed(reference);
    }

    tempPage->RemovePartListener(partListener);
  }

}

}
