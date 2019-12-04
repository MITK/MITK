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
