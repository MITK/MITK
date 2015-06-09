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

#include "berryState.h"

namespace berry
{

void State::AddListener(IStateListener* listener)
{
  stateEvents.AddListener(listener);
}

void State::AddListener(const IStateListener::Events::StateEvent::AbstractDelegate& delegate)
{
  stateEvents.stateChanged.AddListener(delegate);
}

void State::RemoveListener(IStateListener* listener)
{
  stateEvents.RemoveListener(listener);
}

void State::RemoveListener(const IStateListener::Events::StateEvent::AbstractDelegate& delegate)
{
  stateEvents.stateChanged.RemoveListener(delegate);
}

void State::FireStateChanged(const Object::Pointer& oldValue)
{
  stateEvents.stateChanged(State::Pointer(this), oldValue);
}

QString State::GetId() const
{
  return id;
}

Object::Pointer State::GetValue() const
{
  return value;
}

void State::SetId(const QString& id)
{
  this->id = id;
}

void State::SetValue(const Object::Pointer& value)
{
  if (this->value != value)
  {
    const Object::Pointer oldValue(this->value);
    this->value = value;
    this->FireStateChanged(oldValue);
  }
}

}
