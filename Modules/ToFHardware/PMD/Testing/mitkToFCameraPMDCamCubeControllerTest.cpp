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

#include <mitkTestingMacros.h>
#include <mitkToFCameraPMDCamCubeController.h>

/**Documentation
 *  test for the class "ToFCameraPMDCamCubeController".
 */
int mitkToFCameraPMDCamCubeControllerTest(int /* argc */, char* /*argv*/[])
{

  MITK_TEST_BEGIN("ToFCameraPMDCamCubeController");
  mitk::ToFCameraPMDCamCubeController::Pointer camCubeController = mitk::ToFCameraPMDCamCubeController::New();
  try
  {

  MITK_TEST_CONDITION_REQUIRED(camCubeController.IsNotNull(),"Testing initialzation!");
  MITK_TEST_CONDITION_REQUIRED(camCubeController->GetCaptureHeight()== 200 ,"Testing initialization of CaptureHeight");
  MITK_TEST_CONDITION_REQUIRED(camCubeController->GetCaptureWidth()== 200 ,"Testing initialization of CaptureWidth");
  MITK_TEST_CONDITION_REQUIRED(camCubeController->OpenCameraConnection(),"Testing opening of camera connection!");
  MITK_TEST_CONDITION_REQUIRED(camCubeController->UpdateCamera(),"Testing UpdateCamera()");
  MITK_TEST_CONDITION_REQUIRED(camCubeController->SetDistanceOffset(0.5f),"Testing SetDistanceOffset()");
  MITK_TEST_OUTPUT(<<"Call GetDistanceOffset()");
  MITK_INFO<<camCubeController->GetDistanceOffset();
  MITK_TEST_CONDITION_REQUIRED(camCubeController->SetRegionOfInterest(3,5,200,201),"Testing SetRegionOfInterest()");
  MITK_TEST_OUTPUT(<<"Call GetRegionOfInterest()");
  MITK_INFO<<camCubeController->GetRegionOfInterest();
  MITK_TEST_CONDITION_REQUIRED(camCubeController->SetExposureMode(0),"Testing SetExposureMode()");
  MITK_TEST_CONDITION_REQUIRED(camCubeController->SetFieldOfView(35.7f),"Testing SetFieldOfView()");
  MITK_TEST_CONDITION_REQUIRED(camCubeController->SetFPNCalibration(true),"Testing SetFPNCalibration()");
  MITK_TEST_CONDITION_REQUIRED(camCubeController->SetFPPNCalibration(true),"Testing SetFPPNCalibration()");
  MITK_TEST_CONDITION_REQUIRED(camCubeController->SetLinearityCalibration(true),"Testing SetLinearityCalibration()");
  MITK_TEST_CONDITION_REQUIRED(camCubeController->SetLensCalibration(true),"Testing SetLensCalibration()");
  MITK_TEST_CONDITION_REQUIRED(camCubeController->CloseCameraConnection(),"Testing closing of camera connection!");
  }
  catch(std::exception &e)
  {
      MITK_INFO << e.what();
  }

  MITK_TEST_END();
}
