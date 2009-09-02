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

#include "cherryOpenCherryTestDriver.h"

#include "internal/cherryTestRegistry.h"

#include <CppUnit/TestRunner.h>

namespace cherry
{

OpenCherryTestDriver::OpenCherryTestDriver(const std::vector<
    ITestDescriptor::Pointer>& descriptors, const std::string& testName,
    bool wait) :
  descriptors(descriptors), testName(testName), wait(wait)
{

}

int OpenCherryTestDriver::Run()
{
  CppUnit::TestRunner runner;

  for (std::vector<ITestDescriptor::Pointer>::iterator i = descriptors.begin(); i
      != descriptors.end(); ++i)
  {
    ITestDescriptor::Pointer descr(*i);
    ITest::Pointer test(descr->CreateTest());
    runner.addTest(descr->GetId(), test->GetTest());
  }

  std::vector<std::string> args;
  args.push_back("OpenCherryTestDriver");
  if (testName.empty())
    args.push_back("-all");
  else
    args.push_back(testName);
  if (wait)
    args.push_back("-wait");

  return runner.run(args) ? 0 : 1;
}

int OpenCherryTestDriver::Run(const std::string& pluginId)
{
  TestRegistry testRegistry;
  const std::vector<ITestDescriptor::Pointer>& tests = testRegistry.GetTestsForId(
      pluginId);

  if (tests.empty())
  {
    std::cout << "No tests for plugin " << pluginId << " registered."
        << std::endl;
    return 0;
  }

  OpenCherryTestDriver driver(tests);
  return driver.Run();
}

}
