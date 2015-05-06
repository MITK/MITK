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
