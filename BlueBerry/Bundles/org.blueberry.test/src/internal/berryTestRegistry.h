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


#ifndef BERRYTESTREGISTRY_H_
#define BERRYTESTREGISTRY_H_

#include <service/berryIConfigurationElement.h>

#include "berryITestDescriptor.h"

#include <Poco/HashMap.h>
#include <vector>

namespace berry {

class TestRegistry
{
public:

  static const std::string TAG_TEST; // = "test"
  static const std::string ATT_ID; // = "id"
  static const std::string ATT_CLASS; // = "class"
  static const std::string ATT_DESCRIPTION; // = "description"
  static const std::string ATT_UITEST; // = "uitest"

  static const std::string TEST_MANIFEST; // = "CppUnitTest"

  TestRegistry();

  const std::vector<ITestDescriptor::Pointer>& GetTestsForId(const std::string& pluginid);

protected:

  void ReadTest(IConfigurationElement::Pointer testElem);

private:

  Poco::HashMap<std::string, std::vector<ITestDescriptor::Pointer> > mapIdToTests;
};

}

#endif /* BERRYTESTREGISTRY_H_ */
