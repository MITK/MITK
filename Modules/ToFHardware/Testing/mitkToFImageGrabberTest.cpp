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
#include <mitkToFImageGrabber.h>

#include <mitkImageDataItem.h>
#include <mitkPicFileReader.h>
#include <mitkToFCameraMITKPlayerDevice.h>
#include <mitkToFConfig.h>

static bool CompareImages(mitk::Image::Pointer image1, mitk::Image::Pointer image2)
{
  //check if epsilon is exceeded
  unsigned int sliceDimension = image1->GetDimension(0)*image1->GetDimension(1);
  bool picturesEqual = true;

  float* floatArray1 = (float*)image1->GetSliceData(0, 0, 0)->GetData();
  float* floatArray2 = (float*)image2->GetSliceData(0, 0, 0)->GetData();
  for(unsigned int i = 0; i < sliceDimension; i++)
  {
    if(!(mitk::Equal(floatArray1[i], floatArray2[i])))
    {
      picturesEqual = false;
    }
  }
  return picturesEqual;
}

/**Documentation
 *  test for the class "ToFImageGrabber".
 */
int mitkToFImageGrabberTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("ToFImageGrabber");

  std::string dirName = MITK_TOF_DATA_DIR;
  mitk::ToFImageGrabber::Pointer tofImageGrabber = mitk::ToFImageGrabber::New();
  mitk::ToFCameraMITKPlayerDevice::Pointer tofCameraMITKPlayerDevice = mitk::ToFCameraMITKPlayerDevice::New();
  tofImageGrabber->SetCameraDevice(tofCameraMITKPlayerDevice);
  MITK_TEST_CONDITION_REQUIRED(tofCameraMITKPlayerDevice==tofImageGrabber->GetCameraDevice(),"Test Set/GetCameraDevice()");
  int modulationFrequency = 20;
  tofImageGrabber->SetModulationFrequency(modulationFrequency);
  MITK_TEST_CONDITION_REQUIRED(modulationFrequency==tofImageGrabber->GetModulationFrequency(),"Test Set/GetModulationFrequency()");
  int integrationTime = 500;
  tofImageGrabber->SetIntegrationTime(integrationTime);
  MITK_TEST_CONDITION_REQUIRED(integrationTime==tofImageGrabber->GetIntegrationTime(),"Test Set/GetIntegrationTime()");
  MITK_TEST_OUTPUT(<<"Test methods with invalid file name");
  MITK_TEST_FOR_EXCEPTION(std::logic_error, !tofImageGrabber->ConnectCamera());
  MITK_TEST_OUTPUT(<<"Call StartCamera()");
  tofImageGrabber->StartCamera();
  MITK_TEST_OUTPUT(<<"Call StopCamera()");
  tofImageGrabber->StopCamera();
  MITK_TEST_CONDITION_REQUIRED(!tofImageGrabber->DisconnectCamera(),"Test DisconnectCamera() with no file name set");

  std::string distanceFileName = dirName + "/PMDCamCube2_MF0_IT0_1Images_DistanceImage.pic";
  tofImageGrabber->SetProperty("DistanceImageFileName",mitk::StringProperty::New(distanceFileName));
  std::string amplitudeFileName = dirName + "/PMDCamCube2_MF0_IT0_1Images_AmplitudeImage.pic";
  tofImageGrabber->SetProperty("AmplitudeImageFileName",mitk::StringProperty::New(amplitudeFileName));
  std::string intensityFileName = dirName + "/PMDCamCube2_MF0_IT0_1Images_IntensityImage.pic";
  tofImageGrabber->SetProperty("IntensityImageFileName",mitk::StringProperty::New(intensityFileName));

  // Load images with PicFileReader for comparison
  mitk::PicFileReader::Pointer picFileReader = mitk::PicFileReader::New();
  mitk::Image::Pointer expectedResultImage = NULL;

  MITK_TEST_OUTPUT(<<"Test ToFImageGrabber using ToFCameraMITKPlayerDevice");
  MITK_TEST_CONDITION_REQUIRED(tofImageGrabber->ConnectCamera(),"Test ConnectCamera()");
  MITK_TEST_CONDITION_REQUIRED(!tofImageGrabber->IsCameraActive(),"IsCameraActive() before StartCamera()");
  MITK_TEST_OUTPUT(<<"Call StartCamera()");
  tofImageGrabber->StartCamera();
  MITK_TEST_CONDITION_REQUIRED(tofImageGrabber->IsCameraActive(),"IsCameraActive() after StartCamera()");
  picFileReader->SetFileName(distanceFileName);
  picFileReader->Update();
  expectedResultImage = picFileReader->GetOutput();
  int captureWidth = expectedResultImage->GetDimension(0);
  int captureHeight = expectedResultImage->GetDimension(1);
  MITK_TEST_CONDITION_REQUIRED(tofImageGrabber->GetCaptureWidth()==captureWidth,"Test GetCaptureWidth()");
  MITK_TEST_CONDITION_REQUIRED(tofImageGrabber->GetCaptureHeight()==captureHeight,"Test GetCaptureHeight()");
  MITK_TEST_OUTPUT(<<"Call Update()");
  tofImageGrabber->Update();
  mitk::Image::Pointer distanceImage = tofImageGrabber->GetOutput(0);
  MITK_TEST_CONDITION_REQUIRED(CompareImages(expectedResultImage,distanceImage),"Test GetOutput(0)");
  picFileReader->SetFileName(amplitudeFileName);
  picFileReader->Update();
  mitk::Image::Pointer amplitudeImage = tofImageGrabber->GetOutput(1);
  MITK_TEST_CONDITION_REQUIRED(CompareImages(expectedResultImage,amplitudeImage),"Test GetOutput(1)");
  picFileReader->SetFileName(intensityFileName);
  picFileReader->Update();
  mitk::Image::Pointer intensityImage = tofImageGrabber->GetOutput(2);
  MITK_TEST_CONDITION_REQUIRED(CompareImages(expectedResultImage,intensityImage),"Test GetOutput(2)");
  MITK_TEST_OUTPUT(<<"Call StopCamera()");
  tofImageGrabber->StopCamera();
  MITK_TEST_CONDITION_REQUIRED(!tofImageGrabber->IsCameraActive(),"IsCameraActive() after StopCamera()");
  MITK_TEST_CONDITION_REQUIRED(tofImageGrabber->DisconnectCamera(),"Test DisconnectCamera()");
  MITK_TEST_CONDITION_REQUIRED(!tofImageGrabber->IsCameraActive(),"IsCameraActive() after DisconnectCamera()");

  MITK_TEST_END();;
}


