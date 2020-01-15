/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>

#include "berryCoreRuntimeTestSuite.h"

#include "berryPreferencesTest.h"
#include "berryPreferencesServiceTest.h"

namespace berry {

CoreRuntimeTestSuite::CoreRuntimeTestSuite(const CoreRuntimeTestSuite& other)
{

}

CoreRuntimeTestSuite::CoreRuntimeTestSuite()
: CppUnit::TestSuite("CoreRuntimeTestSuite")
{
  addTest(PreferencesTest::Suite());
  addTest(PreferencesServiceTest::Suite());
}

}
