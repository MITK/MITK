/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYITESTDESCRIPTOR_H_
#define BERRYITESTDESCRIPTOR_H_

#include <berryObject.h>
#include <berryMacros.h>

#include <cppunit/Test.h>

namespace berry {

struct ITestDescriptor : public Object
{
  berryObjectMacro(berry::ITestDescriptor)

  virtual CppUnit::Test* CreateTest() = 0;
  virtual QString GetId() const = 0;
  virtual QString GetContributor() const = 0;
  virtual QString GetDescription() const = 0;

  virtual bool IsUITest() const = 0;
};

}

#endif /* BERRYITESTDESCRIPTOR_H_ */
