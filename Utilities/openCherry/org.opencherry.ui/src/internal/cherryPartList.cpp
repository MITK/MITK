#include "cherryPartList.h"

#include "../cherryIWorkbenchPartConstants.h"

namespace cherry
{

void PartList::PropertyChanged(WorkbenchPartReference::Pointer ref, int propId)
{
  if (propId == WorkbenchPartReference::INTERNAL_PROPERTY_OPENED)
  {
    this->PartOpened(ref);
  }
  else if (propId == WorkbenchPartReference::INTERNAL_PROPERTY_CLOSED)
  {
    this->PartClosed(ref);
  }
  else if (propId == WorkbenchPartReference::INTERNAL_PROPERTY_VISIBLE)
  {
    if (ref->GetVisible())
    {
      this->PartVisible(ref);
    }
    else
    {
      this->PartHidden(ref);
    }
  }
  else if (propId == IWorkbenchPartConstants::PROP_INPUT)
  {
    this->PartInputChanged(ref);
  } 
}

IWorkbenchPartReference::Pointer PartList::GetActivePartReference()
{
  return activePartReference;
}

IEditorReference::Pointer PartList::GetActiveEditorReference()
{
  return activeEditorReference;
}

IEditorPart::Pointer PartList::GetActiveEditor()
{
  return activeEditorReference.IsNull() ? 0
      : activeEditorReference->GetEditor(false);
}

IWorkbenchPart::Pointer PartList::GetActivePart()
{
  return activePartReference.IsNull() ? 0 : activePartReference->GetPart(false);
}

void PartList::AddPart(WorkbenchPartReference::Pointer ref)
{
  poco_assert(ref.IsNotNull());

  ref->GetInternalPropertyEvent().AddListener(this, &PartList::PropertyChanged);

  // parts.add(ref);
  this->FirePartAdded(ref);

  // If this part is already open, fire the "part opened" event
  // immediately
  if (ref->GetPart(false).IsNotNull())
  {
    this->PartOpened(ref);
  }

  // If this part is already visible, fire the visibility event
  // immediately
  if (ref->GetVisible())
  {
    this->PartVisible(ref);
  }
}

void PartList::SetActivePart(IWorkbenchPartReference::Pointer ref)
{
  if (ref == activePartReference)
  {
    return;
  }

  IWorkbenchPartReference::Pointer oldPart = activePartReference;

  // A part can't be activated until it is added
  // Assert.isTrue(ref == null || parts.contains(ref));

  if (ref.IsNotNull())
  {
    IWorkbenchPart::Pointer part = ref->GetPart(true);
    poco_assert(part.IsNotNull());
  }

  activePartReference = ref;

  this->FireActivePartChanged(oldPart, ref);
}

void PartList::SetActiveEditor(IEditorReference::Pointer ref)
{
  if (ref == activeEditorReference)
  {
    return;
  }

  // A part can't be activated until it is added
  // Assert.isTrue(ref == null || parts.contains(ref));

  if (ref.IsNotNull())
  {
    IWorkbenchPart::Pointer part = ref->GetPart(true);
    poco_assert(part.IsNotNull());
  }

  activeEditorReference = ref;

  this->FireActiveEditorChanged(ref);
}

void PartList::RemovePart(WorkbenchPartReference::Pointer ref)
{
  poco_assert(ref.IsNotNull());
  // It is an error to remove a part that isn't in the list
  // Assert.isTrue(parts.contains(ref));
  // We're not allowed to remove the active part. We must deactivate it
  // first.
  poco_assert(activePartReference != ref);
  // We're not allowed to remove the active editor. We must deactivate it
  // first.
  poco_assert(activeEditorReference != ref.Cast<IEditorReference>());

  if (ref->GetVisible())
  {
    ref->SetVisible(false);
  }

  // If this part is currently open, fire the "part closed" event before
  // removal
  if (ref->GetPart(false).IsNotNull())
  {
    this->PartClosed(ref);
  }

  ref->GetInternalPropertyEvent().RemoveListener(this, &PartList::PropertyChanged);

  this->FirePartRemoved(ref);
}

void PartList::PartInputChanged(WorkbenchPartReference::Pointer ref)
{
  this->FirePartInputChanged(ref);
}

void PartList::PartHidden(WorkbenchPartReference::Pointer ref)
{
  // Part should not be null
  poco_assert(ref.IsNotNull());
  // This event should only be fired if the part is actually visible
  poco_assert(!ref->GetVisible());
  // We shouldn't be receiving events from parts until they are in the
  // list
  // Assert.isTrue(parts.contains(ref));

  this->FirePartHidden(ref);
}

void PartList::PartOpened(WorkbenchPartReference::Pointer ref)
{
  poco_assert(ref.IsNotNull());

  IWorkbenchPart::Pointer actualPart = ref->GetPart(false);
  // We're firing the event that says "the part was just created"... so
  // there better be a part there.
  poco_assert(actualPart.IsNotNull());
  // Must be called after the part is inserted into the part list
  // Assert.isTrue(parts.contains(ref));
  // The active part must be opened before it is activated, so we should
  // never get an
  // open event for a part that is already active. (This either indicates
  // that a redundant
  // open event was fired or that a closed part was somehow activated)
  poco_assert(activePartReference != ref);
  // The active editor must be opened before it is activated, so we should
  // never get an
  // open event for an editor that is already active. (This either
  // indicates that a redundant
  // open event was fired or that a closed editor was somehow activated)
  poco_assert(activeEditorReference != ref.Cast<IEditorReference>());

  //SaveablesList modelManager = (SaveablesList) actualPart
  //    .getSite().getService(ISaveablesLifecycleListener.class);
  //modelManager.postOpen(actualPart);

  // Fire the "part opened" event
  this->FirePartOpened(ref);
}

void PartList::PartClosed(WorkbenchPartReference::Pointer ref)
{
  poco_assert(ref.IsNotNull());

  IWorkbenchPart::Pointer actualPart = ref->GetPart(false);
  // Called before the part is disposed, so the part should still be
  // there.
  poco_assert(actualPart.IsNotNull());
  // Must be called before the part is actually removed from the part list
  // Assert.isTrue(parts.contains(ref));
  // Not allowed to close the active part. The part must be deactivated
  // before it may
  // be closed.
  poco_assert(activePartReference != ref);
  // Not allowed to close the active editor. The editor must be
  // deactivated before it may
  // be closed.
  poco_assert(activeEditorReference != ref.Cast<IEditorReference>());

  this->FirePartClosed(ref);
}

void PartList::PartVisible(WorkbenchPartReference::Pointer ref)
{
  // Part should not be null
  poco_assert(ref.IsNotNull());
  // This event should only be fired if the part is actually visible
  poco_assert(ref->GetVisible());
  // We shouldn't be receiving events from parts until they are in the
  // list
  // Assert.isTrue(parts.contains(ref));
  // Part must be open before it can be made visible
  poco_assert(ref->GetPart(false).IsNotNull());

  this->FirePartVisible(ref);
}

}
