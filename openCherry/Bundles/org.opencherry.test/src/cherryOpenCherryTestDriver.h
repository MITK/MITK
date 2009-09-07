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


#ifndef CHERRYOPENCHERRYTESTDRIVER_H_
#define CHERRYOPENCHERRYTESTDRIVER_H_

#include "cherryTestDll.h"

#include "cherryITestDescriptor.h"

#include <vector>

namespace cherry
{

/**
 * A TestDriver for CppUnit that supports running tests inside openCherry as well as
 * running standalone.
 * Example call: TODO
 */
class CHERRY_TEST_EXPORT OpenCherryTestDriver
{
public:

  OpenCherryTestDriver(const std::vector<ITestDescriptor::Pointer>& descriptors, bool uitests = false, const std::string& testName="", bool wait=false);

  int Run();

  static int Run(const std::string& pluginId, bool uitests = false);

protected:

  std::vector<ITestDescriptor::Pointer> descriptors;
  bool uitests;
  std::string testName;
  bool wait;
};

}

#endif /* CHERRYOPENCHERRYTESTDRIVER_H_ */
