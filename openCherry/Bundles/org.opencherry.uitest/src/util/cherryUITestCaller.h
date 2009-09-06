/*=========================================================================
 
 Program:   openCherry Platform
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


#ifndef CHERRYUITESTCALLER_H_
#define CHERRYUITESTCALLER_H_

#include <CppUnit/TestCase.h>
#include <CppUnit/Guards.h>

#include <memory>

namespace cherry {

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


} // namespace cherry


#define CppUnit_addUITest(suite, cls, mth) \
  suite->addTest(new cherry::UITestCaller<cls>(#mth, &cls::mth))

#endif /* CHERRYUITESTCALLER_H_ */
