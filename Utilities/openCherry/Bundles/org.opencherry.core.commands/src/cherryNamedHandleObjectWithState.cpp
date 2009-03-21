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
#include "cherryINamedHandleStateIds.h"

#include <Poco/Exception.h>

namespace cherry
{

void NamedHandleObjectWithState::AddState(const std::string& stateId,
    const State::Pointer state)
{
  if (!state)
  {
    throw Poco::NullPointerException("Cannot add a null state"); //$NON-NLS-1$
  }

  states[stateId] = state;
}

std::string NamedHandleObjectWithState::GetDescription() const
{
  const std::string description(NamedHandleObject::GetDescription()); // Trigger a NDE.

  const State::ConstPointer
      descriptionState(this->GetState(INamedHandleStateIds::DESCRIPTION));
  if (descriptionState.IsNotNull())
  {
    const Object::ConstPointer value(descriptionState->GetValue());
    if (value.IsNotNull())
    {
      return value->ToString();
    }
  }

  return description;
}

std::string NamedHandleObjectWithState::GetName() const
{
  const std::string name(NamedHandleObject::GetName()); // Trigger a NDE, if necessary.

  const State::ConstPointer
      nameState(this->GetState(INamedHandleStateIds::NAME));
  if (nameState.IsNotNull())
  {
    const Object::ConstPointer value(nameState->GetValue());
    if (value.IsNotNull())
    {
      return value->ToString();
    }
  }

  return name;
}

State::Pointer NamedHandleObjectWithState::GetState(
    const std::string& stateId) const
{
  if (states.empty())
  {
    return State::Pointer(0);
  }

  std::map<std::string, State::Pointer>::const_iterator iter = states.find(stateId);
  return iter->second;
}

std::vector<std::string> NamedHandleObjectWithState::GetStateIds() const
{
  if (states.empty())
  {
    return std::vector<std::string>();
  }

  std::vector<std::string> stateIds;
  for (std::map<std::string, State::Pointer>::const_iterator iter = states.begin();
      iter != states.end(); ++iter)
  {
    stateIds.push_back(iter->first);
  }
  return stateIds;
}

void NamedHandleObjectWithState::RemoveState(const std::string& id)
{
  if (id.empty())
  {
    throw Poco::InvalidArgumentException("Cannot remove an empty id"); //$NON-NLS-1$
  }

  states.erase(id);
}

NamedHandleObjectWithState::NamedHandleObjectWithState(const std::string& id)
: NamedHandleObject(id)
{

}

}
