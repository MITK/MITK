/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYUITESTCALLER_H_
#define BERRYUITESTCALLER_H_

#include <cppunit/TestCase.h>
#include <cppunit/Guards.h>

#include <memory>

namespace berry {

template <class Fixture>
class UITestCaller: public CppUnit::TestCase
{
  REFERENCEOBJECT (UITestCaller)

  typedef void (Fixture::*TestMethod)();

public:
  UITestCaller(const std::string& name, TestMethod test):
    CppUnit::TestCase(name),
    _test(test),
    _fixture(new Fixture(name))
  {
  }

protected:
  void runTest()
  {
    (_fixture.get()->*_test)();
  }

  void setUp()
  {
    _fixture.get()->DoSetUp();
  }

  void tearDown()
  {
    _fixture.get()->DoTearDown();
  }

private:
  TestMethod             _test;
  std::unique_ptr<Fixture> _fixture;
};


} // namespace berry


#define CppUnit_addUITest(suite, cls, mth) \
  suite->addTest(new berry::UITestCaller<cls>(#mth, &cls::mth))

#endif /* BERRYUITESTCALLER_H_ */
