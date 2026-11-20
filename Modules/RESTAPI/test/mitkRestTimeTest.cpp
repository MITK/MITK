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
#include <mitkLocaleSwitch.h>
#include <mitkLog.h>
#include <mitkProportionalTimeGeometry.h>
#include <mitkRestRoute.h>
#include <mitkRestTime.h>

#include <cmath>
#include <limits>
#include <locale>
#include <memory>
#include <sstream>
#include <stdexcept>

namespace
{
  mitk::RestRequest MakeRequest(const std::string& target)
  {
    mitk::RestRequest request;
    request.Method = "GET";
    request.SetTarget(target);
    return request;
  }

  /** An image with time steps of 100 ms (or the given duration) from 0 ms (or the given time point) on, or one time step. */
  mitk::Image::Pointer MakeImage(unsigned int timeSteps, double stepDuration = 100.0, double firstTimePoint = 0.0)
  {
    unsigned int dimensions[] = { 2, 2, 2, timeSteps };
    auto image = mitk::Image::New();
    image->Initialize(mitk::MakeScalarPixelType<unsigned char>(), timeSteps > 1 ? 4 : 3, dimensions);

    if (timeSteps > 1)
    {
      auto timeGeometry = mitk::ProportionalTimeGeometry::New();
      timeGeometry->Initialize(image->GetGeometry(), timeSteps);
      timeGeometry->SetFirstTimePoint(firstTimePoint);
      timeGeometry->SetStepDuration(stepDuration);
      image->SetTimeGeometry(timeGeometry);
    }

    return image;
  }

  /** A time point as text that reads back as the same double. */
  std::string ToQueryValue(double timePoint)
  {
    std::ostringstream text;
    text.imbue(std::locale::classic());
    text.precision(std::numeric_limits<double>::max_digits10);
    text << timePoint;
    return text.str();
  }

  /** The least double below a value. */
  double Below(double value)
  {
    return std::nextafter(value, -std::numeric_limits<double>::infinity());
  }

  /** Makes a German locale the one of C and C++ for a while, if the system has one. */
  class GermanLocale
  {
  public:
    GermanLocale()
    {
      for (const char* name : { "de_DE.UTF-8", "de_DE.utf8", "de-DE" })
      {
        try
        {
          const std::locale german(name);
          m_Switch = std::make_unique<mitk::LocaleSwitch>(name);
          m_Previous = std::locale::global(german);
          m_Installed = true;
          return;
        }
        catch (const std::runtime_error&)
        {
        }
      }

      MITK_INFO << "The system has no German locale; the checks under it are skipped.";
    }

    ~GermanLocale()
    {
      if (m_Installed)
        std::locale::global(m_Previous);
    }

    GermanLocale(const GermanLocale&) = delete;
    GermanLocale& operator=(const GermanLocale&) = delete;

    bool IsInstalled() const
    {
      return m_Installed;
    }

  private:
    std::unique_ptr<mitk::LocaleSwitch> m_Switch;
    std::locale m_Previous;
    bool m_Installed = false;
  };

  /** Asserts that the steps of the selection of a time point, given as text, are the expected ones. */
  void AssertSteps(const std::string& timePoint, const mitk::Image* image, const std::vector<mitk::TimeStepType>& expected)
  {
    const auto selection = mitk::RestTimeSelection::FromRequest(MakeRequest("/api/v1/x?timepoint_ms=" + timePoint));
    CPPUNIT_ASSERT_MESSAGE("time point " + timePoint, selection.GetTimeSteps(image) == expected);
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
  MITK_TEST(TimePointsAtTheBoundsOfInexactStepsSelectTheirSteps);
  MITK_TEST(TimePointsAreReadWithoutRegardToTheLocale);
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
    for (const std::string value : { "soon", "12ms", "nan", "250,5", "", "0x10", "1e999" })
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

  void TimePointsAtTheBoundsOfInexactStepsSelectTheirSteps()
  {
    // 28 steps of a duration that has no exact binary form, like 28 frames per second,
    // from first time points at which the division alone misses the bounds of steps.
    const double duration = 1000.0 / 28;
    const unsigned int count = 28;

    for (const double firstTimePoint : { 0.0, 100.5, -37.25 })
    {
      const auto image = MakeImage(count, duration, firstTimePoint);
      const auto* timeGeometry = image->GetTimeGeometry();

      for (mitk::TimeStepType step = 0; step < count; ++step)
      {
        const auto first = timeGeometry->GetMinimumTimePoint(step);
        const auto end = timeGeometry->GetMaximumTimePoint(step);

        // The lower bound belongs to the step, as the description of the step tells clients.
        AssertSteps(ToQueryValue(first), image, { step });

        // The upper bound does not: it is the first time point of the next step.
        if (step + 1 < count)
          AssertSteps(ToQueryValue(end), image, { step + 1 });
        else
          AssertSteps(ToQueryValue(end), image, {});

        if (step > 0)
          AssertSteps(ToQueryValue(Below(first)), image, { step - 1 });
        else
          AssertSteps(ToQueryValue(Below(first)), image, {});

        AssertSteps(ToQueryValue(Below(end)), image, { step });
      }
    }
  }

  void TimePointsAreReadWithoutRegardToTheLocale()
  {
    const GermanLocale german;

    if (!german.IsInstalled())
      return;

    const auto image = MakeImage(28, 1000.0 / 28);

    // 250.5 ms is in step 7 (250.0 ms to 285.7 ms), and 1.234 ms in step 0; a dot of thousands would make it 1234 ms.
    AssertSteps("250.5", image, { 7 });
    AssertSteps("1.234", image, { 0 });
    AssertSteps(ToQueryValue(1000.0 / 28 * 13), image, { 13 });

    for (const std::string value : { "250,5", "1.234,5", "1,234" })
    {
      CPPUNIT_ASSERT_THROW(mitk::RestTimeSelection::FromRequest(MakeRequest("/api/v1/x?timepoint_ms=" + value)), mitk::RestException);
    }
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkRestTime)
