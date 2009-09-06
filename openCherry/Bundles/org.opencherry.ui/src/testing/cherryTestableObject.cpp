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

#include "cherryTestableObject.h"

namespace cherry
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
