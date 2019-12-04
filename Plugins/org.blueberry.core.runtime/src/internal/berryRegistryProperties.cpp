/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryRegistryProperties.h"

#include <berryPlatform.h>

#include <ctkPluginContext.h>

namespace berry {

QHash<QString,QString> RegistryProperties::registryProperties;
ctkPluginContext* RegistryProperties::context = nullptr;

void RegistryProperties::SetContext(ctkPluginContext* context)
{
  RegistryProperties::context = context;
}

QString RegistryProperties::GetProperty(const QString& propertyName)
{
  QHash<QString,QString>::const_iterator i = registryProperties.find(propertyName);
  if (i != registryProperties.end() && !i.value().isNull())
    return i.value();

  return GetContextProperty(propertyName);
}

QString RegistryProperties::GetProperty(const QString& property, const QString& defaultValue)
{
  QString result = RegistryProperties::GetProperty(property);
  return result.isNull() ? defaultValue : result;
}

void RegistryProperties::SetProperty(const QString& propertyName, const QString& propertyValue)
{
  registryProperties.insert(propertyName, propertyValue);
}

QString RegistryProperties::GetContextProperty(const QString& propertyName)
{
  return context->getProperty(propertyName).toString();
}

}
