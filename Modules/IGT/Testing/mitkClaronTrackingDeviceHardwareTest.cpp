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
#include "mitkStandardFileLocations.h"
#include <mitkIGTConfig.h>

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
  std::string calibrdir("");
#ifdef MITK_MICRON_TRACKER_CALIBRATION_DIR
   calibrdir = MITK_MICRON_TRACKER_CALIBRATION_DIR;
#endif

  MITK_TEST_CONDITION_REQUIRED( (calibrdir.empty() == false), "MITK_MICRON_TRACKER_CALIBRATION_DIR was not set in CMake, but is required for the test.");

  //create tracking device
  mitk::ClaronTrackingDevice::Pointer myDevice = mitk::ClaronTrackingDevice::New();
  myDevice->SetCalibrationDir(calibrdir.c_str());
  MITK_TEST_OUTPUT(<<".. Creating tracking device.");

  //add tools
  mitk::TrackingTool::Pointer tool1 = myDevice->AddTool("Tool1",toolfile1.c_str());
  mitk::TrackingTool::Pointer tool2 = myDevice->AddTool("Tool2",toolfile2.c_str());
  mitk::TrackingTool::Pointer tool3 = myDevice->AddTool("Tool3",toolfile3.c_str());
  MITK_TEST_CONDITION(myDevice->GetToolCount() == 3, ".. Adding 3 tools to tracking device.");


  MITK_TEST_CONDITION(myDevice->OpenConnection(),"Opening connection to Tracking Device (MicronTracker).");
  MITK_TEST_CONDITION(myDevice->StartTracking(),"...start tracking");
  MITK_TEST_CONDITION(myDevice->StopTracking(),"...stop tracking");
  MITK_TEST_CONDITION(myDevice->CloseConnection(),"...close connection");
  //TODO: test hardware here
  MITK_TEST_END();
}

