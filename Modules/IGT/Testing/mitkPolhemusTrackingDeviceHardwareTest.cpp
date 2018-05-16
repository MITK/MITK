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

#include "mitkPolhemusInterface.h"
#include "mitkTestingMacros.h"
#include "mitkStandardFileLocations.h"
#include <mitkIGTConfig.h>

// Testing
#include "mitkTestingMacros.h"
#include "mitkTestFixture.h"

class mitkPolhemusTrackingDeviceHardwareTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPolhemusTrackingDeviceHardwareTestSuite);
  // Test the append method
  MITK_TEST(testInterface);
  CPPUNIT_TEST_SUITE_END();


public:
  void setUp() override
  {

  }
  void tearDown() override
  {

  }

  void testInterface()
  {
    mitk::PolhemusInterface::Pointer myInterface = mitk::PolhemusInterface::New();
    CPPUNIT_ASSERT_MESSAGE("Testing connection.", myInterface->Connect());
    CPPUNIT_ASSERT_MESSAGE("Start tracking.", myInterface->StartTracking());

    CPPUNIT_ASSERT_MESSAGE("Tracking 20 frames ...", true);
    for (int i = 0; i < 20; i++)
    {
      std::vector<mitk::PolhemusInterface::trackingData> lastFrame = myInterface->GetLastFrame();
      MITK_INFO << "Frame " << i;
      for (size_t j = 0; j < lastFrame.size(); j++)
      {
        MITK_INFO << "[" << j << "]" << " Pos:" << lastFrame.at(j).pos << " Rot:" << lastFrame.at(j).rot;
      }
    }

  }

};

MITK_TEST_SUITE_REGISTRATION(mitkPolhemusTrackingDeviceHardware)
