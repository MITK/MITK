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
#include <mitkToFCameraPMDCamCubeDevice.h>

/**Documentation
 *  test for the class "ToFCameraPMDCamCubeDevice".
 */
int mitkToFCameraPMDCamCubeDeviceTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("ToFCameraPMDCamCubeDevice");

  mitk::ToFCameraPMDCamCubeDevice::Pointer tofCameraPMDCamCubeDevice = mitk::ToFCameraPMDCamCubeDevice::New();
  // No hardware attached for automatic testing -> test correct error handling
  MITK_TEST_CONDITION_REQUIRED(tofCameraPMDCamCubeDevice->ConnectCamera(), "Test ConnectCamera()");
  MITK_TEST_CONDITION_REQUIRED(!tofCameraPMDCamCubeDevice->IsCameraActive(), "Test IsCameraActive() before StartCamera()");
  MITK_TEST_OUTPUT(<<"Call StartCamera()");
  tofCameraPMDCamCubeDevice->StartCamera();
  MITK_TEST_CONDITION_REQUIRED(tofCameraPMDCamCubeDevice->IsCameraActive(), "Test IsCameraActive() after StartCamera()");
  MITK_TEST_OUTPUT(<<"Call UpdateCamera()");
  tofCameraPMDCamCubeDevice->UpdateCamera();
  int numberOfPixels = tofCameraPMDCamCubeDevice->GetCaptureWidth()*tofCameraPMDCamCubeDevice->GetCaptureHeight();
  MITK_INFO<<numberOfPixels;
  float* distances = new float[numberOfPixels];
  float* amplitudes = new float[numberOfPixels];
  float* intensities = new float[numberOfPixels];
  char* sourceData = new char[numberOfPixels];
  int requiredImageSequence = 0;
  int imageSequence = 0;
  tofCameraPMDCamCubeDevice->GetDistances(distances,imageSequence);
  tofCameraPMDCamCubeDevice->GetAmplitudes(amplitudes,imageSequence);
  tofCameraPMDCamCubeDevice->GetIntensities(intensities,imageSequence);
  tofCameraPMDCamCubeDevice->GetAllImages(distances,amplitudes,intensities,sourceData,requiredImageSequence,imageSequence);
  MITK_TEST_CONDITION_REQUIRED(tofCameraPMDCamCubeDevice->IsCameraActive(), "Test IsCameraActive() before StopCamera()");
  MITK_TEST_OUTPUT(<<"Call StopCamera()");
  tofCameraPMDCamCubeDevice->StopCamera();
  MITK_TEST_CONDITION_REQUIRED(!tofCameraPMDCamCubeDevice->IsCameraActive(), "Test IsCameraActive() after StopCamera()");

  MITK_TEST_CONDITION_REQUIRED(tofCameraPMDCamCubeDevice->DisconnectCamera(), "Test DisonnectCamera()");
  delete[] distances;
  delete[] amplitudes;
  delete[] intensities;
  delete[] sourceData;

  MITK_TEST_END();

}


