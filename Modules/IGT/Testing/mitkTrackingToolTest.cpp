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
    MITK_INFO << "GetToolName_Default_ReturnsEmptyString";
    CPPUNIT_ASSERT_MESSAGE("Tool name should be empty", !strcmp(m_TrackingTool->GetToolName(), ""));
  }

  void GetErrorMessage_Default_ReturnsEmptyString()
  {
    MITK_INFO << "GetErrorMessage_Default_ReturnsEmptyString";
    CPPUNIT_ASSERT_MESSAGE("Error message should be empty", !strcmp(m_TrackingTool->GetErrorMessage(), ""));
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkTrackingTool)
