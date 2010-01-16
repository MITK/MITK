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


#ifndef BERRYTESTDESCRIPTOR_H_
#define BERRYTESTDESCRIPTOR_H_

#include "../berryITestDescriptor.h"

#include <service/berryIConfigurationElement.h>

namespace berry {

class TestDescriptor : public ITestDescriptor
{
public:

  berryObjectMacro(TestDescriptor)

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

#endif /* BERRYTESTDESCRIPTOR_H_ */
