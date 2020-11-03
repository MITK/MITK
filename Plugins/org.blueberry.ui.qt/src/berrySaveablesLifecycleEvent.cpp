/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berrySaveablesLifecycleEvent.h"

namespace berry
{

const int SaveablesLifecycleEvent::POST_OPEN = 1;
const int SaveablesLifecycleEvent::PRE_CLOSE = 2;
const int SaveablesLifecycleEvent::POST_CLOSE = 3;
const int SaveablesLifecycleEvent::DIRTY_CHANGED = 4;

SaveablesLifecycleEvent::SaveablesLifecycleEvent(Object::Pointer source_,
    int eventType_, const QList<Saveable::Pointer>& saveables_,
    bool force_) :
  eventType(eventType_), saveables(saveables_), force(force_), veto(false),
      source(source_)
{

}

int SaveablesLifecycleEvent::GetEventType()
{
  return eventType;
}

Object::Pointer SaveablesLifecycleEvent::GetSource()
{
  return source;
}

QList<Saveable::Pointer> SaveablesLifecycleEvent::GetSaveables()
{
  return saveables;
}

bool SaveablesLifecycleEvent::IsVeto()
{
  return veto;
}

void SaveablesLifecycleEvent::SetVeto(bool veto)
{
  this->veto = veto;
}

bool SaveablesLifecycleEvent::IsForce()
{
  return force;
}

}
