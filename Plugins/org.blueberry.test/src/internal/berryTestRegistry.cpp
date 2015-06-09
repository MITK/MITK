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

#include "berryTestRegistry.h"
#include "berryTestDescriptor.h"

#include <berryPlatform.h>
#include <berryIExtensionRegistry.h>

namespace berry
{

const QString TestRegistry::TAG_TEST = "test";
const QString TestRegistry::ATT_ID = "id";
const QString TestRegistry::ATT_CLASS = "class";
const QString TestRegistry::ATT_DESCRIPTION = "description";
const QString TestRegistry::ATT_UITEST = "uitest";

TestRegistry::TestRegistry()
{
  QList<IConfigurationElement::Pointer> elements(
      Platform::GetExtensionRegistry()->GetConfigurationElementsFor(
          "org.blueberry.tests"));

  foreach (const IConfigurationElement::Pointer& configElem, elements)
  {
    if (configElem->GetName() == TAG_TEST)
    {
      this->ReadTest(configElem);
    }
  }
}

const QList<ITestDescriptor::Pointer>&
TestRegistry::GetTestsForId(const QString& pluginid)
{
  return mapIdToTests[pluginid];
}

void TestRegistry::ReadTest(const IConfigurationElement::Pointer& testElem)
{
  ITestDescriptor::Pointer descriptor(new TestDescriptor(testElem));
  Q_ASSERT(!descriptor->GetId().isEmpty());
  mapIdToTests[descriptor->GetContributor()].push_back(descriptor);
}

}
