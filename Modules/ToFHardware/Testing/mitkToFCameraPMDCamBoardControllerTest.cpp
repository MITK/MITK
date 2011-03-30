/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-03-12 14:05:50 +0100 (Fr, 12 Mrz 2010) $
Version:   $Revision: 16010 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <mitkTestingMacros.h>
#include <mitkToFCameraPMDCamBoardController.h>

/**Documentation
 *  test for the class "ToFCameraPMDCamBoardController".
 */
int mitkToFCameraPMDCamBoardControllerTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("ToFCameraPMDCamBoardController");

  mitk::ToFCameraPMDCamBoardController::Pointer testObject = mitk::ToFCameraPMDCamBoardController::New();
  MITK_TEST_CONDITION_REQUIRED(!testObject.GetPointer()==NULL,"Testing initialzation!");
  MITK_TEST_CONDITION_REQUIRED(testObject->GetCaptureHeight()== 200 ,"Testing initialization of CaptureHeight");
  MITK_TEST_CONDITION_REQUIRED(testObject->GetCaptureWidth()== 200 ,"Testing initialization of CaptureWidth");
  MITK_TEST_CONDITION_REQUIRED(testObject->OpenCameraConnection(),"Testing opening of camera connection!");
  MITK_TEST_CONDITION_REQUIRED(testObject->UpdateCamera(),"Testing UpdateCamera()");
  MITK_TEST_CONDITION_REQUIRED(testObject->SetDistanceOffset(0.5f),"Testing SetDistanceOffset()");
  MITK_TEST_OUTPUT(<<"Call GetDistanceOffset()");
  MITK_INFO<<testObject->GetDistanceOffset();
  MITK_TEST_CONDITION_REQUIRED(testObject->SetRegionOfInterest(3,5,200,201),"Testing SetRegionOfInterest()");
  MITK_TEST_OUTPUT(<<"Call GetRegionOfInterest()");
  MITK_INFO<<testObject->GetRegionOfInterest();
  MITK_TEST_CONDITION_REQUIRED(testObject->SetExposureMode(0),"Testing SetExposureMode()");
  MITK_TEST_CONDITION_REQUIRED(testObject->SetFieldOfView(35.7f),"Testing SetFieldOfView()");
  MITK_TEST_CONDITION_REQUIRED(testObject->SetFPNCalibration(true),"Testing SetFPNCalibration()");
  MITK_TEST_CONDITION_REQUIRED(testObject->SetFPPNCalibration(true),"Testing SetFPPNCalibration()");
  MITK_TEST_CONDITION_REQUIRED(testObject->SetLinearityCalibration(true),"Testing SetLinearityCalibration()");
  MITK_TEST_CONDITION_REQUIRED(testObject->SetLensCalibration(true),"Testing SetLensCalibration()");
  MITK_TEST_CONDITION_REQUIRED(testObject->CloseCameraConnection(),"Testing closing of camera connection!");

  MITK_TEST_END();

}


