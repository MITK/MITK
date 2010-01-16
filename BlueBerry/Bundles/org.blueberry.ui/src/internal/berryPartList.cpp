#include "berryPartList.h"

#include "berrySaveablesList.h"

#include "../berryPartPane.h"
#include "../berryIWorkbenchPartConstants.h"
#include "../berryIWorkbenchPartSite.h"

#include <berryObjects.h>

namespace berry
{

void PartList::PropertyChange(Object::Pointer source, int propId)
{
  WorkbenchPartReference::Pointer ref = source.Cast<
      WorkbenchPartReference> ();

  if (propId == IWorkbenchPartConstants::PROP_OPENED)
  {
    this->PartOpened(ref);
  }
  else if (propId == IWorkbenchPartConstants::PROP_CLOSED)
  {
    this->PartClosed(ref);
  }
  else if (propId == IWorkbenchPartConstants::PROP_VISIBLE)
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
  return activeEditorReference.IsNull() ? IEditorPart::Pointer(0) : activeEditorReference->GetEditor(
      false);
}

IWorkbenchPart::Pointer PartList::GetActivePart()
{
  return activePartReference == 0 ? IWorkbenchPart::Pointer(0) : activePartReference->GetPart(false);
}

//std::vector<IEditorReference::Pointer> PartList::GetEditors()
//{
//  std::vector<IEditorReference::Pointer> result;
//  for (std::deque<IWorkbenchPartReference::Pointer>::iterator iter =
//      parts.begin(); iter != parts.end(); ++iter)
//  {
//    if (iter->Cast<IEditorReference> () != 0)
//      result.push_back(iter->Cast<IEditorReference> ());
//  }
//
//  return result;
//}

void PartList::AddPart(WorkbenchPartReference::Pointer ref)
{
  poco_assert(ref.IsNotNull());

  ref->AddPropertyListener(IPropertyChangeListener::Pointer(this));

//  if (!this->Contains(ref))
//  {
//    parts.push_back(ref);
//  }

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
  //poco_assert(ref == 0 || this->Contains(ref));

  //std::remove(parts.begin(), parts.end(), ref);
  //parts.push_front(ref);

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
  //poco_assert(ref == 0 || this->Contains(ref));

  activeEditorReference = ref;
  //std::remove(parts.begin(), parts.end(), ref);
  //parts.push_front(ref);

  this->FireActiveEditorChanged(ref);
}

void PartList::RemovePart(WorkbenchPartReference::Pointer ref)
{
  poco_assert(ref.IsNotNull());
  // It is an error to remove a part that isn't in the list
  //poco_assert(this->Contains(ref));
  // We're not allowed to remove the active part. We must deactivate it
  // first.
  poco_assert(activePartReference != ref);
  // We're not allowed to remove the active editor. We must deactivate it
  // first.
  if (ref.Cast<IEditorReference>())
  {
    poco_assert(activeEditorReference != ref.Cast<IEditorReference>());
  }

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

  //std::remove(parts.begin(), parts.end(), ref);
  ref->RemovePropertyListener(IPropertyChangeListener::Pointer(this));

  this->FirePartRemoved(ref);
}

//int PartList::IndexOf(const IWorkbenchPartReference::Pointer ref) const
//{
//  std::deque<IWorkbenchPartReference::Pointer>::const_iterator result = std::find(parts.begin(), parts.end(), ref);
//  if (result == parts.end()) return -1;
//  else return result - parts.begin();
//}

//void PartList::BringToTop(IWorkbenchPartReference::Pointer ref)
//{
//  IStackableContainer::Pointer targetContainer;
//  if (ref != 0)
//  {
//    PartPane::Pointer pane = ref.Cast<WorkbenchPartReference>()->GetPane();
//    if (pane != 0)
//    {
//      targetContainer = pane->GetContainer();
//    }
//  }
//
//  std::deque<IWorkbenchPartReference::Pointer>::iterator newIndex = this->LastIndexOfContainer(targetContainer);
//
//  // //New index can be -1 if there is no last index
//  // if (newIndex >= 0 && ref == parts.get(newIndex))
//  //  return;
//
//  std::remove(parts.begin(), parts.end(), ref);
//  if(newIndex != parts.end())
//  {
//    parts.insert(newIndex, ref);
//  }
//  else
//  parts.push_front(ref);
//}

//std::vector<IWorkbenchPartReference::Pointer>
//PartList::GetParts(const std::vector<IViewReference::Pointer>& views)
//{
//  std::vector<IWorkbenchPartReference::Pointer> resultList;
//  for (std::deque<IWorkbenchPartReference::Pointer>::iterator partIter = parts.begin();
//      partIter != parts.end(); ++partIter)
//  {
//    IWorkbenchPartReference::Pointer ref = *partIter;
//    if (ref.Cast<IViewReference>() != 0)
//    {
//      //Filter views from other perspectives
//      for (unsigned int i = 0; i < views.size(); i++)
//      {
//        if (ref == views[i])
//        {
//          resultList.push_back(ref);
//          break;
//        }
//      }
//    }
//    else
//    {
//      resultList.push_back(ref);
//    }
//  }
//  return resultList;
//}

//std::deque<IWorkbenchPartReference::Pointer>::iterator
//PartList::LastIndexOfContainer(IStackableContainer::Pointer container)
//{
//  for (std::deque<IWorkbenchPartReference::Pointer>::iterator iter = parts.begin();
//      iter != parts.end(); ++iter)
//  {
//    IWorkbenchPartReference::Pointer ref = *iter;
//
//    IStackableContainer::Pointer cnt;
//    PartPane::Pointer pane = ref.Cast<WorkbenchPartReference>()->GetPane();
//    if (pane != 0)
//    {
//      cnt = pane->GetContainer();
//    }
//    if (cnt == container)
//    {
//      return iter;
//    }
//  }
//
//  return parts.end();
//}

//bool PartList::Contains(IWorkbenchPartReference::Pointer ref)
//{
//  return std::find(parts.begin(), parts.end(), ref) != parts.end();
//}

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
  //poco_assert(this->Contains(ref));

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
  //poco_assert(this->Contains(ref));
  // The active part must be opened before it is activated, so we should
  // never get an open event for a part that is already active.
  // (This either indicates that a redundant
  // open event was fired or that a closed part was somehow activated)
  poco_assert(activePartReference != ref);
  // The active editor must be opened before it is activated, so we should
  // never get an open event for an editor that is already active.
  // (This either indicates that a redundant
  // open event was fired or that a closed editor was somehow activated)
  poco_assert((void*)activeEditorReference.GetPointer() != (void*)ref.GetPointer());

  SaveablesList::Pointer modelManager = actualPart
  ->GetSite()->GetService(ISaveablesLifecycleListener::GetManifestName()).Cast<SaveablesList>();
  modelManager->PostOpen(actualPart);

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
  // poco_assert(this->Contains(ref));
  // Not allowed to close the active part. The part must be deactivated
  // before it may be closed.
  poco_assert(activePartReference != ref);
  // Not allowed to close the active editor. The editor must be
  // deactivated before it may be closed.
  if (ref.Cast<IEditorReference>())
  {
    poco_assert(activeEditorReference != ref.Cast<IEditorReference>());
  }

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
  //poco_assert(this->Contains(ref));
  // Part must be open before it can be made visible
  poco_assert(ref->GetPart(false).IsNotNull());

  this->FirePartVisible(ref);
}

}
