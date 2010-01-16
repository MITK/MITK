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


#ifndef BERRYBLUEBERRYTESTDRIVER_H_
#define BERRYBLUEBERRYTESTDRIVER_H_

#include "berryTestDll.h"

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
