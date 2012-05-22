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
#include <mitkToFCameraPMDDevice.h>

/**Documentation
 *  test for the class "ToFCameraPMDDevice".
 */
int mitkToFCameraPMDDeviceTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("ToFCameraPMDDevice");

  mitk::ToFCameraPMDDevice::Pointer tofCameraPMDDevice = mitk::ToFCameraPMDDevice::New();
  // No hardware attached for automatic testing -> test correct error handling
  MITK_TEST_CONDITION_REQUIRED(!tofCameraPMDDevice->ConnectCamera(), "Test ConnectCamera()");
  MITK_TEST_CONDITION_REQUIRED(!tofCameraPMDDevice->IsCameraActive(), "Test IsCameraActive()");
  MITK_TEST_OUTPUT(<<"Call StartCamera()");
  tofCameraPMDDevice->StartCamera();
  MITK_TEST_OUTPUT(<<"Call UpdateCamera()");
  tofCameraPMDDevice->UpdateCamera();
  int numberOfPixels = tofCameraPMDDevice->GetCaptureWidth()*tofCameraPMDDevice->GetCaptureHeight();
  MITK_INFO<<numberOfPixels;
  float* distances = new float[numberOfPixels];
  float* amplitudes = new float[numberOfPixels];
  float* intensities = new float[numberOfPixels];
  char* sourceData = new char[numberOfPixels];
  int requiredImageSequence = 0;
  int imageSequence = 0;
  tofCameraPMDDevice->GetDistances(distances,imageSequence);
  tofCameraPMDDevice->GetAmplitudes(amplitudes,imageSequence);
  tofCameraPMDDevice->GetIntensities(intensities,imageSequence);
  tofCameraPMDDevice->GetAllImages(distances,amplitudes,intensities,sourceData,requiredImageSequence,imageSequence);
  MITK_TEST_OUTPUT(<<"Call StopCamera()");
  tofCameraPMDDevice->StopCamera();

  MITK_TEST_CONDITION_REQUIRED(!tofCameraPMDDevice->DisconnectCamera(), "Test DisonnectCamera()");
  delete[] distances;
  delete[] amplitudes;
  delete[] intensities;
  delete[] sourceData;

  MITK_TEST_END();

}


