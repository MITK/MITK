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
#include <mitkKinectDevice.h>

/**Documentation
 *  test for the class "KinectDevice".
 */
int mitkKinectDeviceTest(int /* argc */, char* /*argv*/[])
{

  MITK_TEST_BEGIN("KinectDevice");
  //Try to connect a device. In failure case an exception is thrown.
  mitk::KinectDevice::Pointer kinectDevice = mitk::KinectDevice::New();
  try
  {
  MITK_TEST_CONDITION_REQUIRED(kinectDevice.IsNotNull(),"Testing initialzation!");
  MITK_TEST_CONDITION_REQUIRED(kinectDevice->ConnectCamera(),"Testing ConnectCamera()");
  MITK_TEST_OUTPUT(<<"Testing StartCamera()");
  kinectDevice->StartCamera();
  int captureHeight = kinectDevice->GetCaptureHeight();
  int captureWidth = kinectDevice->GetCaptureWidth();
  MITK_TEST_CONDITION_REQUIRED(captureHeight== 480 ,"Testing initialization of CaptureHeight");
  MITK_TEST_CONDITION_REQUIRED(captureWidth== 640 ,"Testing initialization of CaptureWidth");
  //
  MITK_TEST_OUTPUT(<<"Testing StopCamera()");
  kinectDevice->StopCamera();
  MITK_TEST_CONDITION_REQUIRED(kinectDevice->DisconnectCamera(),"Testing DisconnectCamera()");
  }
  catch(std::exception &e)
  {
      MITK_INFO << e.what();
      MITK_TEST_CONDITION(!kinectDevice->IsCameraActive(), "Testing that no device could be connected.");
  }
  MITK_TEST_END();

}
