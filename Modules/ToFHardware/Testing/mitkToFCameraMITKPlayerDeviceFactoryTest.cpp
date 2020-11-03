/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>
#include "mitkToFCameraMITKPlayerDeviceFactory.h"
#include "mitkIToFDeviceFactory.h"

//MicroServices
#include <usModuleContext.h>
#include <usGetModuleContext.h>
#include <usServiceReference.h>

class mitkToFCameraMITKPlayerDeviceFactoryTestSuite : public mitk::TestFixture
{

  CPPUNIT_TEST_SUITE(mitkToFCameraMITKPlayerDeviceFactoryTestSuite);
  MITK_TEST(ConnectToFDevice_DeviceIsNotNull);
  MITK_TEST(CreateThreeDevices_NumberOfDevicesEqualsThree);
  MITK_TEST(GetCurrentDeviceName_NameContainsNoNumber);
  MITK_TEST(GetCurrentDeviceName_RegisterThreeDevices_NameContainsThree);
  CPPUNIT_TEST_SUITE_END();

private:

  mitk::ToFCameraMITKPlayerDeviceFactory* m_DummyPlayerDeviceFactory;

public:

  void setUp() override
  {
    us::ModuleContext* context = us::GetModuleContext();
    //get the service which is generated in the mitkToFHardwareModuleActivator
    //filter for the player factory, because other factories could be registered
    std::string filter = "(ToFFactoryName=MITK Player Factory)";
    us::ServiceReference<mitk::IToFDeviceFactory> serviceRef = context->GetServiceReferences<mitk::IToFDeviceFactory>(filter).front();
    m_DummyPlayerDeviceFactory = dynamic_cast<mitk::ToFCameraMITKPlayerDeviceFactory*>( context->GetService<mitk::IToFDeviceFactory>(serviceRef) );
  }

  void ConnectToFDevice_DeviceIsNotNull()
  {
    mitk::ToFCameraDevice::Pointer playerDevice = m_DummyPlayerDeviceFactory->ConnectToFDevice();
    CPPUNIT_ASSERT_MESSAGE("Check that a device was generated.", playerDevice.IsNotNull());
    CPPUNIT_ASSERT_MESSAGE("Check that the device name is for a player.", m_DummyPlayerDeviceFactory->GetCurrentDeviceName().find("MITK Player") == 0);
    m_DummyPlayerDeviceFactory->DisconnectToFDevice( playerDevice );
    //only the default device should be left
    CPPUNIT_ASSERT( m_DummyPlayerDeviceFactory->GetNumberOfDevices() == 1 );
  }

  void CreateThreeDevices_NumberOfDevicesEqualsThree()
  {
    //connect two devices, because there is already one default device connected.
    mitk::ToFCameraDevice::Pointer device2 = m_DummyPlayerDeviceFactory->ConnectToFDevice();
    mitk::ToFCameraDevice::Pointer device3 = m_DummyPlayerDeviceFactory->ConnectToFDevice();
    //two newly connected + one default = 3
    CPPUNIT_ASSERT( m_DummyPlayerDeviceFactory->GetNumberOfDevices() == 3 );

    m_DummyPlayerDeviceFactory->DisconnectToFDevice( device2 );
    m_DummyPlayerDeviceFactory->DisconnectToFDevice( device3 );
    //only the default device should be left
    CPPUNIT_ASSERT( m_DummyPlayerDeviceFactory->GetNumberOfDevices() == 1 );
  }

  void GetCurrentDeviceName_NameContainsNoNumber()
  {
    CPPUNIT_ASSERT( m_DummyPlayerDeviceFactory->GetCurrentDeviceName() == "MITK Player" );
  }

  void GetCurrentDeviceName_RegisterThreeDevices_NameContainsThree()
  {
    //connect two devices, because there is already one default device connected.
    mitk::ToFCameraDevice::Pointer device2 = m_DummyPlayerDeviceFactory->ConnectToFDevice();
    mitk::ToFCameraDevice::Pointer device3 = m_DummyPlayerDeviceFactory->ConnectToFDevice();
    //two newly connected + one default = 3
    CPPUNIT_ASSERT( m_DummyPlayerDeviceFactory->GetCurrentDeviceName() == "MITK Player 3" );

    m_DummyPlayerDeviceFactory->DisconnectToFDevice( device2 );
    m_DummyPlayerDeviceFactory->DisconnectToFDevice( device3 );
    CPPUNIT_ASSERT( m_DummyPlayerDeviceFactory->GetCurrentDeviceName() == "MITK Player" );
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkToFCameraMITKPlayerDeviceFactory)
