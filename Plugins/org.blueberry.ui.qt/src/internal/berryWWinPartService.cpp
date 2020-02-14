/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryWWinPartService.h"

#include "berryIWorkbenchWindow.h"
#include "berryWorkbenchPage.h"

namespace berry
{

struct WWinListener: public IPartListener
{ //, IPageChangedListener {

  WWinListener(WWinPartService* wwps) :
    wwps(wwps)
  {
  }

  Events::Types GetPartEventTypes() const override
  {
    return Events::ALL;
  }

  void PartActivated(const IWorkbenchPartReference::Pointer&  /*ref*/) override
  {
    wwps->UpdateActivePart();
  }

  void PartBroughtToTop(const IWorkbenchPartReference::Pointer& ref) override
  {
    wwps->partService.FirePartBroughtToTop(ref);
  }

  void PartClosed(const IWorkbenchPartReference::Pointer& ref) override
  {
    wwps->partService.FirePartClosed(ref);
  }

  void PartDeactivated(const IWorkbenchPartReference::Pointer& /*ref*/) override
  {
    wwps->UpdateActivePart();
  }

  void PartOpened(const IWorkbenchPartReference::Pointer& ref) override
  {
    wwps->partService.FirePartOpened(ref);
  }

  void PartHidden(const IWorkbenchPartReference::Pointer& ref) override
  {
    wwps->partService.FirePartHidden(ref);
  }

  void PartVisible(const IWorkbenchPartReference::Pointer& ref) override
  {
    wwps->partService.FirePartVisible(ref);
  }

  void PartInputChanged(const IWorkbenchPartReference::Pointer& ref) override
  {
    wwps->partService.FirePartInputChanged(ref);
  }

  //        void PageChanged(PageChangedEvent::Pointer event) {
  //          partService.firePageChanged(event);
  //        }

private:
  WWinPartService* wwps;
};

WWinPartService::WWinPartService(IWorkbenchWindow* window) :
  partService("", ""), selectionService(window), activePage(nullptr),
  partListener(new WWinListener(
      this))
{

}

void WWinPartService::AddPartListener(IPartListener* l)
{
  partService.AddPartListener(l);
}

void WWinPartService::RemovePartListener(IPartListener* l)
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

ISelectionService* WWinPartService::GetSelectionService() const
{
  return const_cast<WindowSelectionService*>(&selectionService);
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
    QList<IWorkbenchPartReference::Pointer> refs(newPage.Cast<
        WorkbenchPage> ()->GetOpenParts());

    for (int i = 0; i < refs.size(); i++)
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
    partService.SetActivePart(IWorkbenchPartReference::Pointer(nullptr));
    selectionService.SetActivePart(IWorkbenchPart::Pointer(nullptr));
  }

  // Unhook listener from the old page.
  Reset();

  // Update active page.
  activePage = newPage.GetPointer();

  if (newPage)
  {
    newPage->AddPartListener(partListener.data());
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
  IWorkbenchPage* tempPage = activePage;
  activePage = nullptr;
  if (tempPage)
  {
    WorkbenchPage* page = dynamic_cast<WorkbenchPage*>(tempPage);

    QList<IWorkbenchPartReference::Pointer> refs(page->GetOpenParts());

    for (int i = 0; i < refs.size(); i++)
    {
      IWorkbenchPartReference::Pointer reference = refs[i];

      if (page->IsPartVisible(reference))
      {
        partService.FirePartHidden(reference);
      }

      partService.FirePartClosed(reference);
    }

    tempPage->RemovePartListener(partListener.data());
  }

}

}
