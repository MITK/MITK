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


