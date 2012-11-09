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


#include <CppUnit/TestCase.h>
#include <CppUnit/TestSuite.h>
#include <CppUnit/TestCaller.h>

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
