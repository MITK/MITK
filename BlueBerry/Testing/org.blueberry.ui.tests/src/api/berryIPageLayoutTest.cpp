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

#include "berryIPageLayoutTest.h"

#include <CppUnit/TestSuite.h>
#include <CppUnit/TestCaller.h>

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
