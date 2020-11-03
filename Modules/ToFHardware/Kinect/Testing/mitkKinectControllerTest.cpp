/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkTestingMacros.h>
#include <mitkKinectController.h>

/**Documentation
 *  test for the class "KinectController".
 */
int mitkKinectControllerTest(int /* argc */, char* /*argv*/[])
{

  MITK_TEST_BEGIN("KinectController");

  mitk::KinectController::Pointer kinectController = mitk::KinectController::New();
  //try to work with a kinect controller
  try
  {
  MITK_TEST_CONDITION_REQUIRED(kinectController.IsNotNull(),"Testing initialzation!");
  MITK_TEST_CONDITION_REQUIRED(kinectController->GetCaptureHeight()== 480 ,"Testing initialization of CaptureHeight");
  MITK_TEST_CONDITION_REQUIRED(kinectController->GetCaptureWidth()== 640 ,"Testing initialization of CaptureWidth");
  MITK_TEST_CONDITION_REQUIRED(kinectController->OpenCameraConnection(),"Testing opening of camera connection!");
  MITK_TEST_CONDITION_REQUIRED(kinectController->UpdateCamera(),"Testing UpdateCamera()");
  MITK_TEST_CONDITION_REQUIRED(kinectController->CloseCameraConnection(),"Testing closing of camera connection!");
  }
  catch(std::exception &e)
  {
      MITK_INFO << e.what();
  }

  MITK_TEST_END();

}
