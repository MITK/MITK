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


#ifndef CHERRYTESTDESCRIPTOR_H_
#define CHERRYTESTDESCRIPTOR_H_

#include "../cherryITestDescriptor.h"

#include <service/cherryIConfigurationElement.h>

namespace cherry {

class TestDescriptor : public ITestDescriptor
{
public:

  cherryObjectMacro(TestDescriptor)

  TestDescriptor(IConfigurationElement::Pointer elem);

  CppUnit::Test* CreateTest();
  std::string GetId() const;
  std::string GetContributor() const;
  std::string GetDescription() const;

  bool IsUITest() const;

private:

  IConfigurationElement::Pointer configElem;
};

}

#endif /* CHERRYTESTDESCRIPTOR_H_ */
