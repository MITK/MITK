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

#include <mitkImage.h>
#include <mitkTestingMacros.h>
#include <mitkImageSliceSelector.h>
#include <mitkTestFixture.h>
#include <mitkToFImageRecorder.h>
#include <mitkToFCameraMITKPlayerDevice.h>
#include <mitkToFConfig.h>
#include <mitkIOUtil.h>

class mitkToFImageRecorderTestSuite : public mitk::TestFixture
{

  CPPUNIT_TEST_SUITE(mitkToFImageRecorderTestSuite);
  //Removed due to random fails on linux continuous bug #12977
  MITK_TEST(StartRecording_ValidDepthImage_WritesImageToFile);
  MITK_TEST(StartRecording_ValidAmplitudeImage_WritesImageToFile);
  MITK_TEST(StartRecording_ValidIntensityImage_WritesImageToFile);
  CPPUNIT_TEST_SUITE_END();

private:

  /** Members used inside the different (sub-)tests. All members are initialized via setUp().
    * Every recorder needs a device. Here we use a player device.
    * The player device needs data to load. The ground truth is loaded via IOUtil.
    */
  mitk::ToFImageRecorder::Pointer m_ToFImageRecorder;
  mitk::ToFCameraMITKPlayerDevice::Pointer m_PlayerDevice;
  std::string m_DistanceImageName;
  std::string m_AmplitudeImageName;
  std::string m_IntensityImageName;

  mitk::Image::Pointer m_GroundTruthDepthImage;
  mitk::Image::Pointer m_GroundTruthIntensityImage;
  mitk::Image::Pointer m_GroundTruthAmplitudeImage;

public:

  /**
    * @brief Setup a recorder including a device. Here, we use a player, because in an automatic test
    * hardware is not useful.
    */
  void setUp()
  {
    m_ToFImageRecorder = mitk::ToFImageRecorder::New();
    m_DistanceImageName = "test_DistanceImage.nrrd";
    m_AmplitudeImageName = "test_AmplitudeImage.nrrd";
    m_IntensityImageName = "test_IntensityImage.nrrd";


    m_PlayerDevice = mitk::ToFCameraMITKPlayerDevice::New();
    m_ToFImageRecorder->SetCameraDevice(m_PlayerDevice);

    //the test data set has 20 frames, so we record 20 per default
    m_ToFImageRecorder->SetNumOfFrames(20);
    m_ToFImageRecorder->SetRecordMode(mitk::ToFImageRecorder::PerFrames);

    std::string dirName = MITK_TOF_DATA_DIR;
    std::string distanceFileName = dirName + "/PMDCamCube2_MF0_IT0_20Images_DistanceImage.pic";
    std::string amplitudeFileName = dirName + "/PMDCamCube2_MF0_IT0_20Images_AmplitudeImage.pic";
    std::string intensityFileName = dirName + "/PMDCamCube2_MF0_IT0_20Images_IntensityImage.pic";

    m_PlayerDevice->SetProperty("DistanceImageFileName",mitk::StringProperty::New(distanceFileName));
    m_PlayerDevice->SetProperty("AmplitudeImageFileName",mitk::StringProperty::New(amplitudeFileName));
    m_PlayerDevice->SetProperty("IntensityImageFileName",mitk::StringProperty::New(intensityFileName));

    //comparing against IOUtil seems fair enough
    m_GroundTruthDepthImage = mitk::IOUtil::LoadImage(distanceFileName);
    m_GroundTruthAmplitudeImage = mitk::IOUtil::LoadImage(amplitudeFileName);
    m_GroundTruthIntensityImage = mitk::IOUtil::LoadImage(intensityFileName);

    m_PlayerDevice->ConnectCamera();
    m_PlayerDevice->StartCamera();

  }

  void tearDown()
  {
    m_PlayerDevice->StopCamera();
    m_PlayerDevice->DisconnectCamera();
  }

  void StartRecording_ValidDepthImage_WritesImageToFile()
  {
    m_ToFImageRecorder->SetDistanceImageFileName(m_DistanceImageName);
    m_ToFImageRecorder->StartRecording();
    m_ToFImageRecorder->WaitForThreadBeingTerminated(); // wait to allow recording
    m_ToFImageRecorder->StopRecording();

    mitk::Image::Pointer recordedImage = mitk::IOUtil::LoadImage(m_DistanceImageName);
    MITK_ASSERT_EQUAL( m_GroundTruthDepthImage, recordedImage, "Recorded image should be equal to the test data.");

    if( remove( m_DistanceImageName.c_str() ) != 0 )
    {
      MITK_ERROR<<"File: test_distance.nrrd not successfully deleted!";
    }
  }

  void StartRecording_ValidAmplitudeImage_WritesImageToFile()
  {
    m_ToFImageRecorder->SetAmplitudeImageFileName(m_AmplitudeImageName);
    m_ToFImageRecorder->SetAmplitudeImageSelected(true);
    m_ToFImageRecorder->SetDistanceImageSelected(false);
    m_ToFImageRecorder->StartRecording();
    m_ToFImageRecorder->WaitForThreadBeingTerminated(); // wait to allow recording
    m_ToFImageRecorder->StopRecording();

    mitk::Image::Pointer recordedImage = mitk::IOUtil::LoadImage(m_AmplitudeImageName);
    MITK_ASSERT_EQUAL( m_GroundTruthAmplitudeImage, recordedImage, "Recorded image should be equal to the test data.");

    if( remove( m_AmplitudeImageName.c_str() ) != 0 )
    {
      MITK_ERROR<<"File: test_amplitude.nrrd not successfully deleted!";
    }
  }

  void StartRecording_ValidIntensityImage_WritesImageToFile()
  {
    m_ToFImageRecorder->SetIntensityImageFileName(m_IntensityImageName);
    m_ToFImageRecorder->SetIntensityImageSelected(true);
    m_ToFImageRecorder->SetDistanceImageSelected(false);
    m_ToFImageRecorder->StartRecording();
    m_ToFImageRecorder->WaitForThreadBeingTerminated(); // wait to allow recording
    m_ToFImageRecorder->StopRecording();

    mitk::Image::Pointer recordedImage = mitk::IOUtil::LoadImage(m_IntensityImageName);
    MITK_ASSERT_EQUAL( m_GroundTruthIntensityImage, recordedImage, "Recorded image should be equal to the test data.");

    if( remove( m_IntensityImageName.c_str() ) != 0 )
    {
      MITK_ERROR<<"File: test_intensity.nrrd not successfully deleted!";
    }
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkToFImageRecorder)
