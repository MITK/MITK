/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkRestJob.h>
#include <mitkRestPythonSegmentation.h>
#include <mitkRestRoute.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <QCoreApplication>

class mitkRestPythonSegmentationTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkRestPythonSegmentationTestSuite);
  MITK_TEST(TestParsingNamesFromRequests);
  MITK_TEST(TestStartWithoutSessionFailsOnceIdle);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() override
  {
    static int argc = 1;
    static char name[] = "test";
    static char* argv[] = { name, nullptr };
    static QCoreApplication application(argc, argv);
  }

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

  /**
   * A session that ended while the job waited for the tool, as a DELETE
   * during a prompt or a cancel of the seeding ends it, fails the start
   * instead of reporting it as succeeded.
   */
  void TestStartWithoutSessionFailsOnceIdle()
  {
    auto tool = mitk::nnInteractiveTool::New();
    auto job = std::make_shared<mitk::RestJob>("id", "Start an nnInteractive session");

    mitk::SucceedRestJobWhenSessionSeeded(job, tool);

    // The thread of the test is the GUI thread, whose posted calls run here.
    while (!job->HasEnded())
      QCoreApplication::processEvents();

    const auto json = job->ToJson();
    CPPUNIT_ASSERT_EQUAL(std::string("failed"), json["state"].get<std::string>());
    CPPUNIT_ASSERT(json["error"].get<std::string>().find("ended before it was ready") != std::string::npos);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkRestPythonSegmentation)
