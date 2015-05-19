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


#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>

#include "berryOSGiCoreTestSuite.h"

#include "berryObjectTest.h"
#include "berrySmartPointerTest.h"
#include "berryWeakPointerTest.h"
#include "berryMessageTest.h"

namespace berry {

OSGiCoreTestSuite::OSGiCoreTestSuite()
: CppUnit::TestSuite("OSGiCoreTestSuite")
{
  addTest(ObjectTest::Suite());
  addTest(SmartPointerTest::Suite());
  addTest(WeakPointerTest::Suite());
  addTest(MessageTest::Suite());
}

OSGiCoreTestSuite::OSGiCoreTestSuite(const OSGiCoreTestSuite& other)
{
  Q_UNUSED(other)
}

}
