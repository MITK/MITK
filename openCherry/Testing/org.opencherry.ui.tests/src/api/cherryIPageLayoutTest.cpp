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

#include "cherryIPageLayoutTest.h"

#include <CppUnit/TestSuite.h>
#include <cherryUITestCaller.h>

#include <cherryEmptyPerspective.h>

namespace cherry
{

IPageLayoutTest::IPageLayoutTest(const std::string& testName) :
  UITestCase(testName)
{

}

CppUnit::Test* IPageLayoutTest::Suite()
{
  CppUnit::TestSuite* suite = new CppUnit::TestSuite("IPageLayoutTest");

  CppUnit_addUITest(suite, IPageLayoutTest, TestGetDescriptor);

  return suite;
}

void IPageLayoutTest::TestGetDescriptor()
{
  EmptyPerspective::SetLastPerspective("");
  OpenTestWindow(EmptyPerspective::PERSP_ID);
  assertEqual(EmptyPerspective::PERSP_ID, EmptyPerspective::GetLastPerspective());
}

}
