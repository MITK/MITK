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


