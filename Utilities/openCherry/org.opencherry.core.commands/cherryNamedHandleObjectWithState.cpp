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

#include "cherryNamedHandleObjectWithState.h"

namespace cherry
{

void NamedHandleObjectWithState::AddState(const std::string& stateId,
    const State::ConstPointer state)
{
  if (state.IsNull())
  {
    throw NullPointerException("Cannot add a null state"); //$NON-NLS-1$
  }

  states[stateId] = state;
}

std::string NamedHandleObjectWithState::GetDescription()
{
  const std::string description(NamedHandleObject::GetDescription()); // Trigger a NDE.

  const State::ConstPointer
      descriptionState(this->GetState(INamedHandleStateIds::DESCRIPTION));
  if (descriptionState.IsNotNull())
  {
    const Object::ConstPointer value(descriptionState->GetValue());
    if (value.IsNotNull())
    {
      return value.toString();
    }
  }

  return description;
}

std::string NamedHandleObjectWithState::GetName()
{
  const std::string name(NamedHandleObject::GetName()); // Trigger a NDE, if necessary.

  const State::ConstPointer
      nameState(this->GetState(INamedHandleStateIds::NAME));
  if (nameState.IsNotNull())
  {
    const Object::ConstPointer value(nameState->GetValue());
    if (value.IsNotNull())
    {
      return value.toString();
    }
  }

  return name;
}

const State::Pointer NamedHandleObjectWithState::GetState(
    const std::string& stateId) const
{
  if (states.isEmpty())
  {
    return NULL_STATE;
  }

  return states.find(stateId);
}

std::vector<std::string> NamedHandleObjectWithState::GetStateIds() const
{
  if (states.isEmpty())
  {
    return std::vector<std::string>();
  }

  std::vector<std::string> stateIds;
  for (std::map<std::string, State>::const_iterator iter = states.begin();
      iter != states.end(); ++iter)
  {
    stateIds.push_back(iter->first);
  }
  return stateIds;
}

void NamedHandleObjectWithState::RemoveState(const std::string& id)
{
  if (id == null)
  {
    throw new NullPointerException("Cannot remove a null id"); //$NON-NLS-1$
  }

  if (states != null)
  {
    states.remove(id);
    if (states.isEmpty())
    {
      states = null;
    }
  }
}

NamedHandleObjectWithState::NamedHandleObjectWithState(const std::string& id)
: NamedHandleObject(id)
{

}

}
