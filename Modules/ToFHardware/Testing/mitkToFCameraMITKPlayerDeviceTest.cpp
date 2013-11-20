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
#include <mitkTestFixture.h>
#include "mitkToFCameraMITKPlayerDevice.h"
#include "mitkIToFDeviceFactory.h"

//MicroServices
#include <usModuleContext.h>
#include <usGetModuleContext.h>
#include <usServiceReference.h>
#include <mitkIOUtil.h>

class mitkToFCameraMITKPlayerDeviceTestSuite : public mitk::TestFixture
{

  CPPUNIT_TEST_SUITE(mitkToFCameraMITKPlayerDeviceTestSuite);
  MITK_TEST(GetDistances_ValidData_ImagesEqual);
  MITK_TEST(DeviceNotConnected_NotActive);
  MITK_TEST(ConnectCamera_ValidData_ReturnsTrue);
  MITK_TEST(StartCamera_ValidData_DeviceIsConnected);
  CPPUNIT_TEST_SUITE_END();

private:

  mitk::ToFCameraMITKPlayerDevice* m_PlayerDevice;
  std::string m_PathToDepthData;

public:

  void setUp()
  {
    us::ModuleContext* context = us::GetModuleContext();

    us::ServiceReference<mitk::ToFCameraDevice> serviceRefDevice = context->GetServiceReference<mitk::ToFCameraDevice>();
    m_PlayerDevice = dynamic_cast<mitk::ToFCameraMITKPlayerDevice*>( context->GetService<mitk::ToFCameraDevice>(serviceRefDevice) );

    //during the tests here, we use one special test data set located in MITK-Data
    m_PathToDepthData = GetTestDataFilePath("ToF-Data/Kinect_Lego_Phantom_DistanceImage.nrrd");
    m_PlayerDevice->SetProperty("DistanceImageFileName",mitk::StringProperty::New(m_PathToDepthData));
  }

  void tearDown()
  {
    //Wait some time to avoid threading issues.
    itksys::SystemTools::Delay(1000);
    if(m_PlayerDevice->IsCameraActive())
      m_PlayerDevice->StopCamera();
    if(m_PlayerDevice->IsCameraConnected())
      m_PlayerDevice->DisconnectCamera();
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

      //initialize an array with the correct size
      unsigned int numberOfPixels = dimension[0]*dimension[1];
      float* distances = new float[numberOfPixels];
      int imageSequence = 0;

      //fill the array with the device output
      m_PlayerDevice->GetDistances(distances,imageSequence);

      //initialize an image and fill it with the array
      mitk::Image::Pointer resultDepthImage = mitk::Image::New();
      resultDepthImage->Initialize(mitk::PixelType(mitk::MakeScalarPixelType<float>()), 2, dimension,1);
      resultDepthImage->SetSlice(distances);

      mitk::Image::Pointer expectedDepthImage = mitk::IOUtil::LoadImage(m_PathToDepthData);

      MITK_ASSERT_EQUAL( expectedDepthImage, resultDepthImage,
        "Image from the player should be the same as loaded from the harddisk, because we just load one slice.");

//      m_PlayerDevice->StopCamera();
//      MITK_TEST_OUTPUT(<< "Device stopped");
//      CPPUNIT_ASSERT_MESSAGE("After stopping the device, the device should be inactive.", m_PlayerDevice->IsCameraActive()==false);
//      CPPUNIT_ASSERT_MESSAGE("DisconnectCamera() should return true in case of success.", m_PlayerDevice->DisconnectCamera()==true);
//      MITK_TEST_OUTPUT(<< "Device disconnected");


      delete[] distances;
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
      m_PlayerDevice->ConnectCamera();
      m_PlayerDevice->StartCamera();
      unsigned int dimension[2];
      dimension[0] = m_PlayerDevice->GetCaptureWidth();
      dimension[1] = m_PlayerDevice->GetCaptureHeight();

      //initialize an array with the correct size
      unsigned int numberOfPixels = dimension[0]*dimension[1];
      float* distances = new float[numberOfPixels];
      int imageSequence = 0;

      //fill the array with the device output
      m_PlayerDevice->GetDistances(distances,imageSequence);

      //initialize an image and fill it with the array

      mitk::Image::Pointer resultDepthImage = mitk::Image::New();
      resultDepthImage->Initialize(mitk::PixelType(mitk::MakeScalarPixelType<float>()), 2, dimension,1);
      resultDepthImage->SetSlice(distances);

      m_PlayerDevice->StopCamera();
      CPPUNIT_ASSERT_MESSAGE("After stopping the device, the device should be inactive.", m_PlayerDevice->IsCameraActive()==false);
      CPPUNIT_ASSERT_MESSAGE("DisconnectCamera() should return true in case of success.", m_PlayerDevice->DisconnectCamera()==true);

      delete[] distances;
    }
    catch(std::exception  &e)
    {
      MITK_ERROR << "Unknown exception occured: " << e.what();
    }
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkToFCameraMITKPlayerDevice)
