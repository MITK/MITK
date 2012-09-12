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
#include <mitkToFCameraPMDRawDataDevice.h>

/**Documentation
 *  test for the class "ToFCameraPMDDevice".
 */
int mitkToFCameraPMDRawDataDeviceTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("ToFCameraPMDRawDataDevice");

  mitk::ToFCameraPMDRawDataDevice::Pointer tofCameraPMDRawDataDevice = mitk::ToFCameraPMDRawDataDevice::New();
  // No hardware attached for automatic testing -> test correct error handling
  MITK_TEST_CONDITION_REQUIRED(!tofCameraPMDRawDataDevice->ConnectCamera(), "Test ConnectCamera()");
  MITK_TEST_CONDITION_REQUIRED(!tofCameraPMDRawDataDevice->IsCameraActive(), "Test IsCameraActive()");
  MITK_TEST_OUTPUT(<<"Call StartCamera()");
  tofCameraPMDRawDataDevice->StartCamera();
  MITK_TEST_OUTPUT(<<"Call UpdateCamera()");
  tofCameraPMDRawDataDevice->UpdateCamera();
  int numberOfPixels = tofCameraPMDRawDataDevice->GetCaptureWidth()*tofCameraPMDRawDataDevice->GetCaptureHeight();
  MITK_INFO<<numberOfPixels;
  float* distances = new float[numberOfPixels];
  float* amplitudes = new float[numberOfPixels];
  float* intensities = new float[numberOfPixels];
  char* sourceData = new char[numberOfPixels];
  int requiredImageSequence = 0;
  int imageSequence = 0;
  tofCameraPMDRawDataDevice->GetDistances(distances,imageSequence);
  tofCameraPMDRawDataDevice->GetAmplitudes(amplitudes,imageSequence);
  tofCameraPMDRawDataDevice->GetIntensities(intensities,imageSequence);
  tofCameraPMDRawDataDevice->GetAllImages(distances,amplitudes,intensities,sourceData,requiredImageSequence,imageSequence);
  MITK_TEST_OUTPUT(<<"Call StopCamera()");
  tofCameraPMDRawDataDevice->StopCamera();

  MITK_TEST_CONDITION_REQUIRED(!tofCameraPMDRawDataDevice->DisconnectCamera(), "Test DisonnectCamera()");
  delete[] distances;
  delete[] amplitudes;
  delete[] intensities;
  delete[] sourceData;

  MITK_TEST_END();

}
