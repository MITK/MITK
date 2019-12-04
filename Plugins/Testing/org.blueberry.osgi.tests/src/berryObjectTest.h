/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


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
