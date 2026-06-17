/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <cmath>
#include <limits>

#include <mitkSUVCalculation.h>
#include <mitkHalfLifeConstants.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

class mitkSUVCalculationTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkSUVCalculationTestSuite);
  MITK_TEST(ScaleFactor_KnownInputs);
  MITK_TEST(ScaleFactor_ZeroDecay_EqualsRatio);
  MITK_TEST(ScaleFactor_OneHalfLife_DoublesRatio);
  MITK_TEST(ScaleFactor_NegativeDecay_HalvesRatio);
  MITK_TEST(ComputeSUVbw_RoundTrip);
  MITK_TEST(ScaleFactor_LargeDecay_Finite);
  MITK_TEST(HalfLifeConstants_PinnedValues);
  CPPUNIT_TEST_SUITE_END();

public:

  void ScaleFactor_KnownInputs()
  {
    // Realistic FDG / 18F numbers. The expected value is recomputed from the
    // closed-form formula so the test pins the *contract*, not just a number
    // someone copy-pasted.
    constexpr double activity   = 1.85e8;                              // 5 mCi in Bq
    constexpr double weight     = 70.0;                                // kg
    constexpr double decayTime  = 3600.0;                              // 60 min
    const double halfLife       = mitk::HALFLIFECONSTANTS::VALUE_18F;  // 6586.2 s

    const double expected =
      (weight * 1000.0) / (activity * std::pow(2.0, -decayTime / halfLife));
    const double actual =
      mitk::computeSUVbwScaleFactor(activity, weight, decayTime, halfLife);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(expected, actual, expected * 1e-12);
  }

  void ScaleFactor_ZeroDecay_EqualsRatio()
  {
    // Pins the Admin-strategy semantics: when decayTime == 0, the decay term
    // collapses to 2^0 = 1 and the factor is exactly weight*1000 / activity.
    constexpr double activity = 2.0e8;
    constexpr double weight   = 80.0;
    constexpr double halfLife = 6586.26;

    const double actual =
      mitk::computeSUVbwScaleFactor(activity, weight, 0.0, halfLife);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(weight * 1000.0 / activity, actual, 1e-15);
  }

  void ScaleFactor_OneHalfLife_DoublesRatio()
  {
    // decayTime == halfLife -> 2^(-1) decay term -> factor = 2 * (weight*1000/activity).
    constexpr double activity = 1.0e8;
    constexpr double weight   = 75.0;
    constexpr double halfLife = 6586.26;

    const double ratio  = weight * 1000.0 / activity;
    const double actual = mitk::computeSUVbwScaleFactor(activity, weight, halfLife, halfLife);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0 * ratio, actual, 2.0 * ratio * 1e-12);
  }

  void ScaleFactor_NegativeDecay_HalvesRatio()
  {
    // Pins current behaviour: a negative decay time produces a factor
    // *smaller* than the ratio (decay term > 1). A future change to throw on
    // negative decay would flip this; we want that to be a deliberate
    // decision, not an accidental regression.
    constexpr double activity = 1.0e8;
    constexpr double weight   = 75.0;
    constexpr double halfLife = 6586.26;

    const double ratio  = weight * 1000.0 / activity;
    const double actual = mitk::computeSUVbwScaleFactor(activity, weight, -halfLife, halfLife);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5 * ratio, actual, 0.5 * ratio * 1e-12);
  }

  void ComputeSUVbw_RoundTrip()
  {
    constexpr double activity  = 1.85e8;
    constexpr double weight    = 70.0;
    constexpr double decayTime = 3600.0;
    constexpr double halfLife  = 6586.26;
    constexpr double value     = 4.2e3;

    const double scale = mitk::computeSUVbwScaleFactor(activity, weight, decayTime, halfLife);
    const double suv   = mitk::computeSUVbw(value, activity, weight, decayTime, halfLife);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(value * scale, suv, std::fabs(value * scale) * 1e-12);
  }

  void ScaleFactor_LargeDecay_Finite()
  {
    // 20 half-lives -- extreme but legal input. Factor blows up to ~1e6 *
    // (weight*1000/activity) but must remain finite.
    constexpr double activity = 1.0e8;
    constexpr double weight   = 75.0;
    constexpr double halfLife = 6586.26;
    const double decayTime    = 20.0 * halfLife;

    const double actual = mitk::computeSUVbwScaleFactor(activity, weight, decayTime, halfLife);

    CPPUNIT_ASSERT_MESSAGE("Large-decay factor must be finite.", std::isfinite(actual));
    CPPUNIT_ASSERT_MESSAGE("Large-decay factor must be positive.", actual > 0.0);
  }

  void HalfLifeConstants_PinnedValues()
  {
    // Pin the literal half-life values. The formula tests recompute their
    // "expected" from the same constant and so cannot catch a typo in the
    // constant itself; a wrong half-life rescales every SUV. Values are
    // NNDC NuDat 3 (see mitkHalfLifeConstants.h).
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6586.2, mitk::HALFLIFECONSTANTS::VALUE_18F,  1e-9);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4062.6, mitk::HALFLIFECONSTANTS::VALUE_68Ga, 1e-9);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1221.8, mitk::HALFLIFECONSTANTS::VALUE_11C,  1e-9);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(122.24, mitk::HALFLIFECONSTANTS::VALUE_15O,  1e-9);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkSUVCalculation)
