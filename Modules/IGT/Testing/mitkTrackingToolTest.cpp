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

#include "mitkTrackingTool.h"

// VTK includes
#include <vtkDebugLeaks.h>

class mitkTrackingToolTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkTrackingToolTestSuite);

  MITK_TEST(GetToolName_Default_ReturnsEmptyString);
  MITK_TEST(GetErrorMessage_Default_ReturnsEmptyString);
  MITK_TEST(IsEnabled_Default_ReturnsTrue);
  MITK_TEST(IsEnabled_Enable_ReturnsTrue);
  MITK_TEST(IsEnabled_Disable_ReturnsFalse);
  MITK_TEST(GetPosition_Default_ReturnsAllValuesZero);
  MITK_TEST(GetPosition_SetValidPosition_ReturnsValidPosition);
  MITK_TEST(GetOrientation_Default_ReturnsAllValuesZero);
  MITK_TEST(GetOrientation_SetValidOrientation_ReturnsValidOrientation);
  MITK_TEST(GetOrientation_SetToolTip_ReturnsTransformedOrientation);
  MITK_TEST(GetTrackingError_Default_ReturnsZero);
  MITK_TEST(GetTrackingError_SetValidTrackingError_ReturnsValidTrackingError);
  MITK_TEST(IsDataValid_Default_ReturnsFalse);
  MITK_TEST(IsDataValid_SetTrue_ReturnsTrue);
  MITK_TEST(IsDataValid_SetFalse_ReturnsFalse);
  MITK_TEST(GetToolTipPosition_Default_ReturnsAllValuesZero);
  MITK_TEST(GetToolTipOrientation_Default_ReturnsDefaultOrientation);
  MITK_TEST(IsToolTipSet_Default_ReturnsFalse);
  MITK_TEST(GetToolTipX_SetValidToolTip_ReturnsValidToolTip);
  MITK_TEST(GetMTime_SetPosition_ReturnsModifiedTime);
  MITK_TEST(GetMTime_SetOrientation_ReturnsModifiedTime);

  CPPUNIT_TEST_SUITE_END();

private:
  mitk::TrackingTool::Pointer m_TrackingTool;

public:
  void setUp() override
  {
    m_TrackingTool = mitk::TrackingTool::New();
  }

  void tearDown() override
  {
    m_TrackingTool = nullptr;
  }

  void GetToolName_Default_ReturnsEmptyString()
  {
    CPPUNIT_ASSERT_MESSAGE("Tool name should be empty", !strcmp(m_TrackingTool->GetToolName(), ""));
  }

  void GetErrorMessage_Default_ReturnsEmptyString()
  {
    CPPUNIT_ASSERT_MESSAGE("Error message should be empty", !strcmp(m_TrackingTool->GetErrorMessage(), ""));
  }

  void IsEnabled_Default_ReturnsTrue()
  {
    CPPUNIT_ASSERT_EQUAL(true, m_TrackingTool->IsEnabled());
  }

  void IsEnabled_Enable_ReturnsTrue()
  {
    m_TrackingTool->Enable();
    CPPUNIT_ASSERT_EQUAL(true, m_TrackingTool->IsEnabled());
  }

  void IsEnabled_Disable_ReturnsFalse()
  {
    m_TrackingTool->Disable();
    CPPUNIT_ASSERT_EQUAL(false, m_TrackingTool->IsEnabled());
  }

  void GetPosition_Default_ReturnsAllValuesZero()
  {
    mitk::Point3D expectedPosition;
    expectedPosition.Fill(0);

    mitk::Point3D actualPosition;
    m_TrackingTool->GetPosition(actualPosition);

    CPPUNIT_ASSERT_EQUAL(expectedPosition, actualPosition);
  }

  void GetPosition_SetValidPosition_ReturnsValidPosition()
  {
    mitk::Point3D expectedPosition;
    expectedPosition[0] = 100;
    expectedPosition[1] = 200;
    expectedPosition[2] = 300;

    m_TrackingTool->SetPosition(expectedPosition);

    mitk::Point3D actualPosition;
    m_TrackingTool->GetPosition(actualPosition);

    CPPUNIT_ASSERT_EQUAL(expectedPosition, actualPosition);
  }

  void GetOrientation_Default_ReturnsAllValuesZero()
  {
    mitk::Quaternion expectedOrientation(0, 0, 0, 0);

    mitk::Quaternion actualOrientation;
    m_TrackingTool->GetOrientation(actualOrientation);

    CPPUNIT_ASSERT_EQUAL(expectedOrientation, actualOrientation);
  }

  void GetOrientation_SetValidOrientation_ReturnsValidOrientation()
  {
    mitk::Quaternion expectedOrientation(0.344, 0.625, 0.999, 0.574);
    m_TrackingTool->SetOrientation(expectedOrientation);

    mitk::Quaternion actualOrientation;
    m_TrackingTool->GetOrientation(actualOrientation);

    CPPUNIT_ASSERT_EQUAL(expectedOrientation, actualOrientation);
  }

  void GetOrientation_SetToolTip_ReturnsTransformedOrientation()
  {
    mitk::Point3D toolTipPosition;
    mitk::FillVector3D(toolTipPosition, 1, 1, 1);

    mitk::Quaternion toolTipOrientation = mitk::Quaternion(0.5, 0, 0, 1);

    m_TrackingTool->SetToolTipPosition(toolTipPosition, toolTipOrientation);

    mitk::Point3D toolPosition;
    mitk::FillVector3D(toolPosition, 5, 6, 7);

    mitk::Quaternion toolOrientation = mitk::Quaternion(0, 0.5, 0, 1);

    m_TrackingTool->SetPosition(toolPosition);
    m_TrackingTool->SetOrientation(toolOrientation);

    mitk::Quaternion expectedToolOrientation = mitk::Quaternion(0.5, 0.5, -0.25, 1);

    mitk::Quaternion actualToolOrientation;
    m_TrackingTool->GetOrientation(actualToolOrientation);

    CPPUNIT_ASSERT_EQUAL(expectedToolOrientation, actualToolOrientation);
  }

  void GetTrackingError_Default_ReturnsZero()
  {
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0f, m_TrackingTool->GetTrackingError(), mitk::eps);
  }

  void GetTrackingError_SetValidTrackingError_ReturnsValidTrackingError()
  {
    m_TrackingTool->SetTrackingError(0.2f);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.2f, m_TrackingTool->GetTrackingError(), mitk::eps);
  }

  void IsDataValid_Default_ReturnsFalse()
  {
    CPPUNIT_ASSERT_EQUAL(false, m_TrackingTool->IsDataValid());
  }

  void IsDataValid_SetTrue_ReturnsTrue()
  {
    m_TrackingTool->SetDataValid(true);
    CPPUNIT_ASSERT_EQUAL(true, m_TrackingTool->IsDataValid());
  }

  void IsDataValid_SetFalse_ReturnsFalse()
  {
    m_TrackingTool->SetDataValid(false);
    CPPUNIT_ASSERT_EQUAL(false, m_TrackingTool->IsDataValid());
  }

  void GetToolTipPosition_Default_ReturnsAllValuesZero()
  {
    mitk::Point3D expectedPosition;
    expectedPosition.Fill(0);

    CPPUNIT_ASSERT_EQUAL(expectedPosition, m_TrackingTool->GetToolTipPosition());
  }

  void GetToolTipOrientation_Default_ReturnsDefaultOrientation()
  {
    mitk::Quaternion expectedOrientation(0, 0, 0, 1);

    CPPUNIT_ASSERT_EQUAL(expectedOrientation,  m_TrackingTool->GetToolAxisOrientation());
  }

  void IsToolTipSet_Default_ReturnsFalse()
  {
    CPPUNIT_ASSERT_EQUAL(false, m_TrackingTool->IsToolTipSet());
  }

  void GetToolTipX_SetValidToolTip_ReturnsValidToolTip()
  {
    mitk::Point3D expectedPosition;
    expectedPosition[0] = 100;
    expectedPosition[1] = 200;
    expectedPosition[2] = 300;

    mitk::Quaternion expectedOrientation(0.344, 0.625, 0.999, 0.574);

    m_TrackingTool->SetToolTipPosition(expectedPosition, expectedOrientation);

    CPPUNIT_ASSERT_EQUAL(expectedPosition, m_TrackingTool->GetToolTipPosition());
    CPPUNIT_ASSERT_EQUAL(expectedOrientation, m_TrackingTool->GetToolAxisOrientation());
    CPPUNIT_ASSERT_EQUAL(true, m_TrackingTool->IsToolTipSet());
  }

  void GetMTime_SetPosition_ReturnsModifiedTime()
  {
    itk::ModifiedTimeType time = m_TrackingTool->GetMTime();

    mitk::Point3D position1;
    mitk::FillVector3D(position1, 1.1, 2.2, 3.3);
    m_TrackingTool->SetPosition(position1);

    CPPUNIT_ASSERT(time < m_TrackingTool->GetMTime());

    time = m_TrackingTool->GetMTime();
    m_TrackingTool->SetPosition(position1);

    CPPUNIT_ASSERT(time == m_TrackingTool->GetMTime());

    mitk::Point3D position2;
    mitk::FillVector3D(position2, 1, 2, 3);
    m_TrackingTool->SetPosition(position2);

    CPPUNIT_ASSERT(time < m_TrackingTool->GetMTime());
  }

  void GetMTime_SetOrientation_ReturnsModifiedTime()
  {
    itk::ModifiedTimeType time = m_TrackingTool->GetMTime();

    mitk::Quaternion orientation1 = mitk::Quaternion(0, 0, 0.70710678118654757, 0.70710678118654757);
    m_TrackingTool->SetOrientation(orientation1);

    CPPUNIT_ASSERT(time < m_TrackingTool->GetMTime());

    time = m_TrackingTool->GetMTime();
    m_TrackingTool->SetOrientation(orientation1);

    CPPUNIT_ASSERT(time == m_TrackingTool->GetMTime());

    mitk::Quaternion orientation2 = mitk::Quaternion(0, 0, 0.70710678118654757, 0.70710678118654757 + 0.00001);
    m_TrackingTool->SetOrientation(orientation2);

    CPPUNIT_ASSERT(time < m_TrackingTool->GetMTime());
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkTrackingTool)
