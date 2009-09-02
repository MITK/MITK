/*=========================================================================
 
 Program:   openCherry Platform
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

#include "cherryTestRegistry.h"
#include "cherryTestDescriptor.h"

#include <cherryPlatform.h>
#include <service/cherryIExtensionPointService.h>

namespace cherry
{

const std::string TestRegistry::TAG_TEST = "test";
const std::string TestRegistry::ATT_ID = "id";
const std::string TestRegistry::ATT_CLASS = "class";
const std::string TestRegistry::ATT_DESCRIPTION = "description";

TestRegistry::TestRegistry()
{
  std::vector<IConfigurationElement::Pointer> elements(
      Platform::GetExtensionPointService()->GetConfigurationElementsFor(
          "org.opencherry.tests"));

  for (std::vector<IConfigurationElement::Pointer>::iterator i =
      elements.begin(); i != elements.end(); ++i)
  {
    if ((*i)->GetName() == TAG_TEST)
    {
      this->ReadTest(*i);
    }
  }
}

const std::vector<ITestDescriptor::Pointer>&
TestRegistry::GetTestsForId(const std::string& pluginid)
{
  return mapIdToTests[pluginid];
}

void TestRegistry::ReadTest(IConfigurationElement::Pointer testElem)
{
  ITestDescriptor::Pointer descriptor(new TestDescriptor(testElem));
  poco_assert(descriptor->GetId() != "");
  mapIdToTests[descriptor->GetContributor()].push_back(descriptor);
}

}
