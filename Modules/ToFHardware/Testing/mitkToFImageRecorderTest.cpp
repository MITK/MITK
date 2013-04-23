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
#include <mitkToFImageRecorder.h>
#include <mitkToFCameraMITKPlayerDevice.h>
#include <mitkToFConfig.h>
#include <mitkPicFileReader.h>
#include <itksys/SystemTools.hxx>
#include <mitkImageDataItem.h>
#include <mitkItkImageFileReader.h>


/**Documentation
 *  test for the class "ToFImageRecorder".
 */

static bool CompareImages(mitk::Image::Pointer image1, mitk::Image::Pointer image2)
{
  //check if epsilon is exceeded
  unsigned int sliceDimension = image1->GetDimension(0)*image1->GetDimension(1);
  bool picturesEqual = true;

  int numOfFrames = image1->GetDimension(2);
  for (unsigned int i=0; i<numOfFrames; i++)
  {
    float* floatArray1 = (float*)image1->GetSliceData(i, 0, 0)->GetData();
    float* floatArray2 = (float*)image2->GetSliceData(i, 0, 0)->GetData();
    for(unsigned int j = 0; j < sliceDimension; j++)
    {
      if(!(mitk::Equal(floatArray1[j], floatArray2[j])))
      {
        picturesEqual = false;
      }
    }
  }

  return picturesEqual;
}

int mitkToFImageRecorderTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("ToFImageRecorder");

  mitk::ToFImageRecorder::Pointer tofImageRecorder = mitk::ToFImageRecorder::New();

  MITK_TEST_OUTPUT(<< "Test itk-Set/Get-Makros");
  std::string testFileName_Distance = "test_DistanceImage.nrrd";
  std::string testFileName_Amplitude = "test_AmplitudeImage.nrrd";
  std::string testFileName_Intensity = "test_IntensityImage.nrrd";
  std::string requiredName_Distance;
  std::string requiredName_Amplitude;
  std::string requiredName_Intensity;

  tofImageRecorder->SetDistanceImageFileName(testFileName_Distance);
  requiredName_Distance = tofImageRecorder->GetDistanceImageFileName();
  MITK_TEST_CONDITION_REQUIRED(requiredName_Distance==testFileName_Distance,"Test for distance image file name");

  tofImageRecorder->SetAmplitudeImageFileName(testFileName_Amplitude);
  requiredName_Amplitude = tofImageRecorder->GetAmplitudeImageFileName();
  MITK_TEST_CONDITION_REQUIRED(requiredName_Amplitude==testFileName_Amplitude,"Test for amplitude image file name");

  tofImageRecorder->SetIntensityImageFileName(testFileName_Intensity);
  requiredName_Intensity = tofImageRecorder->GetIntensityImageFileName();
  MITK_TEST_CONDITION_REQUIRED(requiredName_Intensity==testFileName_Intensity,"Test for intensity image file name");

  bool distanceImageSelected = false;
  bool amplitudeImageSelected = false;
  bool intensityImageSelected = false;
  bool requiredDistanceImageSelected = false;
  bool requiredAmplitudeImageSelected = false;
  bool requiredIntensityImageSelected = false;

  tofImageRecorder->SetDistanceImageSelected(distanceImageSelected);
  requiredDistanceImageSelected = tofImageRecorder->GetDistanceImageSelected();
  MITK_TEST_CONDITION_REQUIRED(distanceImageSelected==requiredDistanceImageSelected,"Test for distance selection");

  tofImageRecorder->SetAmplitudeImageSelected(amplitudeImageSelected);
  requiredAmplitudeImageSelected = tofImageRecorder->GetAmplitudeImageSelected();
  MITK_TEST_CONDITION_REQUIRED(amplitudeImageSelected==requiredAmplitudeImageSelected,"Test for amplitude selection");

  tofImageRecorder->SetIntensityImageSelected(intensityImageSelected);
  requiredIntensityImageSelected = tofImageRecorder->GetIntensityImageSelected();
  MITK_TEST_CONDITION_REQUIRED(intensityImageSelected==requiredIntensityImageSelected,"Test for intensity selection");

  int numOfFrames = 7;
  tofImageRecorder->SetNumOfFrames(numOfFrames);
  MITK_TEST_CONDITION_REQUIRED(numOfFrames==tofImageRecorder->GetNumOfFrames(),"Test for get/set number of frames");

  std::string fileFormat = ".nrrd";
  tofImageRecorder->SetFileFormat(fileFormat);
  MITK_TEST_CONDITION_REQUIRED(fileFormat==tofImageRecorder->GetFileFormat(),"Test for get/set the file format");



  MITK_TEST_OUTPUT(<< "Test other methods");

  tofImageRecorder->SetRecordMode(mitk::ToFImageRecorder::Infinite);
  MITK_TEST_CONDITION_REQUIRED(mitk::ToFImageRecorder::Infinite==tofImageRecorder->GetRecordMode(),"Test for get/set the record mode");

  mitk::ToFCameraDevice* testDevice = NULL;
  tofImageRecorder->SetCameraDevice(testDevice);
  MITK_TEST_CONDITION_REQUIRED(testDevice == tofImageRecorder->GetCameraDevice(),"Test for get/set the camera device");


  tofImageRecorder->SetToFImageType(mitk::ToFImageWriter::ToFImageType2DPlusT);
  MITK_TEST_CONDITION_REQUIRED(mitk::ToFImageWriter::ToFImageType2DPlusT==tofImageRecorder->GetToFImageType(), "Testing set/get ToFImageType");

  tofImageRecorder->SetToFImageType(mitk::ToFImageWriter::ToFImageType3D);
  MITK_TEST_CONDITION_REQUIRED(mitk::ToFImageWriter::ToFImageType3D==tofImageRecorder->GetToFImageType(), "Testing set/get ToFImageType");



  MITK_TEST_OUTPUT(<< "Test recording");

  tofImageRecorder = mitk::ToFImageRecorder::New();
  std::string dirName = MITK_TOF_DATA_DIR;
  mitk::ToFCameraMITKPlayerDevice::Pointer tofCameraMITKPlayerDevice = mitk::ToFCameraMITKPlayerDevice::New();
  tofImageRecorder->SetCameraDevice(tofCameraMITKPlayerDevice);
  MITK_TEST_CONDITION_REQUIRED(tofCameraMITKPlayerDevice == tofImageRecorder->GetCameraDevice(), "Testing set/get CameraDevice with ToFCameraPlayerDevice");

  std::string distanceFileName = dirName + "/PMDCamCube2_MF0_IT0_20Images_DistanceImage.pic";
  std::string amplitudeFileName = dirName + "/PMDCamCube2_MF0_IT0_20Images_AmplitudeImage.pic";
  std::string intensityFileName = dirName + "/PMDCamCube2_MF0_IT0_20Images_IntensityImage.pic";

  tofCameraMITKPlayerDevice->SetProperty("DistanceImageFileName",mitk::StringProperty::New(distanceFileName));
  tofCameraMITKPlayerDevice->SetProperty("AmplitudeImageFileName",mitk::StringProperty::New(amplitudeFileName));
  tofCameraMITKPlayerDevice->SetProperty("IntensityImageFileName",mitk::StringProperty::New(intensityFileName));

  MITK_TEST_OUTPUT(<< "Test ConnectCamera()");
  tofCameraMITKPlayerDevice->ConnectCamera();
  MITK_TEST_OUTPUT(<< "Test StartCamera()");
  tofCameraMITKPlayerDevice->StartCamera();

  std::string distanceTestFileName = dirName + "test_distance.nrrd";
  std::string amplitudeTestFileName = dirName + "test_amplitude.nrrd";
  std::string intensityTestFileName = dirName + "test_intensity.nrrd";

  tofImageRecorder->SetDistanceImageFileName(distanceTestFileName);
  MITK_TEST_CONDITION_REQUIRED(tofImageRecorder->GetDistanceImageFileName() == distanceTestFileName, "Testing Set/GetDistanceImageFileName()");
  tofImageRecorder->SetAmplitudeImageFileName(amplitudeTestFileName);
  MITK_TEST_CONDITION_REQUIRED(tofImageRecorder->GetAmplitudeImageFileName() == amplitudeTestFileName, "Testing Set/GetAmplitudeImageFileName()");
  tofImageRecorder->SetIntensityImageFileName(intensityTestFileName);
  MITK_TEST_CONDITION_REQUIRED(tofImageRecorder->GetIntensityImageFileName() == intensityTestFileName, "Testing Set/GetIntensityImageFileName()");
  tofImageRecorder->SetRecordMode(mitk::ToFImageRecorder::PerFrames);
  MITK_TEST_CONDITION_REQUIRED(tofImageRecorder->GetRecordMode() == mitk::ToFImageRecorder::PerFrames, "Testing Set/GetRecordMode()");
  tofImageRecorder->SetNumOfFrames(20);
  MITK_TEST_CONDITION_REQUIRED(tofImageRecorder->GetNumOfFrames() == 20, "Testing Set/GetNumOfFrames()");
  tofImageRecorder->SetFileFormat(".nrrd");
  MITK_TEST_OUTPUT(<< "Test StartRecording()");
  tofImageRecorder->StartRecording();
  tofImageRecorder->WaitForThreadBeingTerminated(); // wait to allow recording
  MITK_TEST_OUTPUT(<< "Test StopRecording()");
  tofImageRecorder->StopRecording();

  MITK_TEST_OUTPUT(<< "Test StopCamera()");
  tofCameraMITKPlayerDevice->StopCamera();
  MITK_TEST_OUTPUT(<< "Test DisconnectCamera()");
  tofCameraMITKPlayerDevice->DisconnectCamera();

  // Load images (recorded and original ones) with PicFileReader for comparison
  mitk::ItkImageFileReader::Pointer nrrdReader = mitk::ItkImageFileReader::New();
  mitk::PicFileReader::Pointer picFileReader = mitk::PicFileReader::New();
  mitk::Image::Pointer originalImage = NULL;
  mitk::Image::Pointer recordedImage = NULL;

  MITK_TEST_OUTPUT(<< "Read original distance image using PicFileReader");
  picFileReader->SetFileName(distanceFileName);
  picFileReader->Update();
  originalImage = picFileReader->GetOutput()->Clone();

  MITK_TEST_OUTPUT(<< "Read recorded distance image using ItkImageFileReader");
  nrrdReader->SetFileName(distanceTestFileName);
  nrrdReader->Update();
  recordedImage = nrrdReader->GetOutput()->Clone();

  MITK_TEST_CONDITION_REQUIRED(originalImage->GetDimension(0) == tofImageRecorder->GetToFCaptureWidth(), "Testing capture width");
  MITK_TEST_CONDITION_REQUIRED(originalImage->GetDimension(1) == tofImageRecorder->GetToFCaptureHeight(), "Testing capture height");
  int numFramesOrig = originalImage->GetDimension(2);
  int numFramesRec = recordedImage->GetDimension(2);
  MITK_TEST_CONDITION_REQUIRED(originalImage->GetDimension(2) == recordedImage->GetDimension(2), "Testing number of frames");

  MITK_TEST_CONDITION_REQUIRED(CompareImages(originalImage, recordedImage), "Compare original and saved distance image");


  MITK_TEST_OUTPUT(<< "Read original amplitude image using PicFileReader");
  picFileReader->SetFileName(amplitudeFileName);
  picFileReader->Update();
  originalImage = picFileReader->GetOutput()->Clone();

  MITK_TEST_OUTPUT(<< "Read recorded amplitude image using ItkImageFileReader");
  nrrdReader->SetFileName(amplitudeTestFileName);
  nrrdReader->Update();
  recordedImage = nrrdReader->GetOutput()->Clone();

  MITK_TEST_CONDITION_REQUIRED(originalImage->GetDimension(0) == tofImageRecorder->GetToFCaptureWidth(), "Testing capture width");
  MITK_TEST_CONDITION_REQUIRED(originalImage->GetDimension(1) == tofImageRecorder->GetToFCaptureHeight(), "Testing capture height");
  MITK_TEST_CONDITION_REQUIRED(originalImage->GetDimension(2) == recordedImage->GetDimension(2), "Testing number of frames");

  MITK_TEST_CONDITION_REQUIRED(CompareImages(originalImage, recordedImage), "Compare original and saved amplitude image");


  MITK_TEST_OUTPUT(<< "Read original intensity image using PicFileReader");
  picFileReader->SetFileName(intensityFileName);
  picFileReader->Update();
  originalImage = picFileReader->GetOutput()->Clone();

  MITK_TEST_OUTPUT(<< "Read recorded intensity image using ItkImageFileReader");
  nrrdReader->SetFileName(intensityTestFileName);
  nrrdReader->Update();
  recordedImage = nrrdReader->GetOutput()->Clone();

  MITK_TEST_CONDITION_REQUIRED(originalImage->GetDimension(0) == tofImageRecorder->GetToFCaptureWidth(), "Testing capture width");
  MITK_TEST_CONDITION_REQUIRED(originalImage->GetDimension(1) == tofImageRecorder->GetToFCaptureHeight(), "Testing capture height");
  MITK_TEST_CONDITION_REQUIRED(originalImage->GetDimension(2) == recordedImage->GetDimension(2), "Testing number of frames");

  MITK_TEST_CONDITION_REQUIRED(CompareImages(originalImage, recordedImage), "Compare original and saved intensity image");

  //clean up and delete saved image files
  if( remove( distanceTestFileName.c_str() ) != 0 )
  {
    MITK_ERROR<<"File: test_distance.nrrd not successfully deleted!";
  }
  if( remove( amplitudeTestFileName.c_str() ) != 0 )
  {
    MITK_ERROR<<"File: test_amplitude.nrrd not successfully deleted!";
  }
  if( remove( intensityTestFileName.c_str() ) != 0 )
  {
    MITK_ERROR<<"File: test_intensity.nrrd not successfully deleted!";
  }

  MITK_TEST_END();
}


