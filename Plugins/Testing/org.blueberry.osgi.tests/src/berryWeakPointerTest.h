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
