/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYWEAKPOINTERTEST_H_
#define BERRYWEAKPOINTERTEST_H_

#include <berryTestCase.h>

namespace berry {

class WeakPointerTest : public TestCase
{
public:

  WeakPointerTest(const std::string& testName);

  static CppUnit::Test* Suite();

  void TestReferenceCounting();
  void TestConstructorAndLock();
  void TestOperators();
};

}

#endif /* BERRYWEAKPOINTERTEST_H_ */
