/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <cmath>

#include <mitkHalfLifeConstants.h>
#include <mitkSUVCalculation.h>
#include <mitkSUVFunctorPolicy.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

class mitkSUVFunctorPolicyTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkSUVFunctorPolicyTestSuite);

  // Configuration contract
  MITK_TEST(Default_IsNotConfigured);
  MITK_TEST(SetAll_IsConfigured);
  MITK_TEST(Default_OperatorReturnsNaN);
  MITK_TEST(ZeroInjectedActivity_IsNotConfigured);
  MITK_TEST(ZeroHalfLife_IsNotConfigured);
  MITK_TEST(ZeroScaleNumerator_IsNotConfigured);

  // Generic SUVFunctorPolicy
  MITK_TEST(Generic_AppliesScaleNumerator);
  MITK_TEST(Generic_DecayTimePerVoxel);
  MITK_TEST(Generic_GetNumberOfOutputs_IsOne);
  MITK_TEST(Generic_Equality_OnScalarsOnly);

  // Backward-compat SUVbwFunctorPolicy
  MITK_TEST(SUVbw_MathMatchesGenericWithWeightTimes1000);
  MITK_TEST(SUVbw_SetBodyWeight_UpdatesScaleNumerator);

  CPPUNIT_TEST_SUITE_END();

private:

  static constexpr double kActivity = 1.85e8;                              // 5 mCi in Bq
  static constexpr double kWeight   = 70.0;                                // kg
  const double            kHalfLife = mitk::HALFLIFECONSTANTS::VALUE_18F;  // 6586.2 s

  static mitk::SUVFunctorPolicy::DecayTimeFunctionType ConstantDecay(double seconds)
  {
    return [seconds](const mitk::SUVFunctorPolicy::IndexType&) { return seconds; };
  }

public:

  // ---- Configuration contract ----

  void Default_IsNotConfigured()
  {
    mitk::SUVFunctorPolicy f;
    CPPUNIT_ASSERT(!f.IsConfigured());
  }

  void SetAll_IsConfigured()
  {
    mitk::SUVFunctorPolicy f;
    f.SetInjectedActivity(kActivity);
    f.SetScaleNumerator(kWeight * 1000.0);
    f.SetHalfLife(kHalfLife);
    f.SetDecayTimeFunctor(ConstantDecay(0.0));
    CPPUNIT_ASSERT(f.IsConfigured());
  }

  void Default_OperatorReturnsNaN()
  {
    // The functor must not throw on an unconfigured invocation; the
    // contract is "NaN out", which makes misuse visible at the output.
    // ITK's pipeline relies on a default-constructed functor being
    // safe to call.
    mitk::SUVFunctorPolicy f;
    f.SetDecayTimeFunctor(ConstantDecay(0.0));
    const auto out = f(1.0, mitk::SUVFunctorPolicy::IndexType{ {0, 0, 0} });
    CPPUNIT_ASSERT(std::isnan(out));
  }

  void ZeroInjectedActivity_IsNotConfigured()
  {
    // Zero injected activity is a valid IEEE float but a physical
    // nonsense: it would make the SUV denominator zero and produce
    // NaN/Inf voxels. IsConfigured() must reject it.
    mitk::SUVFunctorPolicy f;
    f.SetInjectedActivity(0.0);
    f.SetScaleNumerator(kWeight * 1000.0);
    f.SetHalfLife(kHalfLife);
    f.SetDecayTimeFunctor(ConstantDecay(0.0));
    CPPUNIT_ASSERT(!f.IsConfigured());
  }

  void ZeroHalfLife_IsNotConfigured()
  {
    // Zero half-life causes division by zero in the decay exponent.
    // IsConfigured() must reject it even though 0.0 is finite.
    mitk::SUVFunctorPolicy f;
    f.SetInjectedActivity(kActivity);
    f.SetScaleNumerator(kWeight * 1000.0);
    f.SetHalfLife(0.0);
    f.SetDecayTimeFunctor(ConstantDecay(0.0));
    CPPUNIT_ASSERT(!f.IsConfigured());
  }

  void ZeroScaleNumerator_IsNotConfigured()
  {
    // Zero scale numerator (body weight, LBM, or BSA) would make the
    // SUV numerator zero and produce an all-zero output image.
    // IsConfigured() must reject it.
    mitk::SUVFunctorPolicy f;
    f.SetInjectedActivity(kActivity);
    f.SetScaleNumerator(0.0);
    f.SetHalfLife(kHalfLife);
    f.SetDecayTimeFunctor(ConstantDecay(0.0));
    CPPUNIT_ASSERT(!f.IsConfigured());
  }

  // ---- Generic SUVFunctorPolicy ----

  void Generic_AppliesScaleNumerator()
  {
    constexpr double scaleNumerator = 70000.0;          // 70 kg in g
    constexpr double decayTime      = 0.0;              // Admin strategy
    constexpr double pixel          = 4.2e3;

    mitk::SUVFunctorPolicy f(kActivity, scaleNumerator, kHalfLife);
    f.SetDecayTimeFunctor(ConstantDecay(decayTime));

    const double expected = pixel * mitk::computeSUVScaleFactor(
      kActivity, scaleNumerator, decayTime, kHalfLife);
    const double actual = f(pixel, mitk::SUVFunctorPolicy::IndexType{ {0, 0, 0} });

    CPPUNIT_ASSERT_DOUBLES_EQUAL(expected, actual, std::fabs(expected) * 1e-12);
  }

  void Generic_DecayTimePerVoxel()
  {
    // Per-voxel decay encoded in the slice index (idx[2]). Verifies the
    // functor wires the index through to the decay-time callback.
    mitk::SUVFunctorPolicy f(kActivity, kWeight * 1000.0, kHalfLife);
    f.SetDecayTimeFunctor([](const mitk::SUVFunctorPolicy::IndexType& i) {
      return static_cast<double>(i[2]) * 100.0;
    });

    constexpr double pixel = 1.0e3;
    const auto out0 = f(pixel, mitk::SUVFunctorPolicy::IndexType{ {0, 0, 0} });
    const auto out5 = f(pixel, mitk::SUVFunctorPolicy::IndexType{ {0, 0, 5} });

    const double exp0 = pixel * mitk::computeSUVScaleFactor(kActivity, kWeight * 1000.0, 0.0,   kHalfLife);
    const double exp5 = pixel * mitk::computeSUVScaleFactor(kActivity, kWeight * 1000.0, 500.0, kHalfLife);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(exp0, out0, std::fabs(exp0) * 1e-12);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(exp5, out5, std::fabs(exp5) * 1e-12);
  }

  void Generic_GetNumberOfOutputs_IsOne()
  {
    mitk::SUVFunctorPolicy f;
    CPPUNIT_ASSERT_EQUAL(1u, f.GetNumberOfOutputs());
  }

  void Generic_Equality_OnScalarsOnly()
  {
    // operator== compares the three scalar parameters; the decay-time
    // functor is deliberately ignored (std::function has no meaningful
    // equality).
    mitk::SUVFunctorPolicy a(kActivity, kWeight * 1000.0, kHalfLife);
    mitk::SUVFunctorPolicy b(kActivity, kWeight * 1000.0, kHalfLife);
    mitk::SUVFunctorPolicy c(kActivity, (kWeight + 1.0) * 1000.0, kHalfLife);

    a.SetDecayTimeFunctor(ConstantDecay(0.0));
    b.SetDecayTimeFunctor(ConstantDecay(123.0));  // different functor

    CPPUNIT_ASSERT(a == b);
    CPPUNIT_ASSERT(!(a != b));
    CPPUNIT_ASSERT(a != c);
    CPPUNIT_ASSERT(!(a == c));
  }

  // ---- Backward-compat SUVbwFunctorPolicy ----

  void SUVbw_MathMatchesGenericWithWeightTimes1000()
  {
    constexpr double decayTime = 1800.0;
    constexpr double pixel     = 4.2e3;

    mitk::SUVbwFunctorPolicy bw(kActivity, kWeight, kHalfLife);
    bw.SetDecayTimeFunctor(ConstantDecay(decayTime));

    mitk::SUVFunctorPolicy generic(kActivity, kWeight * 1000.0, kHalfLife);
    generic.SetDecayTimeFunctor(ConstantDecay(decayTime));

    const auto bwOut      = bw(pixel,      mitk::SUVFunctorPolicy::IndexType{ {0, 0, 0} });
    const auto genericOut = generic(pixel, mitk::SUVFunctorPolicy::IndexType{ {0, 0, 0} });

    CPPUNIT_ASSERT_DOUBLES_EQUAL(genericOut, bwOut, std::fabs(genericOut) * 1e-12);
  }

  void SUVbw_SetBodyWeight_UpdatesScaleNumerator()
  {
    mitk::SUVbwFunctorPolicy bw;
    bw.SetInjectedActivity(kActivity);
    bw.SetHalfLife(kHalfLife);
    bw.SetBodyWeight(kWeight);
    bw.SetDecayTimeFunctor(ConstantDecay(0.0));

    constexpr double pixel = 1.0e3;
    const double expected = pixel * mitk::computeSUVbwScaleFactor(kActivity, kWeight, 0.0, kHalfLife);
    const double actual   = bw(pixel, mitk::SUVFunctorPolicy::IndexType{ {0, 0, 0} });

    CPPUNIT_ASSERT_DOUBLES_EQUAL(expected, actual, std::fabs(expected) * 1e-12);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkSUVFunctorPolicy)
