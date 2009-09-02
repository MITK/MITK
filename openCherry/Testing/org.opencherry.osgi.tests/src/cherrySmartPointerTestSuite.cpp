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

#include "cherrySmartPointerTestSuite.h"

#include <CppUnit/TestCase.h>
#include <CppUnit/TestSuite.h>
#include <CppUnit/TestCaller.h>

namespace cherry {

class SmartPointerTest : public CppUnit::TestCase
{
public:

  SmartPointerTest(const std::string& name) : CppUnit::TestCase(name) {}

  static CppUnit::Test* Suite()
  {
    CppUnit::TestSuite* suite = new CppUnit::TestSuite("SmartPointerTest");

    CppUnit_addTest(suite, SmartPointerTest, testReferenceCount);

    return suite;
  }

  void testReferenceCount()
  {
    assert(1 == 1);
  }


};

CppUnit::Test* SmartPointerTestSuite::GetTest() const
{
  return SmartPointerTest::Suite();
}

}
