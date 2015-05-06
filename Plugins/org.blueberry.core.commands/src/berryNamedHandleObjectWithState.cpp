/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "berryNamedHandleObjectWithState.h"
#include "berryINamedHandleStateIds.h"

#include "berryState.h"

namespace berry
{

void NamedHandleObjectWithState::AddState(const QString& stateId,
                                          const State::Pointer& state)
{
  if (!state)
  {
    throw Poco::NullPointerException("Cannot add a null state");
  }

  states[stateId] = state;
}

QString NamedHandleObjectWithState::GetDescription() const
{
  const QString description(NamedHandleObject::GetDescription()); // Trigger a NDE.

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

QString NamedHandleObjectWithState::GetName() const
{
  const QString name(NamedHandleObject::GetName()); // Trigger a NDE, if necessary.

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
    const QString& stateId) const
{
  if (states.empty())
  {
    return State::Pointer(0);
  }

  QHash<QString, State::Pointer>::const_iterator iter = states.find(stateId);
  return iter.value();
}

QList<QString> NamedHandleObjectWithState::GetStateIds() const
{
  if (states.empty())
  {
    return QList<QString>();
  }

  QList<QString> stateIds;
  for (QHash<QString, State::Pointer>::const_iterator iter = states.begin();
      iter != states.end(); ++iter)
  {
    stateIds.push_back(iter.key());
  }
  return stateIds;
}

void NamedHandleObjectWithState::RemoveState(const QString& id)
{
  if (id.isEmpty())
  {
    throw ctkInvalidArgumentException("Cannot remove an empty id");
  }

  states.remove(id);
}

NamedHandleObjectWithState::NamedHandleObjectWithState(const QString& id)
  : NamedHandleObject(id)
{
}

}
