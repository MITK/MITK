/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

//testing headers
#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>

//headers of IGT classes releated to the tested class
#include <mitkOpenIGTLinkTrackingDevice.h>

//sleep headers
#include <chrono>
#include <thread>

class mitkOpenIGTLinkTrackingDeviceTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkOpenIGTLinkTrackingDeviceTestSuite);
  MITK_TEST(TestInstantiation);
  MITK_TEST(TestSetConnectionParameters);
  MITK_TEST(TestDiscoverToolMethod);
  CPPUNIT_TEST_SUITE_END();

private:
  /** Members used inside the different test methods. All members are initialized via setUp().*/
  mitk::OpenIGTLinkTrackingDevice::Pointer m_OpenIGTLinkTrackingDevice;

public:

  /**@brief Setup Always call this method before each Test-case to ensure correct and new intialization of the used members for a new test case. (If the members are not used in a test, the method does not need to be called).*/
  void setUp() override
  {
    m_OpenIGTLinkTrackingDevice = mitk::OpenIGTLinkTrackingDevice::New();
  }

  void tearDown() override
  {
  }

  void TestInstantiation()
  {
  // let's create objects of our classes
  mitk::OpenIGTLinkTrackingDevice::Pointer testDevice = mitk::OpenIGTLinkTrackingDevice::New();
  CPPUNIT_ASSERT_MESSAGE("Testing instantiation of OpenIGTLinkTrackingDevice",testDevice.IsNotNull());
  }

  void TestSetConnectionParameters()
  {
    m_OpenIGTLinkTrackingDevice->SetHostname("localhost");
    m_OpenIGTLinkTrackingDevice->SetPortNumber(10);
    CPPUNIT_ASSERT_MESSAGE("Testing method SetHostname() ...", m_OpenIGTLinkTrackingDevice->GetHostname()=="localhost");
    CPPUNIT_ASSERT_MESSAGE("Testing method SetPort() ...", m_OpenIGTLinkTrackingDevice->GetPortNumber()==10);
  }

  void TestDiscoverToolMethod()
  {
  CPPUNIT_ASSERT_MESSAGE("Testing DiscoverTools() without initialization. (Warnings are expected)", m_OpenIGTLinkTrackingDevice->DiscoverTools()==false);
  m_OpenIGTLinkTrackingDevice->SetPortNumber(10);
  CPPUNIT_ASSERT_MESSAGE("Testing DiscoverTools() with initialization, but without existing server. (Warnings are expected)", m_OpenIGTLinkTrackingDevice->DiscoverTools()==false);

  // This takes a pretty long time but it is not tested.
  /*m_OpenIGTLinkTrackingDevice->SetHostname("193.174.50.103");
  m_OpenIGTLinkTrackingDevice->SetPortNumber(18944);
  m_OpenIGTLinkTrackingDevice->DiscoverTools(20000);
  m_OpenIGTLinkTrackingDevice->OpenConnection();
  m_OpenIGTLinkTrackingDevice->StartTracking();

  std::this_thread::sleep_for(std::chrono::seconds(20));

  m_OpenIGTLinkTrackingDevice->StopTracking();
  m_OpenIGTLinkTrackingDevice->CloseConnection();*/

  }

};
MITK_TEST_SUITE_REGISTRATION(mitkOpenIGTLinkTrackingDevice)
