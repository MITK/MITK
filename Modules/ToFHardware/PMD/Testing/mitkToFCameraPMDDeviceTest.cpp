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

  mitk::ToFCameraPMDDevice::Pointer PMDDevice = mitk::ToFCameraPMDDevice::New();
  try
  {
  // No hardware attached for automatic testing -> test correct error handling
  MITK_TEST_CONDITION_REQUIRED(!PMDDevice->ConnectCamera(), "Test ConnectCamera()");
  MITK_TEST_CONDITION_REQUIRED(!PMDDevice->IsCameraActive(), "Test IsCameraActive()");
  MITK_TEST_OUTPUT(<<"Call StartCamera()");
  PMDDevice->StartCamera();
  MITK_TEST_OUTPUT(<<"Call UpdateCamera()");
  PMDDevice->UpdateCamera();
  int numberOfPixels = PMDDevice->GetCaptureWidth()*PMDDevice->GetCaptureHeight();
  MITK_INFO<<numberOfPixels;
  float* distances = new float[numberOfPixels];
  float* amplitudes = new float[numberOfPixels];
  float* intensities = new float[numberOfPixels];
  char* sourceData = new char[numberOfPixels];
  int requiredImageSequence = 0;
  int imageSequence = 0;
  PMDDevice->GetDistances(distances,imageSequence);
  PMDDevice->GetAmplitudes(amplitudes,imageSequence);
  PMDDevice->GetIntensities(intensities,imageSequence);
  PMDDevice->GetAllImages(distances,amplitudes,intensities,sourceData,requiredImageSequence,imageSequence);
  MITK_TEST_OUTPUT(<<"Call StopCamera()");
  PMDDevice->StopCamera();

  MITK_TEST_CONDITION_REQUIRED(!PMDDevice->DisconnectCamera(), "Test DisonnectCamera()");
  delete[] distances;
  delete[] amplitudes;
  delete[] intensities;
  delete[] sourceData;
  }
  catch(std::exception &e)
  {
      MITK_INFO << e.what();
      MITK_TEST_CONDITION_REQUIRED(PMDDevice->IsCameraActive()==false, "Testing that no device could be connected.");
  }

  MITK_TEST_END();

}
