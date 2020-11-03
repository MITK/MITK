/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkNDITrackingDevice.h"
#include "mitkNDIPassiveTool.h"
#include "mitkTestingMacros.h"
#include "mitkTrackingTypes.h"
#include "mitkTrackingTool.h"
#include "mitkIGTConfig.h"

/** @brief This test makes general tests of the methods of class NDITrackingDevice. The tracking device need not to be connected to the system to run this tests.
 *         A few methods which needs the tracking device to be connected are not tested here.
 */
int mitkNDITrackingDeviceTest(int /* argc */, char* /*argv*/[])
{
  // always start with this!
  MITK_TEST_BEGIN("NDITrackingDevice");

  // let's create an object of our class
  mitk::NDITrackingDevice::Pointer myNDITrackingDevice = mitk::NDITrackingDevice::New();

  // first test: did this work?
  // using MITK_TEST_CONDITION_REQUIRED makes the test stop after failure, since
  // it makes no sense to continue without an object.
  MITK_TEST_CONDITION_REQUIRED(myNDITrackingDevice.IsNotNull(),"Testing instantiation\n");

  //test method GetState
  MITK_TEST_CONDITION_REQUIRED(myNDITrackingDevice->GetState() == mitk::TrackingDevice::Setup ,"Checking tracking device state == setup.\n");

  //test method CloseConnection
  MITK_TEST_CONDITION( (myNDITrackingDevice->CloseConnection()), "Testing behavior of method CloseConnection().");

  //test method StartTracking
  MITK_TEST_CONDITION( (!myNDITrackingDevice->StartTracking()), "Testing behavior of method StartTracking().");

  //test method Beep(unsigned char count)
  MITK_TEST_CONDITION( (myNDITrackingDevice->Beep(3)== false), "Testing behavior of method Beep(). No Tracking device initialized!");

  std::string file(MITK_IGT_DATA_DIR);
  file.append("/SROMFile.rom");
  MITK_INFO << file;

  MITK_TEST_CONDITION( (myNDITrackingDevice->AddTool("Tool0", file.c_str()))!=nullptr, "Testing AddTool() for tool 0.");

  //test method GetToolCount()
  MITK_TEST_CONDITION( (myNDITrackingDevice->GetToolCount())==1, "Testing GetToolCount() for one tool.");

  //test method GetTool(unsigned int toolNumber)
  MITK_TEST_CONDITION( (myNDITrackingDevice->GetTool(0))!=nullptr, "Testing GetTool() for tool 0.");
  mitk::TrackingTool::Pointer testtool = myNDITrackingDevice->GetTool(0);

  //test method UpdateTool(mitk::TrackingTool* tool)
  MITK_TEST_CONDITION( (!myNDITrackingDevice->UpdateTool(testtool)), "Testing behavior of method UpdateTool().\n");

  //test method RemoveTool(mitk::TrackingTool* tool)
  MITK_TEST_CONDITION( (myNDITrackingDevice->RemoveTool(testtool)), "Testing RemoveTool()for tool 0.");

  //test method SetOperationMode(OperationMode mode)
  MITK_TEST_CONDITION( (myNDITrackingDevice->SetOperationMode( mitk::MarkerTracking3D )== true ), "Testing behavior of method SetOperationMode().\n");

  //test method GetOperationMode()
  myNDITrackingDevice->SetOperationMode(mitk::MarkerTracking3D);
  MITK_TEST_CONDITION( (myNDITrackingDevice->GetOperationMode()==2),"" );

  myNDITrackingDevice->SetOperationMode(mitk::ToolTracking5D);
  MITK_TEST_CONDITION( (myNDITrackingDevice->GetOperationMode()==1),"" );

  myNDITrackingDevice->SetOperationMode(mitk::HybridTracking);
  MITK_TEST_CONDITION( (myNDITrackingDevice->GetOperationMode()==3), "Testing behavior of method GetOperationMode().\n");

  //test method GetMarkerPositions(MarkerPointContainerType* markerpositions)
  auto  markerpositions = new mitk::MarkerPointContainerType();
  MITK_TEST_CONDITION( (!myNDITrackingDevice->GetMarkerPositions(markerpositions)), "Testing behavior of method GetMarkerPositions().\n");
  delete markerpositions;

  // always end with this!
  MITK_TEST_END();
}
