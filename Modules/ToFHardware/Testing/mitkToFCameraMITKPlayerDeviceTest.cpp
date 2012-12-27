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
#include <mitkToFCameraMITKPlayerDevice.h>
#include <mitkImageDataItem.h>
#include <mitkPicFileReader.h>

#include <mitkToFConfig.h>
#include <itksys/SystemTools.hxx>


bool CompareFloatArrays(float* array1, float* array2, unsigned int numberOfElements)
{
  bool arraysEqual = true;
  for (unsigned int i=0; i<numberOfElements; i++)
  {
    if (i==0)
      //MITK_INFO<<array1[i]<<", "<<array2[i];
      if (!mitk::Equal(array1[i],array2[i]))
      {
        arraysEqual = false;
      }
  }
  return arraysEqual;
}

bool CheckValidFrame(float* array, mitk::Image::Pointer image, unsigned int numberOfElements)
{
  bool validFrame = false;
  int numberOfSlices = image->GetDimension(2);
  for (int i=0; i<numberOfSlices; i++)
  {
    float* dataArray = (float*)image->GetSliceData(i,0,0)->GetData();
    if (CompareFloatArrays(dataArray,array,numberOfElements))
    {
      validFrame = true;
    }
  }
  return validFrame;
}

/**Documentation
*  test for the class "ToFCameraMITKPlayerDevice".
*/
int mitkToFCameraMITKPlayerDeviceTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("ToFCameraMITKPlayerDevice");

  std::string dirName = MITK_TOF_DATA_DIR;
  mitk::ToFCameraMITKPlayerDevice::Pointer tofCameraMITKPlayerDevice = mitk::ToFCameraMITKPlayerDevice::New();
  std::string distanceFileName = dirName + "/PMDCamCube2_MF0_IT0_20Images_DistanceImage.pic";
  tofCameraMITKPlayerDevice->SetProperty("DistanceImageFileName",mitk::StringProperty::New(distanceFileName));
  std::string amplitudeFileName = dirName + "/PMDCamCube2_MF0_IT0_20Images_AmplitudeImage.pic";
  tofCameraMITKPlayerDevice->SetProperty("AmplitudeImageFileName",mitk::StringProperty::New(amplitudeFileName));
  std::string intensityFileName = dirName + "/PMDCamCube2_MF0_IT0_20Images_IntensityImage.pic";
  tofCameraMITKPlayerDevice->SetProperty("IntensityImageFileName",mitk::StringProperty::New(intensityFileName));

  MITK_TEST_CONDITION_REQUIRED(tofCameraMITKPlayerDevice->IsCameraActive()==false,"Test IsCameraActive() before start camera");
  MITK_TEST_OUTPUT(<< "Start device");
  MITK_TEST_CONDITION_REQUIRED(tofCameraMITKPlayerDevice->ConnectCamera(),"Test ConnectCamera()");
  tofCameraMITKPlayerDevice->StartCamera();
  MITK_TEST_CONDITION_REQUIRED(tofCameraMITKPlayerDevice->IsCameraActive()==true,"Test IsCameraActive() after start camera");

  unsigned int captureWidth = tofCameraMITKPlayerDevice->GetCaptureWidth();
  unsigned int captureHeight = tofCameraMITKPlayerDevice->GetCaptureHeight();
  unsigned int numberOfPixels = captureWidth*captureHeight;
  float* distances = new float[numberOfPixels];
  float* amplitudes = new float[numberOfPixels];
  float* intensities = new float[numberOfPixels];
  unsigned char* rgbDataArray = new unsigned char[numberOfPixels*3];
  char* sourceDataArray = new char[numberOfPixels];
  float* expectedDistances = NULL;
  float* expectedAmplitudes = NULL;
  float* expectedIntensities = NULL;
  int requiredImageSequence = 0;
  int imageSequence = 0;


  mitk::PicFileReader::Pointer distanceFileReader = mitk::PicFileReader::New();
  distanceFileReader->SetFileName(distanceFileName);
  distanceFileReader->Update();
  mitk::Image::Pointer distanceImage = distanceFileReader->GetOutput();
  mitk::PicFileReader::Pointer amplitudeFileReader = mitk::PicFileReader::New();
  amplitudeFileReader->SetFileName(amplitudeFileName);
  amplitudeFileReader->Update();
  mitk::Image::Pointer amplitudeImage = amplitudeFileReader->GetOutput();
  mitk::PicFileReader::Pointer intensityFileReader = mitk::PicFileReader::New();
  intensityFileReader->SetFileName(intensityFileName);
  intensityFileReader->Update();
  mitk::Image::Pointer intensityImage = intensityFileReader->GetOutput();
  unsigned int numberOfFrames = distanceImage->GetDimension(2);
  for (int i=0; i<numberOfFrames; i++)
  {
    tofCameraMITKPlayerDevice->GetDistances(distances,imageSequence);
    MITK_TEST_CONDITION_REQUIRED(CheckValidFrame(distances,distanceImage,numberOfPixels),"Check frame from GetDistances()");
    tofCameraMITKPlayerDevice->GetAmplitudes(amplitudes,imageSequence);
    MITK_TEST_CONDITION_REQUIRED(CheckValidFrame(amplitudes,amplitudeImage,numberOfPixels),"Check frame from GetAmplitudes()");
    tofCameraMITKPlayerDevice->GetIntensities(intensities,imageSequence);
    MITK_TEST_CONDITION_REQUIRED(CheckValidFrame(intensities,intensityImage,numberOfPixels),"Check frame from GetIntensities()");
    MITK_TEST_OUTPUT(<< "GetAllImages() with rgbDataArray");
    tofCameraMITKPlayerDevice->GetAllImages(distances,amplitudes,intensities,sourceDataArray,requiredImageSequence,imageSequence,rgbDataArray);
    MITK_TEST_CONDITION_REQUIRED(CheckValidFrame(distances,distanceImage,numberOfPixels),"Check distance frame from GetAllImages()");
    MITK_TEST_CONDITION_REQUIRED(CheckValidFrame(amplitudes,amplitudeImage,numberOfPixels),"Check amplitude frame from GetAllImages()");
    MITK_TEST_CONDITION_REQUIRED(CheckValidFrame(intensities,intensityImage,numberOfPixels),"Check intensity frame from GetAllImages()");    //expectedDistances = (float*)distanceImage->GetSliceData(i,0,0)->GetData();
    MITK_TEST_OUTPUT(<< "GetAllImages() without rgbDataArray");
    tofCameraMITKPlayerDevice->GetAllImages(distances,amplitudes,intensities,sourceDataArray,requiredImageSequence,imageSequence);
  }
  itksys::SystemTools::Delay(1000);
  tofCameraMITKPlayerDevice->StopCamera();
  MITK_TEST_CONDITION_REQUIRED(tofCameraMITKPlayerDevice->IsCameraActive()==false,"Test IsCameraActive() after stop camera");
  MITK_TEST_CONDITION_REQUIRED(tofCameraMITKPlayerDevice->DisconnectCamera(),"Test DisconnectCamera()");
  delete [] distances;
  delete [] intensities;
  delete [] amplitudes;
  delete [] rgbDataArray;
  delete [] sourceDataArray;
  MITK_TEST_END();

}
