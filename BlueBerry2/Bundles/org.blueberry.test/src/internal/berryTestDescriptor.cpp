/*=========================================================================
 
 Program:   BlueBerry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$
 
 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.
 
 =========================================================================*/

#include "berryTestDescriptor.h"
#include "berryTestRegistry.h"

#include <Poco/String.h>

namespace berry
{

TestDescriptor::TestDescriptor(IConfigurationElement::Pointer elem) :
  configElem(elem)
{

}

CppUnit::Test* TestDescriptor::CreateTest()
{
  CppUnit::Test* test = configElem->CreateExecutableExtension<CppUnit::Test> (
      TestRegistry::ATT_CLASS, TestRegistry::TEST_MANIFEST);
  return test;
}

std::string TestDescriptor::GetId() const
{
  std::string id;
  configElem->GetAttribute(TestRegistry::ATT_ID, id);
  return id;
}

std::string TestDescriptor::GetContributor() const
{
  return configElem->GetContributor();
}

std::string TestDescriptor::GetDescription() const
{
  std::string descr;
  configElem->GetAttribute(TestRegistry::ATT_DESCRIPTION, descr);
  return descr;
}

bool TestDescriptor::IsUITest() const
{
  std::string isUi;
  if (configElem->GetAttribute(TestRegistry::ATT_UITEST, isUi))
  {
    return !Poco::icompare(isUi, "true");
  }

  return false;
}

}
