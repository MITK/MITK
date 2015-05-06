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

#include "berryRegistryProperties.h"

#include <berryPlatform.h>

#include <ctkPluginContext.h>

namespace berry {

QHash<QString,QString> RegistryProperties::registryProperties;
ctkPluginContext* RegistryProperties::context = NULL;

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
