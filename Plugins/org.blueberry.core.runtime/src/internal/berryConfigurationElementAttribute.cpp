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

#include "berryConfigurationElementAttribute.h"

namespace berry {

ConfigurationElementAttribute::ConfigurationElementAttribute(const QString& name, const QString& value)
  : name(name), value(value)
{
}

QString ConfigurationElementAttribute::GetName() const
{
  return name;
}

QString ConfigurationElementAttribute::GetValue() const
{
  return value;
}

}
