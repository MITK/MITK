/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>
#include <mitkIOUtil.h>
#include "mitkToFCameraMITKPlayerDevice.h"
#include "mitkIToFDeviceFactory.h"

//MicroServices
#include <usModuleContext.h>
#include <usGetModuleContext.h>
#include <usServiceReference.h>

class mitkToFCameraMITKPlayerDeviceTestSuite : public mitk::TestFixture
{

  CPPUNIT_TEST_SUITE(mitkToFCameraMITKPlayerDeviceTestSuite);
  MITK_TEST(DeviceNotConnected_NotActive);
  MITK_TEST(ConnectCamera_ValidData_ReturnsTrue);
  MITK_TEST(GetDistances_ValidData_ImagesEqual);
  MITK_TEST(StartCamera_ValidData_DeviceIsConnected);
  MITK_TEST(DisconnectCamera_ValidData_ReturnsTrue);
  CPPUNIT_TEST_SUITE_END();

private:

  mitk::ToFCameraMITKPlayerDevice* m_PlayerDevice;
  std::string m_PathToDepthData;
  float* m_DistanceArray;

public:

  void setUp() override
  {
    us::ModuleContext* context = us::GetModuleContext();

    //Filter all registered devices for an ToFCameraMITKPlayerDevice via device name
    std::string filter = "(ToFDeviceName=MITK Player)";
    us::ServiceReference<mitk::ToFCameraDevice> serviceRefDevice = context->GetServiceReferences<mitk::ToFCameraDevice>(filter).front();
    //Get the actual device
    m_PlayerDevice = dynamic_cast<mitk::ToFCameraMITKPlayerDevice*>( context->GetService<mitk::ToFCameraDevice>(serviceRefDevice) );

    //during the tests here, we use one special test data set located in MITK-Data
    m_PathToDepthData = GetTestDataFilePath("ToF-Data/Kinect_Lego_Phantom_DistanceImage.nrrd");
    m_PlayerDevice->SetProperty("DistanceImageFileName",mitk::StringProperty::New(m_PathToDepthData));

    //initialize an array with the test data size
    unsigned int numberOfPixels = 640*480;
    m_DistanceArray = new float[numberOfPixels];
  }

  void tearDown() override
  {
    //Wait some time to avoid threading issues.
    itksys::SystemTools::Delay(10);
    //Clean up
    if(m_PlayerDevice->IsCameraActive())
      m_PlayerDevice->StopCamera();
    if(m_PlayerDevice->IsCameraConnected())
      m_PlayerDevice->DisconnectCamera();

    delete[] m_DistanceArray;
  }

  void DeviceNotConnected_NotActive()
  {
    CPPUNIT_ASSERT_MESSAGE("The device (player) should not be active before starting.", m_PlayerDevice->IsCameraActive()==false);
  }

  void ConnectCamera_ValidData_ReturnsTrue()
  {
    CPPUNIT_ASSERT_MESSAGE("ConnectCamera() should return true in case of success.", m_PlayerDevice->ConnectCamera()==true);
  }

  void StartCamera_ValidData_DeviceIsConnected()
  {
    m_PlayerDevice->ConnectCamera();
    m_PlayerDevice->StartCamera();
    CPPUNIT_ASSERT_MESSAGE("After starting the device, the device should be active.", m_PlayerDevice->IsCameraActive()==true);
  }

  void GetDistances_ValidData_ImagesEqual()
  {
    try
    {
      m_PlayerDevice->ConnectCamera();
      m_PlayerDevice->StartCamera();
      unsigned int dimension[2];
      dimension[0] = m_PlayerDevice->GetCaptureWidth();
      dimension[1] = m_PlayerDevice->GetCaptureHeight();
      int imageSequence = 0;

      //fill the array with the device output
      m_PlayerDevice->GetDistances(m_DistanceArray,imageSequence);

      //initialize an image and fill it with the array
      mitk::Image::Pointer resultDepthImage = mitk::Image::New();
      resultDepthImage->Initialize(mitk::PixelType(mitk::MakeScalarPixelType<float>()), 2, dimension,1);
      resultDepthImage->SetSlice(m_DistanceArray);

      mitk::Image::Pointer expectedDepthImage = mitk::IOUtil::Load<mitk::Image>(m_PathToDepthData);

      MITK_ASSERT_EQUAL( expectedDepthImage, resultDepthImage,
        "Image from the player should be the same as loaded from the harddisk, because we just load one slice.");
    }
    catch(std::exception  &e)
    {
      MITK_ERROR << "Unknown exception occured: " << e.what();
    }
  }

  void DisconnectCamera_ValidData_ReturnsTrue()
  {
    try
    {
      int imageSequence = 0;
      m_PlayerDevice->ConnectCamera();
      m_PlayerDevice->StartCamera();
      m_PlayerDevice->GetDistances(m_DistanceArray,imageSequence);
      m_PlayerDevice->StopCamera();
      CPPUNIT_ASSERT_MESSAGE("After stopping the device, the device should be inactive.", m_PlayerDevice->IsCameraActive()==false);
      CPPUNIT_ASSERT_MESSAGE("DisconnectCamera() should return true in case of success.", m_PlayerDevice->DisconnectCamera()==true);
    }
    catch(std::exception  &e)
    {
      MITK_ERROR << "Unknown exception occured: " << e.what();
    }
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkToFCameraMITKPlayerDevice)
