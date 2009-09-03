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


#ifndef CHERRYTESTREGISTRY_H_
#define CHERRYTESTREGISTRY_H_

#include <service/cherryIConfigurationElement.h>

#include "../cherryITestDescriptor.h"

#include <Poco/HashMap.h>
#include <vector>

namespace cherry {

class TestRegistry
{
public:

  static const std::string TAG_TEST; // = "test"
  static const std::string ATT_ID; // = "id"
  static const std::string ATT_CLASS; // = "class"
  static const std::string ATT_DESCRIPTION; // = "description"
  static const std::string ATT_UITEST; // = "uitest"

  TestRegistry();

  const std::vector<ITestDescriptor::Pointer>& GetTestsForId(const std::string& pluginid);

protected:

  void ReadTest(IConfigurationElement::Pointer testElem);

private:

  Poco::HashMap<std::string, std::vector<ITestDescriptor::Pointer> > mapIdToTests;
};

}

#endif /* CHERRYTESTREGISTRY_H_ */
