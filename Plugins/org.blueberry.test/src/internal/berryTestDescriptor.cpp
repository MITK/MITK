/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryTestDescriptor.h"
#include "berryTestRegistry.h"
#include "berryIContributor.h"

#include <Poco/String.h>

Q_DECLARE_INTERFACE(CppUnit::Test, "CppUnit.Test")

namespace berry
{

TestDescriptor::TestDescriptor(IConfigurationElement::Pointer elem) :
  configElem(elem)
{

}

CppUnit::Test* TestDescriptor::CreateTest()
{
  CppUnit::Test* test = configElem->CreateExecutableExtension<CppUnit::Test> (
      TestRegistry::ATT_CLASS);
  if (test == 0)
  {
    // Try legacy BlueBerry manifests instead
    test = configElem->CreateExecutableExtension<CppUnit::Test> (
        TestRegistry::ATT_CLASS);
  }
  return test;
}

QString TestDescriptor::GetId() const
{
  return configElem->GetAttribute(TestRegistry::ATT_ID);
}

QString TestDescriptor::GetContributor() const
{
  return configElem->GetContributor()->GetName();
}

QString TestDescriptor::GetDescription() const
{
  return configElem->GetAttribute(TestRegistry::ATT_DESCRIPTION);
}

bool TestDescriptor::IsUITest() const
{
  QString isUi = configElem->GetAttribute(TestRegistry::ATT_UITEST);
  return isUi.compare("true", Qt::CaseInsensitive);
}

}
