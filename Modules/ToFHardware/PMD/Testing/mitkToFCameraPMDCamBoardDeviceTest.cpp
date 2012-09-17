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
#include <mitkToFCameraPMDCamBoardDevice.h>

/**Documentation
 *  test for the class "ToFCameraPMDCamBoardDevice".
 */
int mitkToFCameraPMDCamBoardDeviceTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("ToFCameraPMDCamBoardDevice");

  mitk::ToFCameraPMDCamBoardDevice::Pointer tofCameraPMDCamBoardDevice = mitk::ToFCameraPMDCamBoardDevice::New();
  // No hardware attached for automatic testing -> test correct error handling
  MITK_TEST_CONDITION_REQUIRED(tofCameraPMDCamBoardDevice->ConnectCamera(), "Test ConnectCamera()");
  MITK_TEST_CONDITION_REQUIRED(!tofCameraPMDCamBoardDevice->IsCameraActive(), "Test IsCameraActive() before StartCamera()");
  MITK_TEST_OUTPUT(<<"Call StartCamera()");
  tofCameraPMDCamBoardDevice->StartCamera();
  MITK_TEST_CONDITION_REQUIRED(tofCameraPMDCamBoardDevice->IsCameraActive(), "Test IsCameraActive() after StartCamera()");
  MITK_TEST_OUTPUT(<<"Call UpdateCamera()");
  tofCameraPMDCamBoardDevice->UpdateCamera();
  int numberOfPixels = tofCameraPMDCamBoardDevice->GetCaptureWidth()*tofCameraPMDCamBoardDevice->GetCaptureHeight();
  MITK_INFO<<numberOfPixels;
  float* distances = new float[numberOfPixels];
  float* amplitudes = new float[numberOfPixels];
  float* intensities = new float[numberOfPixels];
  char* sourceData = new char[numberOfPixels];
  int requiredImageSequence = 0;
  int imageSequence = 0;
  tofCameraPMDCamBoardDevice->GetDistances(distances,imageSequence);
  tofCameraPMDCamBoardDevice->GetAmplitudes(amplitudes,imageSequence);
  tofCameraPMDCamBoardDevice->GetIntensities(intensities,imageSequence);
  tofCameraPMDCamBoardDevice->GetAllImages(distances,amplitudes,intensities,sourceData,requiredImageSequence,imageSequence, 0);
  MITK_TEST_CONDITION_REQUIRED(tofCameraPMDCamBoardDevice->IsCameraActive(), "Test IsCameraActive() before StopCamera()");
  MITK_TEST_OUTPUT(<<"Call StopCamera()");
  tofCameraPMDCamBoardDevice->StopCamera();
  MITK_TEST_CONDITION_REQUIRED(!tofCameraPMDCamBoardDevice->IsCameraActive(), "Test IsCameraActive() after StopCamera()");

  MITK_TEST_CONDITION_REQUIRED(tofCameraPMDCamBoardDevice->DisconnectCamera(), "Test DisonnectCamera()");
  delete[] distances;
  delete[] amplitudes;
  delete[] intensities;
  delete[] sourceData;

  MITK_TEST_END();

}
