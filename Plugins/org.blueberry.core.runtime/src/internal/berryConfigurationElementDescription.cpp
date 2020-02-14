/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryConfigurationElementDescription.h"

#include "berryConfigurationElementAttribute.h"

namespace berry {

ConfigurationElementDescription::ConfigurationElementDescription(const QString& name, const QList<ConfigurationElementAttribute>& attributes,
                                const QString& value, const QList<ConfigurationElementDescription>& children)
  : name(name), attributes(attributes), value(value), children(children)
{
}

ConfigurationElementDescription::ConfigurationElementDescription(const QString& name, const ConfigurationElementAttribute& attribute,
                                const QString& value, const QList<ConfigurationElementDescription>& children)
  : name(name), value(value), children(children)
{
  attributes.push_back(attribute);
}

QList<ConfigurationElementDescription> ConfigurationElementDescription::GetChildren() const
{
  return children;
}

QString ConfigurationElementDescription::GetName() const
{
  return name;
}

QList<ConfigurationElementAttribute> ConfigurationElementDescription::GetAttributes() const
{
  return attributes;
}

QString ConfigurationElementDescription::GetValue() const
{
  return value;
}

}
