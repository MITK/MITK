/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryPropertyTester.h"

#include "berryPlatform.h"
#include "berryIContributor.h"

#include <ctkPlugin.h>

namespace berry
{

void PropertyTester::InternalInitialize(PropertyTesterDescriptor::Pointer descriptor)
{
  fProperties = descriptor->GetProperties();
  fNamespace= descriptor->GetNamespace();
  fConfigElement= descriptor->GetExtensionElement();
}

PropertyTesterDescriptor::Pointer PropertyTester::InternalCreateDescriptor()
{
  PropertyTesterDescriptor::Pointer tester(new PropertyTesterDescriptor(fConfigElement, fNamespace, fProperties));
  return tester;
}

bool PropertyTester::Handles(const QString &namespaze,
    const QString &property)
{
  return fNamespace == namespaze && fProperties.contains("," + property + ",");
}

bool PropertyTester::IsInstantiated()
{
  return true;
}

bool PropertyTester::IsDeclaringPluginActive()
{
  QSharedPointer<ctkPlugin> plugin = Platform::GetPlugin(fConfigElement->GetContributor()->GetName());
  return plugin->getState() == ctkPlugin::ACTIVE;
}

IPropertyTester* PropertyTester::Instantiate()
{
  return this;
}


}  // namespace berry
