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
#include <vtkShortArray.h>
#include <vtkSmartPointer.h>

/**Documentation
 *  test for the class "ToFCameraPMDRawDataCamCubeDevice".
 */
int mitkToFCameraPMDRawDataCamCubeDeviceTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("ToFCameraPMDRawDataCamCubeDevice");

  mitk::ToFCameraPMDRawDataCamCubeDevice::Pointer rawDataCamCubeDevice = mitk::ToFCameraPMDRawDataCamCubeDevice::New();
  //try if hardware is connected
  try
  {
  // No hardware attached for automatic testing -> test correct error handling
  MITK_TEST_CONDITION_REQUIRED(rawDataCamCubeDevice->ConnectCamera(), "Test ConnectCamera()");
  MITK_TEST_CONDITION_REQUIRED(!rawDataCamCubeDevice->IsCameraActive(), "Test IsCameraActive() before StartCamera()");
  MITK_TEST_OUTPUT(<<"Call StartCamera()");
  rawDataCamCubeDevice->StartCamera();
  MITK_TEST_CONDITION_REQUIRED(rawDataCamCubeDevice->IsCameraActive(), "Test IsCameraActive() after StartCamera()");
  MITK_TEST_OUTPUT(<<"Call UpdateCamera()");
  rawDataCamCubeDevice->UpdateCamera();
  int numberOfPixels = rawDataCamCubeDevice->GetCaptureWidth()*rawDataCamCubeDevice->GetCaptureHeight();
  MITK_INFO<<numberOfPixels;
  float* distances = new float[numberOfPixels];
  float* amplitudes = new float[numberOfPixels];
  float* intensities = new float[numberOfPixels];
  char* sourceData = new char[numberOfPixels];
  short* shortSource = new short[numberOfPixels];
  int requiredImageSequence = 0;
  int imageSequence = 0;
  vtkSmartPointer<vtkShortArray> vtkArray = vtkSmartPointer<vtkShortArray>::New();
  rawDataCamCubeDevice->GetChannelSourceData(shortSource, vtkArray);
  MITK_TEST_CONDITION_REQUIRED(rawDataCamCubeDevice->GetChannelSize()!=0, "Test if channelsize is set!");
  MITK_TEST_CONDITION_REQUIRED(vtkArray->GetNumberOfTuples()== 4, "Check channel data processing!");
  vtkArray->Delete();

  rawDataCamCubeDevice->GetDistances(distances,imageSequence);
  rawDataCamCubeDevice->GetAmplitudes(amplitudes,imageSequence);
  rawDataCamCubeDevice->GetIntensities(intensities,imageSequence);
  rawDataCamCubeDevice->GetAllImages(distances,amplitudes,intensities,sourceData,requiredImageSequence,imageSequence);
  MITK_TEST_CONDITION_REQUIRED(rawDataCamCubeDevice->IsCameraActive(), "Test IsCameraActive() before StopCamera()");
  MITK_TEST_OUTPUT(<<"Call StopCamera()");
  rawDataCamCubeDevice->StopCamera();
  MITK_TEST_CONDITION_REQUIRED(!rawDataCamCubeDevice->IsCameraActive(), "Test IsCameraActive() after StopCamera()");

  MITK_TEST_CONDITION_REQUIRED(rawDataCamCubeDevice->DisconnectCamera(), "Test DisonnectCamera()");
  delete[] distances;
  delete[] amplitudes;
  delete[] intensities;
  delete[] sourceData;
  delete[] shortSource;
  }
  catch(std::exception &e)
  {
      MITK_INFO << e.what();
      MITK_TEST_CONDITION_REQUIRED(rawDataCamCubeDevice->IsCameraActive()==false, "Testing that no connection could be established.");
  }

  MITK_TEST_END();
}
