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
#include <mitkToFCameraPMDRawDataCamCubeDevice.h>

// vtk includes
#include "vtkShortArray.h"

/**Documentation
 *  test for the class "ToFCameraPMDRawDataCamCubeDevice".
 */
int mitkToFCameraPMDRawDataCamCubeDeviceTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("ToFCameraPMDRawDataCamCubeDevice");

  mitk::ToFCameraPMDRawDataCamCubeDevice::Pointer tofCameraPMDRawDataCamCubeDevice = mitk::ToFCameraPMDRawDataCamCubeDevice::New();
  // No hardware attached for automatic testing -> test correct error handling
  MITK_TEST_CONDITION_REQUIRED(tofCameraPMDRawDataCamCubeDevice->ConnectCamera(), "Test ConnectCamera()");
  MITK_TEST_CONDITION_REQUIRED(!tofCameraPMDRawDataCamCubeDevice->IsCameraActive(), "Test IsCameraActive() before StartCamera()");
  MITK_TEST_OUTPUT(<<"Call StartCamera()");
  tofCameraPMDRawDataCamCubeDevice->StartCamera();
  MITK_TEST_CONDITION_REQUIRED(tofCameraPMDRawDataCamCubeDevice->IsCameraActive(), "Test IsCameraActive() after StartCamera()");
  MITK_TEST_OUTPUT(<<"Call UpdateCamera()");
  tofCameraPMDRawDataCamCubeDevice->UpdateCamera();
  int numberOfPixels = tofCameraPMDRawDataCamCubeDevice->GetCaptureWidth()*tofCameraPMDRawDataCamCubeDevice->GetCaptureHeight();
  MITK_INFO<<numberOfPixels;
  float* distances = new float[numberOfPixels];
  float* amplitudes = new float[numberOfPixels];
  float* intensities = new float[numberOfPixels];
  char* sourceData = new char[numberOfPixels];
  short* shortSource = new short[numberOfPixels];
  int requiredImageSequence = 0;
  int imageSequence = 0;
  vtkShortArray* vtkArray = vtkShortArray::New();
  tofCameraPMDRawDataCamCubeDevice->GetChannelSourceData(shortSource, vtkArray);
  MITK_TEST_CONDITION_REQUIRED(tofCameraPMDRawDataCamCubeDevice->GetChannelSize()!=0, "Test if channelsize is set!");
  MITK_TEST_CONDITION_REQUIRED(vtkArray->GetNumberOfTuples()== 4, "Check channel data processing!");
  vtkArray->Delete();

  tofCameraPMDRawDataCamCubeDevice->GetDistances(distances,imageSequence);
  tofCameraPMDRawDataCamCubeDevice->GetAmplitudes(amplitudes,imageSequence);
  tofCameraPMDRawDataCamCubeDevice->GetIntensities(intensities,imageSequence);
  tofCameraPMDRawDataCamCubeDevice->GetAllImages(distances,amplitudes,intensities,sourceData,requiredImageSequence,imageSequence);
  MITK_TEST_CONDITION_REQUIRED(tofCameraPMDRawDataCamCubeDevice->IsCameraActive(), "Test IsCameraActive() before StopCamera()");
  MITK_TEST_OUTPUT(<<"Call StopCamera()");
  tofCameraPMDRawDataCamCubeDevice->StopCamera();
  MITK_TEST_CONDITION_REQUIRED(!tofCameraPMDRawDataCamCubeDevice->IsCameraActive(), "Test IsCameraActive() after StopCamera()");

  MITK_TEST_CONDITION_REQUIRED(tofCameraPMDRawDataCamCubeDevice->DisconnectCamera(), "Test DisonnectCamera()");
  delete[] distances;
  delete[] amplitudes;
  delete[] intensities;
  delete[] sourceData;
  delete[] shortSource;

  MITK_TEST_END();
}
