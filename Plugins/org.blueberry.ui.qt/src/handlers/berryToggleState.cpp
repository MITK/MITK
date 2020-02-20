/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
