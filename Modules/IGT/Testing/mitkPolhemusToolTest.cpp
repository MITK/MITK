/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// Testing
#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"

#include "mitkPolhemusTool.h"
#include "mitkPolhemusTrackingDevice.h"

// VTK includes
#include <vtkDebugLeaks.h>

class mitkPolhemusToolTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPolhemusToolTestSuite);

  MITK_TEST(DefaultConstructor);
  MITK_TEST(GetToolPort_SetToolPortTwo_ReturnsToolPortTwo);
  MITK_TEST(GetDistortionLevel_SetLevelZero_ReturnsNoDistortion);
  MITK_TEST(GetDistortionLevel_SetLevelOne_ReturnsMinorDistortion);
  MITK_TEST(GetDistortionLevel_SetLevelTwo_ReturnsSignificantDistortion);
  MITK_TEST(GetDistortionLevel_SetLevelInvalidNegative_ReturnsUndefined);
  MITK_TEST(GetDistortionLevel_SetLevelInvalidPositive_ReturnsUndefined);

  CPPUNIT_TEST_SUITE_END();

private:
  mitk::PolhemusTrackingDevice::Pointer m_Device;
  mitk::PolhemusTool::Pointer m_Tool;

public:
  void setUp() override
  {
    m_Device = mitk::PolhemusTrackingDevice::New();
    m_Device->AddTool("Sensor-1", 1);
    m_Tool = dynamic_cast<mitk::PolhemusTool*>(m_Device->GetTool(0));
  }

  void tearDown() override
  {
    m_Tool = nullptr;
    m_Device = nullptr;
  }

  void DefaultConstructor()
  {
    CPPUNIT_ASSERT_EQUAL(1, m_Tool->GetToolPort());
    CPPUNIT_ASSERT_EQUAL(mitk::PolhemusTool::DistortionLevel::UNDEFINED, m_Tool->GetDistortionLevel());
  }

  void GetToolPort_SetToolPortTwo_ReturnsToolPortTwo()
  {
    m_Tool->SetToolPort(2);
    CPPUNIT_ASSERT_EQUAL(2, m_Tool->GetToolPort());
  }

  void GetDistortionLevel_SetLevelZero_ReturnsNoDistortion()
  {
    m_Tool->SetDistortionLevel(0);
    CPPUNIT_ASSERT_EQUAL(mitk::PolhemusTool::DistortionLevel::NO_DISTORTION, m_Tool->GetDistortionLevel());
  }

  void GetDistortionLevel_SetLevelOne_ReturnsMinorDistortion()
  {
    m_Tool->SetDistortionLevel(1);
    CPPUNIT_ASSERT_EQUAL(mitk::PolhemusTool::DistortionLevel::MINOR_DISTORTION, m_Tool->GetDistortionLevel());
  }

  void GetDistortionLevel_SetLevelTwo_ReturnsSignificantDistortion()
  {
    m_Tool->SetDistortionLevel(2);
    CPPUNIT_ASSERT_EQUAL(mitk::PolhemusTool::DistortionLevel::SIGNIFICANT_DISTORTION, m_Tool->GetDistortionLevel());
  }

  void GetDistortionLevel_SetLevelInvalidNegative_ReturnsUndefined()
  {
    m_Tool->SetDistortionLevel(-1);
    CPPUNIT_ASSERT_EQUAL(mitk::PolhemusTool::DistortionLevel::UNDEFINED, m_Tool->GetDistortionLevel());
  }

  void GetDistortionLevel_SetLevelInvalidPositive_ReturnsUndefined()
  {
    m_Tool->SetDistortionLevel(3);
    CPPUNIT_ASSERT_EQUAL(mitk::PolhemusTool::DistortionLevel::UNDEFINED, m_Tool->GetDistortionLevel());
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkPolhemusTool)
