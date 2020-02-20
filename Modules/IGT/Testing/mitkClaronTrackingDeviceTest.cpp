/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkClaronTrackingDevice.h"
#include "mitkClaronTool.h"
#include "mitkTestingMacros.h"
#include "mitkIGTConfig.h"
#include "mitkIGTException.h"

#ifdef WIN32
static bool TestIsMicronTrackerInstalled()
{
  mitk::ClaronTrackingDevice::Pointer myClaronTrackingDevice = mitk::ClaronTrackingDevice::New();
  bool returnValue = myClaronTrackingDevice->IsDeviceInstalled();
  if (returnValue) {MITK_TEST_OUTPUT(<< "MicronTracker is installed on this system!")}
  else {MITK_TEST_OUTPUT(<< "MicronTracker is not installed on this system!")}
  return returnValue;
}
#endif

static void TestInstantiation()
{
  // let's create an object of our class
  mitk::ClaronTrackingDevice::Pointer testInstance;
  testInstance = mitk::ClaronTrackingDevice::New();
  MITK_TEST_CONDITION_REQUIRED(testInstance.IsNotNull(),"Testing instantiation:")
}

static void TestToolConfiguration()
{
  std::string toolFileName(MITK_IGT_DATA_DIR);
  toolFileName.append("/ClaronTool");

  mitk::ClaronTrackingDevice::Pointer testInstance = mitk::ClaronTrackingDevice::New();
  MITK_TEST_CONDITION(testInstance->AddTool("Tool1", toolFileName.c_str()) != nullptr, "Testing AddTool() for tool 1");
  MITK_TEST_CONDITION(testInstance->GetToolCount() == 1, "Testing adding tool 1");
  MITK_TEST_CONDITION(testInstance->AddTool("Tool2", toolFileName.c_str()) != nullptr, "Testing AddTool() for tool 2");
  MITK_TEST_CONDITION(testInstance->GetToolCount() == 2, "Testing adding tool 2");
  MITK_TEST_CONDITION(testInstance->AddTool("Tool3", toolFileName.c_str()) != nullptr, "Testing AddTool() for tool 3");
  MITK_TEST_CONDITION(testInstance->GetToolCount() == 3, "Testing adding tool 3");


  //std::vector<mitk::ClaronTool::Pointer> myTools = testInstance->GetAllTools();
  MITK_TEST_CONDITION(testInstance->GetTool(0)->GetToolName() == std::string("Tool1"), "Testing GetTool() for tool 1");
  MITK_TEST_CONDITION(testInstance->GetTool(1)->GetToolName() == std::string("Tool2"), "Testing GetTool() for tool 2");
  MITK_TEST_CONDITION(testInstance->GetTool(2)->GetToolName() == std::string("Tool3"), "Testing GetTool() for tool 3");

  //Testing 100 tools (maximum by MicronTracker)
  testInstance = nullptr;
  testInstance = mitk::ClaronTrackingDevice::New();
  for (unsigned int i = 0; i < 100; i++)
    testInstance->AddTool("Tool", toolFileName.c_str());
  MITK_TEST_CONDITION(testInstance->GetToolCount() == 100, "Testing adding 100 tools");

  bool failed = false;
  unsigned int max = 100;

  testInstance = mitk::ClaronTrackingDevice::New();
  for (unsigned int i = 0; i < max; i++)
    testInstance->AddTool("Tool", toolFileName.c_str());
  if ((testInstance->GetToolCount() != max))
    failed = true;
  MITK_TEST_CONDITION(!failed, "Testing tool configuration (maximum of 100 tools):");
}

#ifdef WIN32
static void TestAllMethodsOnSystemsWithoutMicronTracker()
{
  //In this case we won't receive valid data but defined invalid return values.

  //initialize
  mitk::ClaronTrackingDevice::Pointer myClaronTrackingDevice = mitk::ClaronTrackingDevice::New();

  //OpenConnection
  MITK_TEST_OUTPUT(<<"Testing behavior of method OpenConnection() (Errors should occur because MicronTracker is not activated).");
  MITK_TEST_FOR_EXCEPTION_BEGIN(mitk::IGTException)
    myClaronTrackingDevice->OpenConnection();
  MITK_TEST_FOR_EXCEPTION_END(mitk::IGTException)

  std::string toolFileName(MITK_IGT_DATA_DIR);
  toolFileName.append("/ClaronTool");

  //add a few tools
  myClaronTrackingDevice->AddTool("Tool1", toolFileName.c_str());
  myClaronTrackingDevice->AddTool("Tool2", toolFileName.c_str());
  myClaronTrackingDevice->AddTool("Tool3", toolFileName.c_str());

  //test IsMicronTrackerInstalled
  MITK_TEST_CONDITION(!myClaronTrackingDevice->IsDeviceInstalled(),"Testing method IsMicronTrackerInstalled().\n")

    //test getToolCount
    int toolCount = myClaronTrackingDevice->GetToolCount();
  MITK_TEST_CONDITION((toolCount==3), "Testing method GetToolCount().\n");

  //test getTool
  mitk::TrackingTool* myTool = myClaronTrackingDevice->GetTool(2);
  MITK_TEST_CONDITION((std::string(myTool->GetToolName()) == "Tool3"), "Testing method GetTool().\n");

  //StartTracking
  MITK_TEST_CONDITION( (!myClaronTrackingDevice->StartTracking()), "Testing behavior of method StartTracking().\n");

  //StopTracking
  MITK_TEST_CONDITION( (myClaronTrackingDevice->StopTracking()), "Testing behavior of method StopTracking().\n");

  //CloseConnection
  MITK_TEST_CONDITION( (myClaronTrackingDevice->CloseConnection()), "Testing behavior of method CloseConnection().\n");
}
#endif

/**
* This function is testing methods of the class ClaronTrackingDevice which are independent from the hardware. For more tests we would need
* the MicronTracker hardware, so only a few simple tests, which can run without the hardware are tested here. More tests can be found in the
* class ClaronTrackingDeviceHardwareTests which tests the interaction with the hardware.
*/
int mitkClaronTrackingDeviceTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("ClaronTrackingDevice");

  TestInstantiation();
  TestToolConfiguration();

  /* The following tests don't run under linux environments. This is or could be caused by the fact that the MicronTracker interface
  * is developed under windows and not tested under linux yet (26.2.2009). So - in my opinion - the best approach is to first test
  * the MicronTracker code under linux (and make the necessary changes of course) and in parallel write the linux tests or make this
  * tests runnable under linux.
  */
#ifdef WIN32
  if (TestIsMicronTrackerInstalled())
  {
    MITK_TEST_OUTPUT(<< "... MicronTracker is installed on your System, so we don't run any further tests. (All tests run on systems without MicronTracker)");
  }
  else
  {
    MITK_TEST_OUTPUT(<< ".Test");
    TestAllMethodsOnSystemsWithoutMicronTracker();
  }
#endif
  MITK_TEST_END();
}
