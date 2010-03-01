#include "berryPartService.h"

namespace berry
{

void PartService::FirePartActivated(IWorkbenchPartReference::Pointer ref)
{
//  UIListenerLogging.logPartListener2Event(debugListeners2Key, this, ref,
//      UIListenerLogging.PE2_ACTIVATED);
//  listeners2.firePartActivated(ref);
  partEvents.partActivated(ref);
}

/**
 * @param ref
 */
void PartService::FirePartDeactivated(IWorkbenchPartReference::Pointer ref)
{
//  UIListenerLogging.logPartListener2Event(debugListeners2Key, this, ref,
//      UIListenerLogging.PE2_PART_DEACTIVATED);
//  listeners2.firePartDeactivated(ref);
  partEvents.partDeactivated(ref);
}

PartService::PartService(const std::string& debugListenersKey,
    const std::string& debugListeners2Key)
{
  this->debugListeners2Key = debugListeners2Key;
  this->debugListenersKey = debugListenersKey;
}

void PartService::AddPartListener(IPartListener::Pointer l)
{
  partEvents.AddListener(l);
}

void PartService::RemovePartListener(IPartListener::Pointer l)
{
  partEvents.RemoveListener(l);
}

void PartService::FirePartBroughtToTop(IWorkbenchPartReference::Pointer ref)
{
//  UIListenerLogging.logPartListener2Event(debugListeners2Key, this, ref,
//      UIListenerLogging.PE2_PART_BROUGHT_TO_TOP);
//  listeners2.firePartBroughtToTop(ref);
  partEvents.partBroughtToTop(ref);
}

void PartService::FirePartClosed(IWorkbenchPartReference::Pointer ref)
{
//  UIListenerLogging.logPartListener2Event(debugListeners2Key, this, ref,
//      UIListenerLogging.PE2_PART_CLOSED);
//  listeners2.firePartClosed(ref);
  partEvents.partClosed(ref);
}

void PartService::FirePartVisible(IWorkbenchPartReference::Pointer ref)
{
//  UIListenerLogging.logPartListener2Event(debugListeners2Key, this, ref,
//      UIListenerLogging.PE2_PART_VISIBLE);
//  listeners2.firePartVisible(ref);
  partEvents.partVisible(ref);
}

void PartService::FirePartHidden(IWorkbenchPartReference::Pointer ref)
{
//  UIListenerLogging.logPartListener2Event(debugListeners2Key, this, ref,
//      UIListenerLogging.PE2_PART_HIDDEN);
//  listeners2.firePartHidden(ref);
  partEvents.partHidden(ref);
}

void PartService::FirePartInputChanged(IWorkbenchPartReference::Pointer ref)
{
//  UIListenerLogging.logPartListener2Event(debugListeners2Key, this, ref,
//      UIListenerLogging.PE2_PART_INPUT_CHANGED);
//  listeners2.firePartInputChanged(ref);
  partEvents.partInputChanged(ref);
}

void PartService::FirePartOpened(IWorkbenchPartReference::Pointer ref)
{
//  UIListenerLogging.logPartListener2Event(debugListeners2Key, this, ref,
//      UIListenerLogging.PE2_PART_OPENED);
//  listeners2.firePartOpened(ref);
  partEvents.partOpened(ref);
}

IWorkbenchPart::Pointer PartService::GetActivePart()
{
  return activePart.Expired() ? IWorkbenchPart::Pointer(0) : activePart.Lock()->GetPart(false);
}

IWorkbenchPartReference::Pointer PartService::GetActivePartReference()
{
  return activePart.Lock();
}

void PartService::SetActivePart(IWorkbenchPartReference::Pointer ref)
{
  IWorkbenchPartReference::Pointer oldRef = activePart.Lock();

  // Filter out redundant activation events
  if (oldRef == ref)
  {
    return;
  }

  if (oldRef.IsNotNull())
  {
    this->FirePartDeactivated(oldRef);
  }

  activePart = ref;

  if (ref.IsNotNull())
  {
    this->FirePartActivated(ref);
  }
}

}
