/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryTestableObject.h"

namespace berry
{

ITestHarness::Pointer TestableObject::GetTestHarness() const
{
  return testHarness;
}

void TestableObject::SetTestHarness(ITestHarness::Pointer testHarness)
{
  poco_assert(testHarness);
  this->testHarness = testHarness;
}

void TestableObject::RunTest(Poco::Runnable* testRunnable)
{
  testRunnable->run();
}

void TestableObject::TestingStarting()
{
  // do nothing
}

void TestableObject::TestingFinished()
{
  // do nothing
}

}
