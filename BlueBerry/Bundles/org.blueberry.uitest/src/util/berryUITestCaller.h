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


#ifndef BERRYUITESTCALLER_H_
#define BERRYUITESTCALLER_H_

#include <CppUnit/TestCase.h>
#include <CppUnit/Guards.h>

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
  std::auto_ptr<Fixture> _fixture;
};


} // namespace berry


#define CppUnit_addUITest(suite, cls, mth) \
  suite->addTest(new berry::UITestCaller<cls>(#mth, &cls::mth))

#endif /* BERRYUITESTCALLER_H_ */
