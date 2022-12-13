/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryToggleState.h"

#include "berryObjects.h"

#include <QStringList>

#include <mitkIPreferences.h>

namespace berry {

ToggleState::ToggleState()
{
  Object::Pointer value(new ObjectBool(false));
  SetValue(value);
}

void ToggleState::Load(const mitk::IPreferences* store,
                       const std::string& preferenceKey)
{
  bool currentValue = GetValue().Cast<ObjectBool>()->GetValue();
  const auto keys = store->Keys();
  if (ShouldPersist() && (std::find(keys.begin(), keys.end(), preferenceKey) != keys.end()))
  {
    const bool value = store->GetBool(preferenceKey, currentValue);
    if (value != currentValue)
    {
      Object::Pointer newValue(new ObjectBool(value));
      SetValue(newValue);
    }
  }
}

void ToggleState::Save(mitk::IPreferences* store,
                       const std::string& preferenceKey)
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
