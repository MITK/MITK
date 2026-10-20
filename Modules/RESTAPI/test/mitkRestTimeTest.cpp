/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>

#include <mitkImage.h>
#include <mitkProportionalTimeGeometry.h>
#include <mitkRestRoute.h>
#include <mitkRestTime.h>

namespace
{
  mitk::RestRequest MakeRequest(const std::string& target)
  {
    mitk::RestRequest request;
    request.Method = "GET";
    request.SetTarget(target);
    return request;
  }

  /** An image with four time steps of 100 ms from 0 ms on, or one time step. */
  mitk::Image::Pointer MakeImage(unsigned int timeSteps)
  {
    unsigned int dimensions[] = { 2, 2, 2, timeSteps };
    auto image = mitk::Image::New();
    image->Initialize(mitk::MakeScalarPixelType<unsigned char>(), timeSteps > 1 ? 4 : 3, dimensions);

    if (timeSteps > 1)
    {
      auto timeGeometry = mitk::ProportionalTimeGeometry::New();
      timeGeometry->Initialize(image->GetGeometry(), timeSteps);
      timeGeometry->SetFirstTimePoint(0.0);
      timeGeometry->SetStepDuration(100.0);
      image->SetTimeGeometry(timeGeometry);
    }

    return image;
  }
}

class mitkRestTimeTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkRestTimeTestSuite);
  MITK_TEST(TimePointSelectsTheStepOfEachData);
  MITK_TEST(TimePointOutsideTheDataSelectsNothing);
  MITK_TEST(AllSelectsEveryStep);
  MITK_TEST(InvalidTimePointIsRejected);
  MITK_TEST(StepsAreDescribedAsTimePoints);
  CPPUNIT_TEST_SUITE_END();

public:
  void TimePointSelectsTheStepOfEachData()
  {
    const auto selection = mitk::RestTimeSelection::FromRequest(MakeRequest("/api/v1/x?timepoint_ms=250"));

    CPPUNIT_ASSERT(!selection.All);
    CPPUNIT_ASSERT_EQUAL(250.0, selection.TimePoint);
    CPPUNIT_ASSERT(selection.GetTimeSteps(MakeImage(4)) == std::vector<mitk::TimeStepType>{ 2 });
    CPPUNIT_ASSERT(selection.GetTimeSteps(MakeImage(1)) == std::vector<mitk::TimeStepType>{ 0 });
  }

  void TimePointOutsideTheDataSelectsNothing()
  {
    const auto selection = mitk::RestTimeSelection::FromRequest(MakeRequest("/api/v1/x?timepoint_ms=400"));

    CPPUNIT_ASSERT(selection.GetTimeSteps(MakeImage(4)).empty());
    CPPUNIT_ASSERT(selection.GetTimeSteps(MakeImage(1)) == std::vector<mitk::TimeStepType>{ 0 });
  }

  void AllSelectsEveryStep()
  {
    const auto selection = mitk::RestTimeSelection::FromRequest(MakeRequest("/api/v1/x?timepoint_ms=all"));

    CPPUNIT_ASSERT(selection.All);
    CPPUNIT_ASSERT(selection.GetTimeSteps(MakeImage(4)) == (std::vector<mitk::TimeStepType>{ 0, 1, 2, 3 }));
    CPPUNIT_ASSERT_EQUAL(std::string("\"all\""), selection.ToJson().dump());
  }

  void InvalidTimePointIsRejected()
  {
    for (const std::string value : { "soon", "12ms", "nan" })
    {
      try
      {
        mitk::RestTimeSelection::FromRequest(MakeRequest("/api/v1/x?timepoint_ms=" + value));
        CPPUNIT_FAIL("No exception for " + value);
      }
      catch (const mitk::RestException& e)
      {
        CPPUNIT_ASSERT_EQUAL(400, e.GetError()["error"]["status"].get<int>());
      }
    }
  }

  void StepsAreDescribedAsTimePoints()
  {
    const auto dynamic = mitk::DescribeRestTimeSteps(MakeImage(4));
    const auto staticSteps = mitk::DescribeRestTimeSteps(MakeImage(1));

    CPPUNIT_ASSERT_EQUAL(std::size_t(4), dynamic.size());
    CPPUNIT_ASSERT_EQUAL(100.0, dynamic[1]["min_timepoint_ms"].get<double>());
    CPPUNIT_ASSERT_EQUAL(200.0, dynamic[1]["max_timepoint_ms"].get<double>());

    CPPUNIT_ASSERT_EQUAL(std::size_t(1), staticSteps.size());
    CPPUNIT_ASSERT(staticSteps[0]["min_timepoint_ms"].is_null());
    CPPUNIT_ASSERT(staticSteps[0]["max_timepoint_ms"].is_null());
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkRestTime)
