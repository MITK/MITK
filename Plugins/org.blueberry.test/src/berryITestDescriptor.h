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
