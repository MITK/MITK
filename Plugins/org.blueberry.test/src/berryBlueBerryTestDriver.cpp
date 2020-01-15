/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryBlueBerryTestDriver.h"

#include "internal/berryTestRegistry.h"

#include "cppunit/TestRunner.h"
#include "cppunit/TestResult.h"
#include "cppunit/TestResultCollector.h"

namespace berry
{

BlueBerryTestDriver::BlueBerryTestDriver(
    const QList<ITestDescriptor::Pointer>& descriptors,
    bool uitests,
    const QString& testName,
    bool wait)
  : descriptors(descriptors)
  , uitests(uitests)
  , testName(testName)
  , wait(wait)
{

}

int BlueBerryTestDriver::Run()
{
  CppUnit::TestRunner runner;

  unsigned int testCounter = 0;
  foreach (const ITestDescriptor::Pointer& descr, descriptors)
  {
    if (descr->IsUITest() == uitests)
    {
      CppUnit::Test* test = descr->CreateTest();
      runner.addTest(test);
      ++testCounter;
    }
  }

  if (testCounter == 0)
  {
    std::cout << "No " << (uitests ? "UI " : "") << "tests registered."
        << std::endl;
    return 0;
  }

  /*
  std::vector<std::string> args;
  args.push_back("BlueBerryTestDriver");
  if (testName.empty())
    args.push_back("-all");
  else
    args.push_back(testName);
  if (wait)
    args.push_back("-wait");

  return runner.run(args) ? 0 : 1;
  */

  CppUnit::TestResult controller;

  CppUnit::TestResultCollector result;
  controller.addListener(&result);

  runner.run(controller);
  return result.wasSuccessful() ? 0 : 1;
}

int BlueBerryTestDriver::Run(const QString& pluginId, bool uitests)
{
  TestRegistry testRegistry;
  const QList<ITestDescriptor::Pointer>& tests = testRegistry.GetTestsForId(
      pluginId);

  BlueBerryTestDriver driver(tests, uitests);
  return driver.Run();
}

}
