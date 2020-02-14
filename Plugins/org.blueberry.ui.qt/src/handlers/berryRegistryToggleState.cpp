/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryRegistryToggleState.h"

#include "berryObjects.h"
#include "berryObjectString.h"
#include "berryObjectStringMap.h"

namespace berry {

const QString RegistryToggleState::STATE_ID = "org.blueberry.ui.commands.toggleState";

void RegistryToggleState::SetInitializationData(
    const SmartPointer<IConfigurationElement>& /*configurationElement*/,
    const QString& /*propertyName*/, const Object::Pointer& data)
{
  if (ObjectString::Pointer objStr = data.Cast<ObjectString>())
  {
    // This is the default value.
    ReadDefault(*objStr);
    SetShouldPersist(true);
  }
  else if (ObjectStringMap::Pointer parameters = data.Cast<ObjectStringMap>())
  {
    QString defaultObject = parameters->value("default");
    if (!defaultObject.isNull())
    {
      ReadDefault(defaultObject);
    }

    QString persistedObject = parameters->value("persisted");
    if (!persistedObject.isNull())
    {
      ReadPersisted(persistedObject);
    }
  }
  else
  {
    SetShouldPersist(true);
  }
}

void RegistryToggleState::ReadDefault(const QString& defaultString)
{
  if (defaultString.compare("true", Qt::CaseInsensitive) == 0)
  {
    Object::Pointer newValue(new ObjectBool(true));
    SetValue(newValue);
  }
}

void RegistryToggleState::ReadPersisted(const QString& persistedString)
{
  if (persistedString.compare("false", Qt::CaseInsensitive) == 0)
  {
    SetShouldPersist(false);
  }
  else
  {
    SetShouldPersist(true);
  }
}

}
