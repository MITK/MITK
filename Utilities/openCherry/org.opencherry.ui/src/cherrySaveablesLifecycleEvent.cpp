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

#include "cherrySaveablesLifecycleEvent.h"

namespace cherry
{

const int SaveablesLifecycleEvent::POST_OPEN = 1;
const int SaveablesLifecycleEvent::PRE_CLOSE = 2;
const int SaveablesLifecycleEvent::POST_CLOSE = 3;
const int SaveablesLifecycleEvent::DIRTY_CHANGED = 4;

SaveablesLifecycleEvent::SaveablesLifecycleEvent(Object::Pointer source_,
    int eventType_, const std::vector<Saveable::Pointer>& saveables_,
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

std::vector<Saveable::Pointer> SaveablesLifecycleEvent::GetSaveables()
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
