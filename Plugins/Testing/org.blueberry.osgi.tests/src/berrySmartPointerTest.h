/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYSMARTPOINTERTEST_H_
#define BERRYSMARTPOINTERTEST_H_

#include <berryTestCase.h>

namespace berry {

class SmartPointerTest : public TestCase
{
public:

  SmartPointerTest(const std::string& testName);

  static CppUnit::Test* Suite();

  void TestConstructors();
  void TestCasting();
  void TestReferenceCounting();
  void TestOperators();

};

}

#endif /* BERRYSMARTPOINTERTEST_H_ */
