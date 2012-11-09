/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkClaronTrackingDevice.h"
#include "mitkClaronTool.h"
#include "mitkTestingMacros.h"
#include "mitkStandardFileLocations.h"

class mitkClaronTrackingDeviceHardwareTestClass
{
public:
};

/** This function is testing the Class ClaronTrackingDevice in interaction with the hardware.
  * So a MicronTracker Tracking System has to be installed and connected to run this test.
  * The test needs the filenames of three toolfiles as arguments.
  */
int mitkClaronTrackingDeviceHardwareTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("ClaronTrackingDeviceHardware");

  if (argc < 4) {MITK_TEST_FAILED_MSG(<<"Error: This tests need 3 toolfiles as arguments.");}
  std::string toolfile1 = argv[1];
  std::string toolfile2 = argv[2];
  std::string toolfile3 = argv[3];

  //create tracking device
  mitk::ClaronTrackingDevice::Pointer myDevice = mitk::ClaronTrackingDevice::New();
  MITK_TEST_OUTPUT(<<".. Creating tracking device.");

  //add tools
  mitk::TrackingTool::Pointer tool1 = myDevice->AddTool("Tool1",argv[1]);
  mitk::TrackingTool::Pointer tool2 = myDevice->AddTool("Tool2",argv[2]);
  mitk::TrackingTool::Pointer tool3 = myDevice->AddTool("Tool3",argv[3]);
  MITK_TEST_CONDITION(myDevice->GetToolCount() == 3, ".. Adding 3 tools to tracking device.");


  MITK_TEST_CONDITION(myDevice->OpenConnection(),"Opening connection to Tracking Device (MicronTracker).");
  MITK_TEST_CONDITION(myDevice->StartTracking(),"...start tracking");
  MITK_TEST_CONDITION(myDevice->StopTracking(),"...stop tracking");
  MITK_TEST_CONDITION(myDevice->CloseConnection(),"...close connection");
  //TODO: test hardware here
  MITK_TEST_END();
}

