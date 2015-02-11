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

//testing headers
#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>

//headers of IGT classes releated to the tested class
#include <mitkOpenIGTLinkTrackingDevice.h>


class mitkOpenIGTLinkTrackingDeviceTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkOpenIGTLinkTrackingDeviceTestSuite);
  MITK_TEST(TestInstantiation);
  CPPUNIT_TEST_SUITE_END();

private:
  /** Members used inside the different test methods. All members are initialized via setUp().*/
  mitk::OpenIGTLinkTrackingDevice::Pointer m_OpenIGTLinkTrackingDevice;

public:

  /**@brief Setup Always call this method before each Test-case to ensure correct and new intialization of the used members for a new test case. (If the members are not used in a test, the method does not need to be called).*/
  void setUp()
  {
  }

  void tearDown()
  {
  }

  void TestInstantiation()
  {
  // let's create objects of our classes
  mitk::OpenIGTLinkTrackingDevice::Pointer testDevice = mitk::OpenIGTLinkTrackingDevice::New();
  CPPUNIT_ASSERT_MESSAGE("Testing instantiation of OpenIGTLinkTrackingDevice",testDevice.IsNotNull());
  }

};
MITK_TEST_SUITE_REGISTRATION(mitkOpenIGTLinkTrackingDevice)
