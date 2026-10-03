/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkImage.h>
#include <mitkImageStatisticsHolder.h>
#include <mitkImageWriteAccessor.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <cmath>
#include <limits>
#include <random>
#include <thread>
#include <vector>

namespace
{
  struct ExpectedExtrema
  {
    double Min;
    double SecondMin;
    double Max;
    double SecondMax;
    unsigned int MinCount;
    unsigned int MaxCount;
  };

  /** The extrema by their definition, one value after the other. */
  template <typename TPixel>
  ExpectedExtrema ComputeExpectedExtrema(const std::vector<TPixel> &values)
  {
    ExpectedExtrema expected = {std::numeric_limits<double>::max(), std::numeric_limits<double>::max(),
                                std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest(), 0, 0};

    for (const auto pixel : values)
    {
      const auto value = static_cast<double>(pixel);

      if (std::isnan(value))
        continue;

      if (value < expected.Min)
      {
        expected.SecondMin = expected.Min;
        expected.Min = value;
        expected.MinCount = 1;
      }
      else if (value == expected.Min)
      {
        ++expected.MinCount;
      }
      else if (value < expected.SecondMin)
      {
        expected.SecondMin = value;
      }

      if (value > expected.Max)
      {
        expected.SecondMax = expected.Max;
        expected.Max = value;
        expected.MaxCount = 1;
      }
      else if (value == expected.Max)
      {
        ++expected.MaxCount;
      }
      else if (value > expected.SecondMax)
      {
        expected.SecondMax = value;
      }
    }

    return expected;
  }

  template <typename TPixel>
  mitk::Image::Pointer CreateImage(const std::vector<TPixel> &values, const std::vector<unsigned int> &dimensions)
  {
    auto image = mitk::Image::New();
    image->Initialize(mitk::MakeScalarPixelType<TPixel>(), static_cast<unsigned int>(dimensions.size()),
                      dimensions.data());

    mitk::ImageWriteAccessor accessor(image);
    std::copy(values.begin(), values.end(), static_cast<TPixel *>(accessor.GetData()));

    return image;
  }

  /** Values from a range whose ends occur several times each, scattered over
   * the whole image so that they meet in more than one run of voxels. */
  template <typename TPixel>
  std::vector<TPixel> CreateValues(std::size_t count, int lowest, int highest)
  {
    std::mt19937 generator(7);
    std::uniform_int_distribution<int> distribution(lowest + 2, highest - 2);
    std::uniform_int_distribution<std::size_t> position(0, count - 1);

    std::vector<TPixel> values(count);

    for (auto &value : values)
      value = static_cast<TPixel>(distribution(generator));

    for (int i = 0; i < 5; ++i)
    {
      values[position(generator)] = static_cast<TPixel>(lowest);
      values[position(generator)] = static_cast<TPixel>(highest);
    }

    values[position(generator)] = static_cast<TPixel>(lowest + 1);
    values[position(generator)] = static_cast<TPixel>(highest - 1);

    return values;
  }

  void AssertExtrema(const ExpectedExtrema &expected, mitk::ImageStatisticsHolder *statistics, int t = 0)
  {
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong minimum", expected.Min, statistics->GetScalarValueMin(t));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong second minimum", expected.SecondMin, statistics->GetScalarValue2ndMin(t));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong maximum", expected.Max, statistics->GetScalarValueMax(t));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong second maximum", expected.SecondMax, statistics->GetScalarValue2ndMax(t));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong count of minimum", static_cast<double>(expected.MinCount),
                                 statistics->GetCountOfMinValuedVoxels(t));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong count of maximum", static_cast<double>(expected.MaxCount),
                                 statistics->GetCountOfMaxValuedVoxels(t));
  }
}

class mitkImageStatisticsHolderTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkImageStatisticsHolderTestSuite);
  MITK_TEST(ShortImage_ExtremaAsDefined);
  MITK_TEST(UnsignedCharImage_ExtremaAsDefined);
  MITK_TEST(FloatImageWithNaN_IgnoresNaN);
  MITK_TEST(AllNaN_ReportsZero);
  MITK_TEST(ConstantImage_SecondExtremaAreTheValue);
  MITK_TEST(TimeSeries_ExtremaPerTimeStep);
  MITK_TEST(ModifiedImage_Recomputes);
  MITK_TEST(ConcurrentRequests_AgreeWithDefinition);
  CPPUNIT_TEST_SUITE_END();

public:
  void ShortImage_ExtremaAsDefined()
  {
    const auto values = CreateValues<short>(64 * 48 * 32, -1024, 3071);
    auto image = CreateImage(values, {64, 48, 32});

    AssertExtrema(ComputeExpectedExtrema(values), image->GetStatistics());
  }

  void UnsignedCharImage_ExtremaAsDefined()
  {
    const auto values = CreateValues<unsigned char>(64 * 48 * 32, 0, 255);
    auto image = CreateImage(values, {64, 48, 32});

    AssertExtrema(ComputeExpectedExtrema(values), image->GetStatistics());
  }

  void FloatImageWithNaN_IgnoresNaN()
  {
    auto values = CreateValues<float>(32 * 32 * 32, -100, 100);

    for (std::size_t i = 0; i < values.size(); i += 97)
      values[i] = std::numeric_limits<float>::quiet_NaN();

    // The first value as well, which a computation seeding its extrema from
    // the first voxel would get wrong.
    values[0] = std::numeric_limits<float>::quiet_NaN();

    auto image = CreateImage(values, {32, 32, 32});

    AssertExtrema(ComputeExpectedExtrema(values), image->GetStatistics());
  }

  void AllNaN_ReportsZero()
  {
    auto image = CreateImage(std::vector<float>(1000, std::numeric_limits<float>::quiet_NaN()), {10, 10, 10});
    auto *statistics = image->GetStatistics();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong minimum", 0.0, statistics->GetScalarValueMin());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong maximum", 0.0, statistics->GetScalarValueMax());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong second minimum", 0.0, statistics->GetScalarValue2ndMin());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong second maximum", 0.0, statistics->GetScalarValue2ndMax());
  }

  void ConstantImage_SecondExtremaAreTheValue()
  {
    auto image = CreateImage(std::vector<short>(1000, 7), {10, 10, 10});

    AssertExtrema({7.0, 7.0, 7.0, 7.0, 1000, 1000}, image->GetStatistics());
  }

  void TimeSeries_ExtremaPerTimeStep()
  {
    const auto first = CreateValues<short>(1000, -10, 10);
    const auto second = CreateValues<short>(1000, 100, 200);

    auto values = first;
    values.insert(values.end(), second.begin(), second.end());

    auto image = CreateImage(values, {10, 10, 10, 2});

    AssertExtrema(ComputeExpectedExtrema(first), image->GetStatistics(), 0);
    AssertExtrema(ComputeExpectedExtrema(second), image->GetStatistics(), 1);
  }

  void ModifiedImage_Recomputes()
  {
    auto values = CreateValues<short>(1000, -10, 10);
    auto image = CreateImage(values, {10, 10, 10});

    AssertExtrema(ComputeExpectedExtrema(values), image->GetStatistics());

    values[500] = 1000;

    {
      mitk::ImageWriteAccessor accessor(image);
      static_cast<short *>(accessor.GetData())[500] = 1000;
    }

    image->Modified();

    AssertExtrema(ComputeExpectedExtrema(values), image->GetStatistics());
  }

  void ConcurrentRequests_AgreeWithDefinition()
  {
    const auto values = CreateValues<int>(128 * 128 * 64, -3024, 3071);
    auto image = CreateImage(values, {128, 128, 64});
    auto *statistics = image->GetStatistics();

    std::vector<double> minima(4);
    std::vector<std::thread> threads;

    for (std::size_t i = 0; i < minima.size(); ++i)
      threads.emplace_back([statistics, &minima, i]() { minima[i] = statistics->GetScalarValueMin(); });

    for (auto &thread : threads)
      thread.join();

    for (const auto minimum : minima)
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong minimum from a concurrent request", -3024.0, minimum);

    AssertExtrema(ComputeExpectedExtrema(values), statistics);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkImageStatisticsHolder)
