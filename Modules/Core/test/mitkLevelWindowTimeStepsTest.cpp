/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkImage.h>
#include <mitkImageWriteAccessor.h>
#include <mitkLevelWindow.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <algorithm>
#include <limits>
#include <vector>

namespace
{
  /** A float image of n x 1 x 1 voxels with one time step per entry; all entries have n values. */
  mitk::Image::Pointer CreateImage(const std::vector<std::vector<float>>& valuesPerTimeStep)
  {
    unsigned int dimensions[] = { static_cast<unsigned int>(valuesPerTimeStep.front().size()), 1, 1,
                                  static_cast<unsigned int>(valuesPerTimeStep.size()) };
    auto image = mitk::Image::New();
    image->Initialize(mitk::MakeScalarPixelType<float>(), 4, dimensions);

    for (mitk::TimeStepType t = 0; t < valuesPerTimeStep.size(); ++t)
    {
      mitk::ImageWriteAccessor accessor(image, image->GetVolumeData(t));
      std::copy(valuesPerTimeStep[t].begin(), valuesPerTimeStep[t].end(), static_cast<float*>(accessor.GetData()));
    }

    return image;
  }
}

class mitkLevelWindowTimeStepsTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkLevelWindowTimeStepsTestSuite);
  MITK_TEST(SetAuto_OtherStepsMinimumAboveOverallMinimum_IsSecondMinimum);
  MITK_TEST(SetAuto_ConstantStepAtMaximum_IsNoSecondMaximum);
  MITK_TEST(SetAuto_InfiniteMinimumInLaterStep_RangeStartsAtSmallestFiniteValue);
  MITK_TEST(SetToImageRange_LaterStepHigher_RangeCoversIt);
  CPPUNIT_TEST_SUITE_END();

public:
  void SetAuto_OtherStepsMinimumAboveOverallMinimum_IsSecondMinimum()
  {
    const auto image = CreateImage({ { 0, 100, 200, 900 }, { 50, 100, 200, 900 } });

    mitk::LevelWindow levelWindow;
    levelWindow.SetAuto(image, false, false);

    // Few voxels hold the minimum, so the window starts at the second smallest value.
    CPPUNIT_ASSERT_EQUAL(mitk::ScalarType(50.0), levelWindow.GetLowerWindowBound());
  }

  void SetAuto_ConstantStepAtMaximum_IsNoSecondMaximum()
  {
    const auto image = CreateImage({ { 0, 400, 600, 1000 }, { 1000, 1000, 1000, 1000 } });

    mitk::LevelWindow levelWindow;
    levelWindow.SetAuto(image, false, false);

    // Many voxels hold the maximum, so the window ends half the inner range [400, 600]
    // above the second largest value.
    CPPUNIT_ASSERT_EQUAL(mitk::ScalarType(700.0), levelWindow.GetUpperWindowBound());
  }

  void SetAuto_InfiniteMinimumInLaterStep_RangeStartsAtSmallestFiniteValue()
  {
    constexpr auto inf = std::numeric_limits<float>::infinity();
    const auto image = CreateImage({ { 10, 20, 30, 40 }, { -inf, -inf, 0, 40 } });

    mitk::LevelWindow levelWindow;
    levelWindow.SetAuto(image, false, false);

    // More than a fifth of the voxels hold the minimum, so the window is clamped to the
    // range instead of starting at the second smallest value and widening the range.
    CPPUNIT_ASSERT_EQUAL(mitk::ScalarType(0.0), levelWindow.GetRangeMin());
    CPPUNIT_ASSERT_EQUAL(mitk::ScalarType(0.0), levelWindow.GetLowerWindowBound());
  }

  void SetToImageRange_LaterStepHigher_RangeCoversIt()
  {
    const auto image = CreateImage({ { 0, 10, 20, 30 }, { 0, 50, 100, 150 } });

    mitk::LevelWindow levelWindow;
    levelWindow.SetToImageRange(image);

    CPPUNIT_ASSERT_EQUAL(mitk::ScalarType(150.0), levelWindow.GetRangeMax());
    CPPUNIT_ASSERT_EQUAL(mitk::ScalarType(150.0), levelWindow.GetUpperWindowBound());
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkLevelWindowTimeSteps)
