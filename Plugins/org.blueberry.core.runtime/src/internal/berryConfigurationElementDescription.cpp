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
