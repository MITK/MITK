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
#include <mitkToFCameraPMDO3Device.h>

/**Documentation
 *  test for the class "ToFCameraPMDO3Device".
 */
int mitkToFCameraPMDO3DeviceTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("ToFCameraPMDO3Device");

  mitk::ToFCameraPMDO3Device::Pointer tofCameraPMDO3Device = mitk::ToFCameraPMDO3Device::New();
  // No hardware attached for automatic testing -> test correct error handling
  MITK_TEST_CONDITION_REQUIRED(tofCameraPMDO3Device->ConnectCamera(), "Test ConnectCamera()");
  MITK_TEST_CONDITION_REQUIRED(!tofCameraPMDO3Device->IsCameraActive(), "Test IsCameraActive() before StartCamera()");
  MITK_TEST_OUTPUT(<<"Call StartCamera()");
  tofCameraPMDO3Device->StartCamera();
  MITK_TEST_CONDITION_REQUIRED(tofCameraPMDO3Device->IsCameraActive(), "Test IsCameraActive() after StartCamera()");
  MITK_TEST_OUTPUT(<<"Call UpdateCamera()");
  tofCameraPMDO3Device->UpdateCamera();
  int numberOfPixels = tofCameraPMDO3Device->GetCaptureWidth()*tofCameraPMDO3Device->GetCaptureHeight();
  MITK_INFO<<numberOfPixels;
  float* distances = new float[numberOfPixels];
  float* amplitudes = new float[numberOfPixels];
  float* intensities = new float[numberOfPixels];
  char* sourceData = new char[numberOfPixels];
  int requiredImageSequence = 0;
  int imageSequence = 0;
  tofCameraPMDO3Device->GetDistances(distances,imageSequence);
  tofCameraPMDO3Device->GetAmplitudes(amplitudes,imageSequence);
  tofCameraPMDO3Device->GetIntensities(intensities,imageSequence);
  tofCameraPMDO3Device->GetAllImages(distances,amplitudes,intensities,sourceData,requiredImageSequence,imageSequence);
  MITK_TEST_CONDITION_REQUIRED(tofCameraPMDO3Device->IsCameraActive(), "Test IsCameraActive() before StopCamera()");
  MITK_TEST_OUTPUT(<<"Call StopCamera()");
  tofCameraPMDO3Device->StopCamera();
  MITK_TEST_CONDITION_REQUIRED(!tofCameraPMDO3Device->IsCameraActive(), "Test IsCameraActive() after StopCamera()");

  MITK_TEST_CONDITION_REQUIRED(tofCameraPMDO3Device->DisconnectCamera(), "Test DisonnectCamera()");
  delete[] distances;
  delete[] amplitudes;
  delete[] intensities;
  delete[] sourceData;

  MITK_TEST_END();

}
