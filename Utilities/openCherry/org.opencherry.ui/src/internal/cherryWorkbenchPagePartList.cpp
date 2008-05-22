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

#include "cherryWorkbenchPagePartList.h"

namespace cherry
{

IWorkbenchPartReference::Pointer WorkbenchPagePartList::GetActivePartReference()
{
  return activePartReference;
}

IEditorReference::Pointer WorkbenchPagePartList::GetActiveEditorReference()
{
  return activeEditorReference;
}

IEditorPart::Pointer WorkbenchPagePartList::GetActiveEditor()
{
  return activeEditorReference == null ? null : activeEditorReference
  .getEditor(false);
}

IWorkbenchPart::Pointer WorkbenchPagePartList::GetActivePart()
{
  return activePartReference == null ? null : activePartReference
  .getPart(false);
}

void WorkbenchPagePartList::AddPart(WorkbenchPartReference::Pointer ref)
{
  Assert.isNotNull(ref);

  ref.addInternalPropertyListener(listener);

  // parts.add(ref);
  firePartAdded(ref);

  // If this part is already open, fire the "part opened" event
  // immediately
  if (ref.getPart(false) != null)
  {
    partOpened(ref);
  }

  // If this part is already visible, fire the visibility event
  // immediately
  if (ref.getVisible())
  {
    partVisible(ref);
  }
}

/**
 * Sets the active part.
 * 
 * @param ref
 */
void WorkbenchPagePartList::SetActivePart(IWorkbenchPartReference::Pointer ref)
{
  if (ref == activePartReference)
  {
    return;
  }

  IWorkbenchPartReference oldPart = activePartReference;

  // A part can't be activated until it is added
  // Assert.isTrue(ref == null || parts.contains(ref));

  activePartReference = ref;

  fireActivePartChanged(oldPart, ref);
}

void WorkbenchPagePartList::SetActiveEditor(IEditorReference::Pointer ref)
{
  if (ref == activeEditorReference)
  {
    return;
  }

  // A part can't be activated until it is added
  // Assert.isTrue(ref == null || parts.contains(ref));

  activeEditorReference = ref;

  fireActiveEditorChanged(ref);
}

/**
 * In order to remove a part, it must first be deactivated.
 */
void WorkbenchPagePartList::RemovePart(WorkbenchPartReference::Pointer ref)
{
  Assert.isNotNull(ref);
  // It is an error to remove a part that isn't in the list
  // Assert.isTrue(parts.contains(ref));
  // We're not allowed to remove the active part. We must deactivate it
  // first.
  Assert.isTrue(ref != activePartReference);
  // We're not allowed to remove the active editor. We must deactivate it
  // first.
  Assert.isTrue(ref != activeEditorReference);

  if (ref.getVisible())
  {
    ref.setVisible(false);
  }

  // If this part is currently open, fire the "part closed" event before
  // removal
  if (ref.getPart(false) != null)
  {
    partClosed(ref);
  }

  ref.removeInternalPropertyListener(listener);

  firePartRemoved(ref);
}

void WorkbenchPagePartList::PartInputChanged(WorkbenchPartReference::Pointer ref)
{
  firePartInputChanged(ref);
}

void WorkbenchPagePartList::PartHidden(WorkbenchPartReference::Pointer ref)
{
  // Part should not be null
  Assert.isNotNull(ref);
  // This event should only be fired if the part is actually visible
  Assert.isTrue(!ref.getVisible());
  // We shouldn't be receiving events from parts until they are in the
  // list
  // Assert.isTrue(parts.contains(ref));

  firePartHidden(ref);
}

void WorkbenchPagePartList::PartOpened(WorkbenchPartReference::Pointer ref)
{
  Assert.isNotNull(ref);

  IWorkbenchPart actualPart = ref.getPart(false);
  // We're firing the event that says "the part was just created"... so
  // there better be a part there.
  Assert.isNotNull(actualPart);
  // Must be called after the part is inserted into the part list
  // Assert.isTrue(parts.contains(ref));
  // The active part must be opened before it is activated, so we should
  // never get an
  // open event for a part that is already active. (This either indicates
  // that a redundant
  // open event was fired or that a closed part was somehow activated)
  Assert.isTrue(activePartReference != ref);
  // The active editor must be opened before it is activated, so we should
  // never get an
  // open event for an editor that is already active. (This either
  // indicates that a redundant
  // open event was fired or that a closed editor was somehow activated)
  Assert.isTrue(activeEditorReference != ref);

//  SaveablesList modelManager = (SaveablesList) actualPart
//  .getSite().getService(ISaveablesLifecycleListener.class);
//  modelManager.postOpen(actualPart);

  // Fire the "part opened" event
  firePartOpened(ref);
}

/**
 * Called when a concrete part is about to be destroyed. This is called
 * BEFORE disposal happens, so the part should still be accessable from the
 * part reference.
 * 
 * @param ref
 */
void WorkbenchPagePartList::PartClosed(WorkbenchPartReference::Pointer ref)
{
  Assert.isNotNull(ref);

  IWorkbenchPart actualPart = ref.getPart(false);
  // Called before the part is disposed, so the part should still be
  // there.
  Assert.isNotNull(actualPart);
  // Must be called before the part is actually removed from the part list
  // Assert.isTrue(parts.contains(ref));
  // Not allowed to close the active part. The part must be deactivated
  // before it may
  // be closed.
  Assert.isTrue(activePartReference != ref);
  // Not allowed to close the active editor. The editor must be
  // deactivated before it may
  // be closed.
  Assert.isTrue(activeEditorReference != ref);

  firePartClosed(ref);
}

void WorkbenchPagePartList::PartVisible(WorkbenchPartReference::Pointer ref)
{
  // Part should not be null
  Assert.isNotNull(ref);
  // This event should only be fired if the part is actually visible
  Assert.isTrue(ref.getVisible());
  // We shouldn't be receiving events from parts until they are in the
  // list
  // Assert.isTrue(parts.contains(ref));
  // Part must be open before it can be made visible
  Assert.isNotNull(ref.getPart(false));

  firePartVisible(ref);
}

WorkbenchPagePartList::WorkbenchPagePartList(
    PageSelectionService* selectionService)
{
  this.selectionService = selectionService;
}

IPartService* WorkbenchPagePartList::GetPartService()
{
  return partService;
}

void WorkbenchPagePartList::FirePartOpened(IWorkbenchPartReference::Pointer part)
{
  partService.firePartOpened(part);
}

void WorkbenchPagePartList::FirePartClosed(IWorkbenchPartReference::Pointer part)
{
  partService.firePartClosed(part);
}

void WorkbenchPagePartList::FirePartAdded(IWorkbenchPartReference::Pointer part)
{
  // TODO: There is no listener for workbench page additions yet 
}

void WorkbenchPagePartList::FirePartRemoved(
    IWorkbenchPartReference::Pointer part)
{
  // TODO: There is no listener for workbench page removals yet
}

void WorkbenchPagePartList::FireActiveEditorChanged(
    IWorkbenchPartReference::Pointer ref)
{
  if (ref != null)
  {
    firePartBroughtToTop(ref);
  }
}

void WorkbenchPagePartList::FireActivePartChanged(
    IWorkbenchPartReference::Pointer oldRef,
    IWorkbenchPartReference::Pointer newRef)
{
  partService.setActivePart(newRef);

  IWorkbenchPart realPart = newRef == null ? null : newRef.getPart(false);
  selectionService.setActivePart(realPart);
}

void WorkbenchPagePartList::FirePartHidden(IWorkbenchPartReference::Pointer ref)
{
  partService.firePartHidden(ref);
}

void WorkbenchPagePartList::FirePartVisible(IWorkbenchPartReference::Pointer ref)
{
  partService.firePartVisible(ref);
}

void WorkbenchPagePartList::FirePartInputChanged(
    IWorkbenchPartReference::Pointer ref)
{
  partService.firePartInputChanged(ref);
}

void WorkbenchPagePartList::FirePartBroughtToTop(
    IWorkbenchPartReference::Pointer ref)
{
  partService.firePartBroughtToTop(ref);
}

}
