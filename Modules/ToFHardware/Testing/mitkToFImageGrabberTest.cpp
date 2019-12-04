/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkTestingMacros.h>
#include <mitkTestingConfig.h>
#include <mitkTestFixture.h>
#include <mitkIOUtil.h>

#include <mitkToFImageGrabber.h>
#include <mitkToFCameraMITKPlayerDevice.h>
#include <mitkToFConfig.h>


#include <mitkImageSliceSelector.h>

/**
 * @brief The mitkToFImageGrabberTestSuite class is a test-suite for mitkToFImageGrabber.
 *
 * Test data is retrieved from MITK-Data.
 */
class mitkToFImageGrabberTestSuite : public mitk::TestFixture
{

  CPPUNIT_TEST_SUITE(mitkToFImageGrabberTestSuite);
  MITK_TEST(GetterAndSetterTests);
  MITK_TEST(ConnectCamera_InvalidFileName_ThrowsException);
  MITK_TEST(ConnectCamera_ValidFileName_ReturnsTrue);
  MITK_TEST(IsCameraActive_DifferentStates_ReturnsCorrectResult);
  MITK_TEST(Update_2DData_ImagesAreEqual);
  MITK_TEST(Update_CamCubeData_PropertiesAreTrue);

  CPPUNIT_TEST_SUITE_END();

private:

  std::string m_KinectDepthImagePath;
  std::string m_KinectColorImagePath;

  std::string m_CamCubeDepthImagePath;
  std::string m_CamCubeIntensityImagePath;
  std::string m_CamCubeAmplitudeImagePath;

  mitk::ToFImageGrabber::Pointer m_ToFImageGrabber;

public:

  void setUp() override
  {
    std::string dirName = MITK_TOF_DATA_DIR;

    std::string depthImagePath = dirName + "/" + "Kinect_Lego_Phantom_DistanceImage.nrrd";
    std::string colorImagePath = dirName + "/" + "Kinect_Lego_Phantom_RGBImage.nrrd";
    std::string distanceImageP = dirName + "/" + "PMDCamCube2_MF0_IT0_1Images_DistanceImage.pic";
    std::string amplitudeImage = dirName + "/" + "PMDCamCube2_MF0_IT0_1Images_AmplitudeImage.pic";
    std::string intensityImage = dirName + "/" + "PMDCamCube2_MF0_IT0_1Images_IntensityImage.pic";

    m_KinectDepthImagePath = GetTestDataFilePath(depthImagePath);
    m_KinectColorImagePath = GetTestDataFilePath(colorImagePath);
    m_CamCubeDepthImagePath = GetTestDataFilePath(distanceImageP);
    m_CamCubeIntensityImagePath = GetTestDataFilePath(amplitudeImage);
    m_CamCubeAmplitudeImagePath = GetTestDataFilePath(intensityImage);

    m_ToFImageGrabber = mitk::ToFImageGrabber::New();

    //The Grabber always needs a device
    mitk::ToFCameraMITKPlayerDevice::Pointer tofCameraMITKPlayerDevice = mitk::ToFCameraMITKPlayerDevice::New();
    m_ToFImageGrabber->SetCameraDevice(tofCameraMITKPlayerDevice);
  }

  void tearDown() override
  {
    if(m_ToFImageGrabber->IsCameraActive())
    {
      m_ToFImageGrabber->StopCamera();
      m_ToFImageGrabber->DisconnectCamera();
    }
    m_ToFImageGrabber = nullptr;
  }

  /**
   * @brief GetterAndSetterTests Testing getters and setters of mitkToFImageGrabber, because
   * they internally contain logic.
   */
  void GetterAndSetterTests()
  {
    int modulationFrequency = 20;
    m_ToFImageGrabber->SetModulationFrequency(modulationFrequency);
    CPPUNIT_ASSERT(modulationFrequency==m_ToFImageGrabber->GetModulationFrequency());

    int integrationTime = 500;
    m_ToFImageGrabber->SetIntegrationTime(integrationTime);
    CPPUNIT_ASSERT(integrationTime==m_ToFImageGrabber->GetIntegrationTime());
  }

  void ConnectCamera_InvalidFileName_ThrowsException()
  {
    MITK_TEST_FOR_EXCEPTION(std::logic_error, m_ToFImageGrabber->ConnectCamera());
    m_ToFImageGrabber->StartCamera();
    m_ToFImageGrabber->StopCamera();
    CPPUNIT_ASSERT_MESSAGE("Test DisconnectCamera() with no file name set", !m_ToFImageGrabber->DisconnectCamera());
  }

  void ConnectCamera_ValidFileName_ReturnsTrue()
  {
    m_ToFImageGrabber->SetProperty("DistanceImageFileName",mitk::StringProperty::New(m_KinectDepthImagePath));
    m_ToFImageGrabber->SetProperty("RGBImageFileName",mitk::StringProperty::New(m_KinectColorImagePath));


    CPPUNIT_ASSERT(m_ToFImageGrabber->ConnectCamera() == true);
  }

  void Update_2DData_ImagesAreEqual()
  {
    try
    {
      m_ToFImageGrabber->SetProperty("DistanceImageFileName",mitk::StringProperty::New(m_KinectDepthImagePath));

      m_ToFImageGrabber->ConnectCamera();
      m_ToFImageGrabber->StartCamera();
      mitk::Image::Pointer expectedResultImage = mitk::IOUtil::Load<mitk::Image>(m_KinectDepthImagePath);

      m_ToFImageGrabber->Update();

      //Select 2D slice to make it comparable in diminsion
      mitk::ImageSliceSelector::Pointer selector = mitk::ImageSliceSelector::New();
      selector->SetSliceNr(0);
      selector->SetTimeNr(0);
      selector->SetInput( m_ToFImageGrabber->GetOutput(0) );
      selector->Update();
      mitk::Image::Pointer distanceImage = selector->GetOutput(0);

      MITK_ASSERT_EQUAL( expectedResultImage, distanceImage, "Test data is 2D. Results should be equal.");
    }
    catch(std::exception &e)
    {
      MITK_ERROR << e.what();
    }
  }

  void IsCameraActive_DifferentStates_ReturnsCorrectResult()
  {
    m_ToFImageGrabber->SetProperty("DistanceImageFileName",mitk::StringProperty::New(m_KinectDepthImagePath));
    m_ToFImageGrabber->SetProperty("RGBImageFileName",mitk::StringProperty::New(m_KinectColorImagePath));

    m_ToFImageGrabber->ConnectCamera();
    CPPUNIT_ASSERT(m_ToFImageGrabber->IsCameraActive() == false);
    m_ToFImageGrabber->StartCamera();
    CPPUNIT_ASSERT(m_ToFImageGrabber->IsCameraActive() == true);
    m_ToFImageGrabber->StopCamera();
    CPPUNIT_ASSERT(m_ToFImageGrabber->IsCameraActive() == false);
    m_ToFImageGrabber->StartCamera();
    CPPUNIT_ASSERT(m_ToFImageGrabber->IsCameraActive() == true);
    m_ToFImageGrabber->StopCamera();
    CPPUNIT_ASSERT(m_ToFImageGrabber->IsCameraActive() == false);

    m_ToFImageGrabber->DisconnectCamera();
    CPPUNIT_ASSERT(m_ToFImageGrabber->IsCameraActive() == false);
  }

  void Update_CamCubeData_PropertiesAreTrue()
  {
    m_ToFImageGrabber->SetProperty("DistanceImageFileName",mitk::StringProperty::New(m_CamCubeDepthImagePath));
    m_ToFImageGrabber->SetProperty("AmplitudeImageFileName",mitk::StringProperty::New(m_CamCubeAmplitudeImagePath));
    m_ToFImageGrabber->SetProperty("IntensityImageFileName",mitk::StringProperty::New(m_CamCubeIntensityImagePath));

    m_ToFImageGrabber->ConnectCamera();
    m_ToFImageGrabber->StartCamera();
    m_ToFImageGrabber->Update();

    CPPUNIT_ASSERT( m_ToFImageGrabber->GetBoolProperty("HasAmplitudeImage") == true);
    CPPUNIT_ASSERT( m_ToFImageGrabber->GetBoolProperty("HasIntensityImage") == true);
    CPPUNIT_ASSERT( m_ToFImageGrabber->GetOutput(1) != nullptr );
    CPPUNIT_ASSERT( m_ToFImageGrabber->GetOutput(2) != nullptr );
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkToFImageGrabber)
