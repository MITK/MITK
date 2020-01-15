/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryAbstractHandlerWithState.h"

#include "berryState.h"

namespace berry {

void AbstractHandlerWithState::AddState(const QString& stateId, const SmartPointer<State>& state)
{
  if (state.IsNull())
  {
    throw ctkInvalidArgumentException("Cannot add a null state");
  }

  states.insert(stateId, state);
  state->AddListener(this);
  HandleStateChange(state, Object::Pointer(nullptr));
}

SmartPointer<State> AbstractHandlerWithState::GetState(const QString& stateId) const
{
  QHash<QString,State::Pointer>::const_iterator i = states.find(stateId);
  if (i != states.end()) return *i;
  return State::Pointer(nullptr);
}

QList<QString> AbstractHandlerWithState::GetStateIds() const
{
  return states.keys();
}

void AbstractHandlerWithState::RemoveState(const QString& stateId)
{
  if (stateId.isNull())
  {
    throw ctkInvalidArgumentException("Cannot remove a null state");
  }

  QHash<QString,State::Pointer>::iterator i = states.find(stateId);
  if (i != states.end())
  {
    (*i)->RemoveListener(this);
    states.erase(i);
  }
}

}
