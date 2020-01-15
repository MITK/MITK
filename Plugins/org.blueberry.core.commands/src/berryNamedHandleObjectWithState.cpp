/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
    return State::Pointer(nullptr);
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
