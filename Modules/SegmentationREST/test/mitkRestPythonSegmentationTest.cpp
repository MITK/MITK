/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkRestPythonSegmentation.h>
#include <mitkRestRoute.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

class mitkRestPythonSegmentationTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkRestPythonSegmentationTestSuite);
  MITK_TEST(TestParsingNamesFromRequests);
  CPPUNIT_TEST_SUITE_END();

public:
  void TestParsingNamesFromRequests()
  {
    using Speed = mitk::TotalSegmentatorTool::Speed;
    CPPUNIT_ASSERT(mitk::ParseRestInteractionType({ { "type", "scribble" } }, "/p") == mitk::nnInteractive::InteractionType::Scribble);

    for (const auto speed : { Speed::Standard, Speed::Fast, Speed::Fastest })
      CPPUNIT_ASSERT(mitk::ParseRestSpeed(mitk::ToRestJson(speed), "/p") == speed);

    // Names are written in lower case; anything else, also of another type, is refused.
    CPPUNIT_ASSERT_THROW(mitk::ParseRestInteractionType({ { "type", "Point" } }, "/p"), mitk::RestException);
    CPPUNIT_ASSERT_THROW(mitk::ParseRestInteractionType(nlohmann::json::object(), "/p"), mitk::RestException);

    for (const auto& junk : { nlohmann::json("Fast"), nlohmann::json(1), nlohmann::json() })
      CPPUNIT_ASSERT_THROW(mitk::ParseRestSpeed(junk, "/p"), mitk::RestException);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkRestPythonSegmentation)
