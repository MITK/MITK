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
  MITK_TEST(Update_ValidData_ImagesAreEqual);
  CPPUNIT_TEST_SUITE_END();

private:

  std::string m_DepthImagePath;
  std::string m_ColorImagePath;

  mitk::ToFImageGrabber::Pointer m_ToFImageGrabber;

public:

  void setUp()
  {
    std::string dirName = MITK_TOF_DATA_DIR;

    std::string depthImagePath = dirName + "/" + "Kinect_Lego_Phantom_DistanceImage.nrrd";
    std::string colorImagePath = dirName + "/" + "Kinect_Lego_Phantom_RGBImage.nrrd";

    m_DepthImagePath = GetTestDataFilePath(depthImagePath);
    m_ColorImagePath = GetTestDataFilePath(colorImagePath);

    m_ToFImageGrabber = mitk::ToFImageGrabber::New();

    //The Grabber always needs a device
    mitk::ToFCameraMITKPlayerDevice::Pointer tofCameraMITKPlayerDevice = mitk::ToFCameraMITKPlayerDevice::New();
    m_ToFImageGrabber->SetCameraDevice(tofCameraMITKPlayerDevice);
  }

  void tearDown()
  {
    if(m_ToFImageGrabber->IsCameraActive())
    {
      m_ToFImageGrabber->StopCamera();
      m_ToFImageGrabber->DisconnectCamera();
    }
    m_ToFImageGrabber = NULL;
    MITK_INFO << "NED";
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
    m_ToFImageGrabber->SetProperty("DistanceImageFileName",mitk::StringProperty::New(m_DepthImagePath));
    m_ToFImageGrabber->SetProperty("RGBImageFileName",mitk::StringProperty::New(m_ColorImagePath));


    CPPUNIT_ASSERT(m_ToFImageGrabber->ConnectCamera() == true);
  }

  void Update_ValidData_ImagesAreEqual()
  {
    m_ToFImageGrabber->SetProperty("DistanceImageFileName",mitk::StringProperty::New(m_DepthImagePath));

    m_ToFImageGrabber->ConnectCamera();
    m_ToFImageGrabber->StartCamera();
    mitk::Image::Pointer expectedResultImage = mitk::IOUtil::LoadImage(m_DepthImagePath);

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

  void IsCameraActive_DifferentStates_ReturnsCorrectResult()
  {
    m_ToFImageGrabber->SetProperty("DistanceImageFileName",mitk::StringProperty::New(m_DepthImagePath));
    m_ToFImageGrabber->SetProperty("RGBImageFileName",mitk::StringProperty::New(m_ColorImagePath));

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
};

MITK_TEST_SUITE_REGISTRATION(mitkToFImageGrabber)
