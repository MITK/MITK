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

#include "berryRadioState.h"

#include "berryObjectString.h"
#include "berryObjectStringMap.h"

#include "berryIPreferences.h"

namespace berry {

const QString RadioState::STATE_ID = "org.blueberry.ui.commands.radioState";
const QString RadioState::PARAMETER_ID = "org.blueberry.ui.commands.radioStateParameter";

RadioState::RadioState()
{
  SetShouldPersist(true);
}

void RadioState::SetInitializationData(const SmartPointer<IConfigurationElement>& /*config*/,
                                       const QString& /*propertyName*/, const Object::Pointer& data)
{
  bool shouldPersist = true; // persist by default
  if (data.Cast<ObjectString>())
  {
    SetValue(data);
  }
  else if (ObjectStringMap::Pointer parameters = data.Cast<ObjectStringMap>())
  {
    const QString defaultString = parameters->value("default");
    if (!defaultString.isNull())
    {
      Object::Pointer value(new ObjectString(defaultString));
      SetValue(value);
    }

    const QString persistedString = parameters->value("persisted");
    if (!persistedString.isNull()
        && persistedString.compare("false", Qt::CaseInsensitive) == 0)
      shouldPersist = false;
  }
  SetShouldPersist(shouldPersist);
}

void RadioState::Load(const SmartPointer<IPreferences>& store, const QString& preferenceKey)
{
  if (!ShouldPersist())
    return;

  const QString prefValue = store->Get(preferenceKey, QString());
  if (!prefValue.isEmpty())
  {
    Object::Pointer value(new ObjectString(prefValue));
    SetValue(value);
  }
}

void RadioState::Save(const SmartPointer<IPreferences>& store, const QString& preferenceKey)
{
  if (!ShouldPersist())
    return;

  const Object::Pointer value = GetValue();
  if (ObjectString::Pointer objStr = value.Cast<ObjectString>())
  {
    store->Put(preferenceKey, *objStr);
  }
}

void RadioState::SetValue(const Object::Pointer& value)
{
  if (!(value.Cast<ObjectString>()))
    return; // we set only String values
  PersistentState::SetValue(value);
}

}
