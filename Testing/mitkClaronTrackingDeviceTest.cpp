/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-02-25 17:27:17 +0100 (Mo, 25 Feb 2008) $
Version:   $Revision: 7837 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkClaronTrackingDevice.h"
#include "mitkClaronTool.h"
#include "mitkTestingMacros.h"

class mitkClaronTrackingDeviceTestClass
  {

  public:

  static bool TestIsMicronTrackerInstalled()
    {
    mitk::ClaronTrackingDevice::Pointer myClaronTrackingDevice = mitk::ClaronTrackingDevice::New();
    bool returnValue = myClaronTrackingDevice->IsMicronTrackerInstalled();
    if (returnValue) {MITK_TEST_OUTPUT(<< "MicronTracker is installed on this system!")}
    else {MITK_TEST_OUTPUT(<< "MicronTracker is not installed on this system!")}
    return returnValue;
    }

  static void TestInstantiation()
    {
    // let's create an object of our class
    mitk::ClaronTrackingDevice::Pointer testInstance;
    testInstance = mitk::ClaronTrackingDevice::New();
    MITK_TEST_CONDITION_REQUIRED(testInstance.IsNotNull(),"Testing instantiation:")
    }

  static void TestToolConfiguration()
    {
    mitk::ClaronTrackingDevice::Pointer testInstance = mitk::ClaronTrackingDevice::New();
    mitk::ClaronTool::Pointer tool1 = mitk::ClaronTool::New();
    tool1->SetToolName("Tool1");
    testInstance->AddTool(tool1);
    MITK_TEST_CONDITION(testInstance->GetToolCount() == 1, "Testing adding tool 1");

    mitk::ClaronTool::Pointer tool2 = mitk::ClaronTool::New();
    tool2->SetToolName("Tool2");
    testInstance->AddTool(tool2);
    MITK_TEST_CONDITION(testInstance->GetToolCount() == 2, "Testing adding tool 2");
    mitk::ClaronTool::Pointer tool3 = mitk::ClaronTool::New();
    tool3->SetToolName("Tool3");
    testInstance->AddTool(tool3);
    MITK_TEST_CONDITION(testInstance->GetToolCount() == 3, "Testing adding tool 3");


    std::vector<mitk::ClaronTool::Pointer> myTools = testInstance->GetAllTools();
    MITK_TEST_CONDITION(myTools.at(0)->GetToolName() == std::string("Tool1"), "Testing GetAllTools() for tool 1");
    MITK_TEST_CONDITION(myTools.at(1)->GetToolName() == std::string("Tool2"), "Testing GetAllTools() for tool 2");
    MITK_TEST_CONDITION(myTools.at(2)->GetToolName() == std::string("Tool3"), "Testing GetAllTools() for tool 3");

    //Testing 100 tools (maximum by MicronTracker)
    testInstance = NULL;
    testInstance = mitk::ClaronTrackingDevice::New();
    for (unsigned int i = 0; i < 100; i++)
      testInstance->AddTool(mitk::ClaronTool::New());
    MITK_TEST_CONDITION(testInstance->GetToolCount() == 100, "Testing adding 100 tools");

    bool failed = false;
    int max = 100;

    testInstance = mitk::ClaronTrackingDevice::New();
    for(int i=0; i<max; i++) testInstance->AddTool(mitk::ClaronTool::New());
    if ((testInstance->GetToolCount()!=max)) failed = true;
    MITK_TEST_CONDITION(!failed,"Testing tool configuration (maximum of 100 tools):");
    }

  static void TestAllMethodsOnSystemsWithoutMicronTracker()
    {
    //In this case we won't recieve valid data but defined invalid return values.

    //initialize
    mitk::ClaronTrackingDevice::Pointer myClaronTrackingDevice = mitk::ClaronTrackingDevice::New();

    //OpenConnection
    MITK_TEST_CONDITION( (!myClaronTrackingDevice->OpenConnection()), "Testing behavior of method OpenConnection() (Errors should occur because MicronTracker is not activated).\n");

    //add a few tools
    mitk::ClaronTool::Pointer tool1 = mitk::ClaronTool::New(); tool1->SetToolName("Tool1");
    mitk::ClaronTool::Pointer tool2 = mitk::ClaronTool::New(); tool2->SetToolName("Tool2");
    mitk::ClaronTool::Pointer tool3 = mitk::ClaronTool::New(); tool3->SetToolName("Tool3");
    myClaronTrackingDevice->AddTool(tool1);
    myClaronTrackingDevice->AddTool(tool2);
    myClaronTrackingDevice->AddTool(tool3);

    //test IsMicronTrackerInstalled
    MITK_TEST_CONDITION(!myClaronTrackingDevice->IsMicronTrackerInstalled(),"Testing method IsMicronTrackerInstalled().\n")

    //test getAllTools
    std::vector<mitk::ClaronTool::Pointer> myTools = myClaronTrackingDevice->GetAllTools();
    MITK_TEST_CONDITION((myTools.size()==3), "Testing method GetAllTools().\n");

    //test getToolCount
    int toolCount = myClaronTrackingDevice->GetToolCount();
    MITK_TEST_CONDITION((toolCount==3),"Testing method GetToolCount().\n");

    //test getTool
    mitk::TrackingTool* myTool = myClaronTrackingDevice->GetTool(2);
    MITK_TEST_CONDITION((std::string(myTool->GetToolName())=="Tool3"), "Testing method GetTool().\n");

    //StartTracking
    MITK_TEST_CONDITION( (!myClaronTrackingDevice->StartTracking()), "Testing behavior of method StartTracking().\n");

    //StopTracking
    MITK_TEST_CONDITION( (myClaronTrackingDevice->StopTracking()), "Testing behavior of method StopTracking().\n");

    //CloseConnection
    MITK_TEST_CONDITION( (myClaronTrackingDevice->CloseConnection()), "Testing behavior of method CloseConnection().\n");
    }
  };

/**
 * This function is testing the Class ClaronTrackingDevice. For most tests we would need the MicronTracker hardware, so only a few
 * simple tests, which can run without the hardware are implemented yet (2009, January, 23rd). As soon as there is a working
 * concept to test the tracking classes which are very close to the hardware on all systems more tests are needed here.
 */
int mitkClaronTrackingDeviceTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("ClaronTrackingDevice")

  mitkClaronTrackingDeviceTestClass::TestInstantiation();
  mitkClaronTrackingDeviceTestClass::TestToolConfiguration();
  
  /* The following tests don't run under linux enviroments. This is or could be caused by the fact that the MicronTracker interface
   * is developped unter windows and not tested under linux yet (26.2.2009). So - in my opinion - the best approach is to first test
   * the MicronTracker code under linux (and make the nessesary changes of course) and parallelly write the linux tests or make this
   * tests runnable under linux.
   */
  #ifdef WIN32
  if (mitkClaronTrackingDeviceTestClass::TestIsMicronTrackerInstalled())
    {
    MITK_TEST_OUTPUT(<< "... MicronTracker is installed on your System, so we don't run any further tests. (All tests run on systems without MicronTracker)");
    }
  else
    {
    MITK_TEST_OUTPUT(<< ".Test");
    mitkClaronTrackingDeviceTestClass::TestAllMethodsOnSystemsWithoutMicronTracker();
    }
  #endif

  MITK_TEST_END();
}