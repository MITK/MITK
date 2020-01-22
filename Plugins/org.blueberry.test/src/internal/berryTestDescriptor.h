/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYTESTDESCRIPTOR_H_
#define BERRYTESTDESCRIPTOR_H_

#include "berryITestDescriptor.h"

#include <berryIConfigurationElement.h>

namespace berry {

class TestDescriptor : public ITestDescriptor
{
public:

  berryObjectMacro(TestDescriptor);

  TestDescriptor(IConfigurationElement::Pointer elem);

  CppUnit::Test* CreateTest();
  QString GetId() const;
  QString GetContributor() const;
  QString GetDescription() const;

  bool IsUITest() const;

private:

  IConfigurationElement::Pointer configElem;
};

}

#endif /* BERRYTESTDESCRIPTOR_H_ */
