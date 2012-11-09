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


#ifndef BERRYBLUEBERRYTESTDRIVER_H_
#define BERRYBLUEBERRYTESTDRIVER_H_

#include <org_blueberry_test_Export.h>

#include "berryITestDescriptor.h"

#include <vector>

namespace berry
{

/**
 * A TestDriver for CppUnit that supports running tests inside BlueBerry as well as
 * running standalone.
 * Example call: TODO
 */
class BERRY_TEST_EXPORT BlueBerryTestDriver
{
public:

  BlueBerryTestDriver(const std::vector<ITestDescriptor::Pointer>& descriptors, bool uitests = false, const std::string& testName="", bool wait=false);

  int Run();

  static int Run(const std::string& pluginId, bool uitests = false);

protected:

  std::vector<ITestDescriptor::Pointer> descriptors;
  bool uitests;
  std::string testName;
  bool wait;
};

}

#endif /* BERRYBLUEBERRYTESTDRIVER_H_ */
