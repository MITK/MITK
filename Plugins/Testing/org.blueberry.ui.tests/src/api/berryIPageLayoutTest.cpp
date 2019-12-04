/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryIPageLayoutTest.h"

#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>

#include <berryEmptyPerspective.h>

namespace berry
{

IPageLayoutTest::IPageLayoutTest(const std::string& testName) :
  UITestCase(testName)
{

}

CppUnit::Test* IPageLayoutTest::Suite()
{
  CppUnit::TestSuite* suite = new CppUnit::TestSuite("IPageLayoutTest");

  CppUnit_addTest(suite, IPageLayoutTest, TestGetDescriptor);

  return suite;
}

void IPageLayoutTest::TestGetDescriptor()
{
  this->IgnoreLeakingObjects();
  //this->LeakDetailsOn();

  EmptyPerspective::SetLastPerspective("");
  OpenTestWindow(EmptyPerspective::PERSP_ID);
  assertEqual(EmptyPerspective::PERSP_ID, EmptyPerspective::GetLastPerspective());
}

}
