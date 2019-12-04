/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
