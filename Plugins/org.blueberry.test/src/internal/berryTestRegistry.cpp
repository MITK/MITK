/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
