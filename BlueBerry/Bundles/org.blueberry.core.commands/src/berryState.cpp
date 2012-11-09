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

void State::AddListener(IStateListener::Pointer listener)
{
  stateEvents.AddListener(listener);
}

void State::RemoveListener(IStateListener::Pointer listener)
{
  stateEvents.RemoveListener(listener);
}

void State::FireStateChanged(Object::Pointer oldValue)
{
  stateEvents.stateChanged(State::Pointer(this), oldValue);
}

std::string State::GetId() const
{
  return id;
}

Object::Pointer State::GetValue() const
{
  return value;
}

void State::SetId(const std::string& id)
{
  this->id = id;
}

void State::SetValue(const Object::Pointer value)
{
  if (this->value != value)
  {
    const Object::Pointer oldValue(this->value);
    this->value = value;
    //fireStateChanged(oldValue);
  }
}

}
