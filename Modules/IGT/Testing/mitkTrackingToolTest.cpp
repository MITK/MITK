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
  MITK_TEST(GetTrackingError_Default_ReturnsZero);
  MITK_TEST(GetTrackingError_SetValidTrackingError_ReturnsValidTrackingError);
  MITK_TEST(IsDataValid_Default_ReturnsFalse);
  MITK_TEST(IsDataValid_SetTrue_ReturnsTrue);
  MITK_TEST(IsDataValid_SetFalse_ReturnsFalse);

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
    srand(time(nullptr));

    // generate a random position to test Set/GetPosition()
    mitk::Point3D expectedPosition;
    expectedPosition[0] = rand()%1000;
    expectedPosition[1] = rand()%1000;
    expectedPosition[2] = rand()%1000;

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
    srand(time(nullptr));

    // generate a random orientation to test Set/GetOrientation()
    mitk::Quaternion expectedOrientation;
    expectedOrientation[0] = (rand()%1000)/1000.0;
    expectedOrientation[1] = (rand()%1000)/1000.0;
    expectedOrientation[2] = (rand()%1000)/1000.0;
    expectedOrientation[3] = (rand()%1000)/1000.0;

    m_TrackingTool->SetOrientation(expectedOrientation);

    mitk::Quaternion actualOrientation;
    m_TrackingTool->GetOrientation(actualOrientation);

    CPPUNIT_ASSERT_EQUAL(expectedOrientation, actualOrientation);
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

};

MITK_TEST_SUITE_REGISTRATION(mitkTrackingTool)
