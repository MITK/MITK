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
