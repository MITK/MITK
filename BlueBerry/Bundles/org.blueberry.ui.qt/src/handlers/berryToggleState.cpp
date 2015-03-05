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

#include "berryToggleState.h"

#include "berryObjects.h"

#include "berryIPreferences.h"

#include <QStringList>

namespace berry {

ToggleState::ToggleState()
{
  Object::Pointer value(new ObjectBool(false));
  SetValue(value);
}

void ToggleState::Load(const SmartPointer<IPreferences>& store,
                       const QString& preferenceKey)
{
  bool currentValue = GetValue().Cast<ObjectBool>()->GetValue();
  //store.setDefault(preferenceKey, currentValue);
  if (ShouldPersist() && (store->Keys().contains(preferenceKey)))
  {
    const bool value = store->GetBool(preferenceKey, currentValue);
    if (value != currentValue)
    {
      Object::Pointer newValue(new ObjectBool(value));
      SetValue(newValue);
    }
  }
}

void ToggleState::Save(const SmartPointer<IPreferences>& store,
                       const QString& preferenceKey)
{
  if (ShouldPersist())
  {
    const Object::Pointer value = GetValue();
    if (ObjectBool::Pointer boolValue = value.Cast<ObjectBool>())
    {
      store->PutBool(preferenceKey, boolValue->GetValue());
    }
  }
}

void ToggleState::SetValue(const Object::Pointer& value)
{
  if (!(value.Cast<ObjectBool>()))
  {
    throw ctkInvalidArgumentException("ToggleState takes a Boolean as a value");
  }

  PersistentState::SetValue(value);
}

}
