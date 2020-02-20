/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryProductExtensionBranding.h"

#include <berryIConfigurationElement.h>
#include <berryIContributor.h>
#include <berryIProduct.h>

#include "berryCTKPluginActivator.h"

namespace berry {

const QString ProductExtensionBranding::ATTR_DESCRIPTION = "description";
const QString ProductExtensionBranding::ATTR_NAME = "name";
const QString ProductExtensionBranding::ATTR_APPLICATION = "application";
const QString ProductExtensionBranding::ATTR_VALUE = "value";


ProductExtensionBranding::ProductExtensionBranding(const QString& id, const SmartPointer<IConfigurationElement>& element)
  : id(id)
{
  if (element.IsNull()) return;

  application = element->GetAttribute(ATTR_APPLICATION);
  name = element->GetAttribute(ATTR_NAME);
  description = element->GetAttribute(ATTR_DESCRIPTION);
  LoadProperties(element);
}

QSharedPointer<ctkPlugin> ProductExtensionBranding::GetDefiningPlugin() const
{
  return definingPlugin;
}

QString ProductExtensionBranding::GetApplication() const
{
  return application;
}

QString ProductExtensionBranding::GetName() const
{
  return name;
}

QString ProductExtensionBranding::GetDescription() const
{
  return description;
}

QString ProductExtensionBranding::GetId() const
{
  return id;
}

QString ProductExtensionBranding::GetProperty(const QString& key) const
{
  auto iter = properties.find(key);
  return iter != properties.end() ? iter.value() : QString::null;
}

SmartPointer<IProduct> ProductExtensionBranding::GetProduct() const
{
  return IProduct::Pointer();
}

void ProductExtensionBranding::LoadProperties(const SmartPointer<IConfigurationElement>& element)
{
  QList<IConfigurationElement::Pointer> children = element->GetChildren();
  properties.clear();
  for (auto child : children)
  {
    QString key = child->GetAttribute(ATTR_NAME);
    QString value = child->GetAttribute(ATTR_VALUE);
    if (!key.isEmpty() && !value.isEmpty())
    {
      properties.insert(key, value);
    }
  }
  definingPlugin = org_blueberry_core_runtime_Activator::GetPlugin(element->GetContributor());
}

}
