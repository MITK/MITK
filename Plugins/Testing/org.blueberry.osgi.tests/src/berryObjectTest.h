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


#ifndef BERRYOBJECTTEST_H_
#define BERRYOBJECTTEST_H_

#include <berryTestCase.h>

namespace berry {

class ObjectTest : public TestCase
{
public:

  static CppUnit::Test* Suite();

  ObjectTest(const std::string& testName);

  void TestReferenceCount();
  void TestAddDestroyListener();
  void TestRemoveDestroyListener();
};

}

#endif /* BERRYOBJECTTEST_H_ */
