/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryWorkbenchPagePartList.h"
#include "berryPartPane.h"

namespace berry
{

void WorkbenchPagePartList::FirePartOpened(IWorkbenchPartReference::Pointer part)
{
  partService.FirePartOpened(part);
}

void WorkbenchPagePartList::FirePartClosed(IWorkbenchPartReference::Pointer part)
{
  partService.FirePartClosed(part);
}

void WorkbenchPagePartList::FirePartAdded(IWorkbenchPartReference::Pointer  /*part*/)
{
  // TODO: There is no listener for workbench page additions yet
}

void WorkbenchPagePartList::FirePartRemoved(
    IWorkbenchPartReference::Pointer  /*part*/)
{
  // TODO: There is no listener for workbench page removals yet
}

void WorkbenchPagePartList::FireActiveEditorChanged(
    IWorkbenchPartReference::Pointer ref)
{
  if (ref.IsNotNull())
  {
    this->FirePartBroughtToTop(ref);
  }
}

void WorkbenchPagePartList::FireActivePartChanged(
    IWorkbenchPartReference::Pointer /*oldRef*/,
    IWorkbenchPartReference::Pointer newRef)
{
  partService.SetActivePart(newRef);

  IWorkbenchPart::Pointer realPart = newRef == 0 ? IWorkbenchPart::Pointer(nullptr) : newRef->GetPart(false);
  selectionService->SetActivePart(realPart);
}

void WorkbenchPagePartList::FirePartHidden(IWorkbenchPartReference::Pointer ref)
{
  partService.FirePartHidden(ref);
}

void WorkbenchPagePartList::FirePartVisible(IWorkbenchPartReference::Pointer ref)
{
  partService.FirePartVisible(ref);
}

void WorkbenchPagePartList::FirePartInputChanged(
    IWorkbenchPartReference::Pointer ref)
{
  partService.FirePartInputChanged(ref);
}

WorkbenchPagePartList::WorkbenchPagePartList(PageSelectionService* selService) :
  selectionService(selService), partService("", "")
{

}

IPartService* WorkbenchPagePartList::GetPartService()
{
  return &partService;
}

void WorkbenchPagePartList::FirePartBroughtToTop(
    IWorkbenchPartReference::Pointer ref)
{
  partService.FirePartBroughtToTop(ref);
}

}
