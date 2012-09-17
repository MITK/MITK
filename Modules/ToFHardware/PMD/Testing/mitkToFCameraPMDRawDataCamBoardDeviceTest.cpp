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
#include <mitkToFCameraPMDRawDataCamBoardDevice.h>

// vtk includes
#include "vtkShortArray.h"

/**Documentation
 *  test for the class "ToFCameraPMDCamBoardDevice".
 */
int mitkToFCameraPMDRawDataCamBoardDeviceTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("ToFCameraPMDRawDataCamBoardDevice");

  mitk::ToFCameraPMDRawDataCamBoardDevice::Pointer tofCameraPMDRawDataCamBoardDevice = mitk::ToFCameraPMDRawDataCamBoardDevice::New();
  // No hardware attached for automatic testing -> test correct error handling
  MITK_TEST_CONDITION_REQUIRED(tofCameraPMDRawDataCamBoardDevice->ConnectCamera(), "Test ConnectCamera()");
  MITK_TEST_CONDITION_REQUIRED(!tofCameraPMDRawDataCamBoardDevice->IsCameraActive(), "Test IsCameraActive() before StartCamera()");
  MITK_TEST_OUTPUT(<<"Call StartCamera()");
  tofCameraPMDRawDataCamBoardDevice->StartCamera();
  MITK_TEST_CONDITION_REQUIRED(tofCameraPMDRawDataCamBoardDevice->IsCameraActive(), "Test IsCameraActive() after StartCamera()");
  MITK_TEST_OUTPUT(<<"Call UpdateCamera()");
  tofCameraPMDRawDataCamBoardDevice->UpdateCamera();
  int numberOfPixels = tofCameraPMDRawDataCamBoardDevice->GetCaptureWidth()*tofCameraPMDRawDataCamBoardDevice->GetCaptureHeight();
  MITK_INFO<<numberOfPixels;
  float* distances = new float[numberOfPixels];
  float* amplitudes = new float[numberOfPixels];
  float* intensities = new float[numberOfPixels];
  char* sourceData = new char[numberOfPixels];
  short* shortSource = new short[numberOfPixels];
  int requiredImageSequence = 0;
  int imageSequence = 0;
  vtkShortArray* channelData = vtkShortArray::New();
  tofCameraPMDRawDataCamBoardDevice->GetChannelSourceData(shortSource, channelData);
  MITK_TEST_CONDITION_REQUIRED(tofCameraPMDRawDataCamBoardDevice->GetChannelSize()!=0, "Test if channelsize is set!");
  MITK_TEST_CONDITION_REQUIRED(channelData->GetNumberOfTuples()== 4, "Check channel data processing!");
  channelData->Delete();

  tofCameraPMDRawDataCamBoardDevice->GetDistances(distances,imageSequence);
  tofCameraPMDRawDataCamBoardDevice->GetAmplitudes(amplitudes,imageSequence);
  tofCameraPMDRawDataCamBoardDevice->GetIntensities(intensities,imageSequence);
  tofCameraPMDRawDataCamBoardDevice->GetAllImages(distances,amplitudes,intensities,sourceData,requiredImageSequence,imageSequence);
  MITK_TEST_CONDITION_REQUIRED(tofCameraPMDRawDataCamBoardDevice->IsCameraActive(), "Test IsCameraActive() before StopCamera()");
  MITK_TEST_OUTPUT(<<"Call StopCamera()");
  tofCameraPMDRawDataCamBoardDevice->StopCamera();
  MITK_TEST_CONDITION_REQUIRED(!tofCameraPMDRawDataCamBoardDevice->IsCameraActive(), "Test IsCameraActive() after StopCamera()");

  MITK_TEST_CONDITION_REQUIRED(tofCameraPMDRawDataCamBoardDevice->DisconnectCamera(), "Test DisonnectCamera()");
  delete[] distances;
  delete[] amplitudes;
  delete[] intensities;
  delete[] sourceData;

  MITK_TEST_END();

}
