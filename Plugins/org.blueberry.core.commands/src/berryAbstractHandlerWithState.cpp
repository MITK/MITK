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
  HandleStateChange(state, Object::Pointer(0));
}

SmartPointer<State> AbstractHandlerWithState::GetState(const QString& stateId) const
{
  QHash<QString,State::Pointer>::const_iterator i = states.find(stateId);
  if (i != states.end()) return *i;
  return State::Pointer(0);
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
