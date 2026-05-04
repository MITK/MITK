/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <cmath>
#include <limits>

#include <mitkSUVNormalizationStrategy.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

class mitkSUVNormalizationStrategyTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkSUVNormalizationStrategyTestSuite);

  // Body weight
  MITK_TEST(BW_Returns_WeightTimes1000);
  MITK_TEST(BW_VariantId);
  MITK_TEST(BW_MissingWeight_Throws);
  MITK_TEST(BW_NegativeWeight_Throws);

  // Lean body mass (Janmahasatian)
  MITK_TEST(LBM_Male_KnownInputs);
  MITK_TEST(LBM_Female_KnownInputs);
  MITK_TEST(LBM_VariantId);
  MITK_TEST(LBM_MissingHeight_Throws);
  MITK_TEST(LBM_MissingSex_Throws);
  MITK_TEST(LBM_MissingWeight_Throws);

  // Body surface area (DuBois)
  MITK_TEST(BSA_KnownInputs);
  MITK_TEST(BSA_VariantId);
  MITK_TEST(BSA_MissingHeight_Throws);
  MITK_TEST(BSA_MissingWeight_Throws);

  // Factory
  MITK_TEST(Factory_BW_ReturnsBodyWeightStrategy);
  MITK_TEST(Factory_LBM_ReturnsLeanBodyMassStrategy);
  MITK_TEST(Factory_BSA_ReturnsBodySurfaceAreaStrategy);

  CPPUNIT_TEST_SUITE_END();

public:

  // ---- Body weight ----

  void BW_Returns_WeightTimes1000()
  {
    mitk::SUVNormalizationInputs inputs;
    inputs.bodyWeightKg = 70.0;

    mitk::BodyWeightStrategy strategy;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(70000.0, strategy.ComputeScaleNumerator(inputs), 1e-9);
  }

  void BW_VariantId()
  {
    mitk::BodyWeightStrategy strategy;
    CPPUNIT_ASSERT_EQUAL(mitk::SUVVariant::BW, strategy.Variant());
  }

  void BW_MissingWeight_Throws()
  {
    mitk::SUVNormalizationInputs inputs;  // bodyWeightKg = NaN
    mitk::BodyWeightStrategy strategy;
    CPPUNIT_ASSERT_THROW(strategy.ComputeScaleNumerator(inputs),
                         mitk::MissingSUVInputException);
  }

  void BW_NegativeWeight_Throws()
  {
    mitk::SUVNormalizationInputs inputs;
    inputs.bodyWeightKg = -1.0;
    mitk::BodyWeightStrategy strategy;
    CPPUNIT_ASSERT_THROW(strategy.ComputeScaleNumerator(inputs),
                         mitk::MissingSUVInputException);
  }

  // ---- Lean body mass (Janmahasatian) ----
  //
  // Reference values recomputed from the closed-form Janmahasatian formula
  // so the tests pin the contract, not a hand-typed number:
  //   LBM_male   [kg] = (9270 * W) / (6680 + 216 * BMI)
  //   LBM_female [kg] = (9270 * W) / (8780 + 244 * BMI)
  // with BMI = W / H^2.

  void LBM_Male_KnownInputs()
  {
    constexpr double w = 75.0;       // kg
    constexpr double h = 1.78;       // m
    const double bmi   = w / (h * h);
    const double expectedKg = (9270.0 * w) / (6680.0 + 216.0 * bmi);
    const double expected   = expectedKg * 1000.0;

    mitk::SUVNormalizationInputs inputs;
    inputs.bodyWeightKg = w;
    inputs.heightM      = h;
    inputs.sex          = mitk::Sex::Male;

    mitk::LeanBodyMassStrategy strategy;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(expected, strategy.ComputeScaleNumerator(inputs),
                                 expected * 1e-12);
  }

  void LBM_Female_KnownInputs()
  {
    constexpr double w = 60.0;
    constexpr double h = 1.65;
    const double bmi   = w / (h * h);
    const double expectedKg = (9270.0 * w) / (8780.0 + 244.0 * bmi);
    const double expected   = expectedKg * 1000.0;

    mitk::SUVNormalizationInputs inputs;
    inputs.bodyWeightKg = w;
    inputs.heightM      = h;
    inputs.sex          = mitk::Sex::Female;

    mitk::LeanBodyMassStrategy strategy;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(expected, strategy.ComputeScaleNumerator(inputs),
                                 expected * 1e-12);
  }

  void LBM_VariantId()
  {
    mitk::LeanBodyMassStrategy strategy;
    CPPUNIT_ASSERT_EQUAL(mitk::SUVVariant::LBM, strategy.Variant());
  }

  void LBM_MissingHeight_Throws()
  {
    mitk::SUVNormalizationInputs inputs;
    inputs.bodyWeightKg = 70.0;
    inputs.sex          = mitk::Sex::Male;
    mitk::LeanBodyMassStrategy strategy;
    CPPUNIT_ASSERT_THROW(strategy.ComputeScaleNumerator(inputs),
                         mitk::MissingSUVInputException);
  }

  void LBM_MissingSex_Throws()
  {
    mitk::SUVNormalizationInputs inputs;
    inputs.bodyWeightKg = 70.0;
    inputs.heightM      = 1.78;
    // sex left as empty optional
    mitk::LeanBodyMassStrategy strategy;
    CPPUNIT_ASSERT_THROW(strategy.ComputeScaleNumerator(inputs),
                         mitk::MissingSUVInputException);
  }

  void LBM_MissingWeight_Throws()
  {
    mitk::SUVNormalizationInputs inputs;
    inputs.heightM = 1.78;
    inputs.sex     = mitk::Sex::Female;
    mitk::LeanBodyMassStrategy strategy;
    CPPUNIT_ASSERT_THROW(strategy.ComputeScaleNumerator(inputs),
                         mitk::MissingSUVInputException);
  }

  // ---- Body surface area (DuBois) ----
  //
  // BSA [m^2] = 0.007184 * W^0.425 * H_cm^0.725
  // Returned as cm^2 (m^2 * 10000).

  void BSA_KnownInputs()
  {
    constexpr double w  = 70.0;
    constexpr double h  = 1.75;        // m
    const double hCm    = h * 100.0;
    const double bsaM2  = 0.007184 * std::pow(w, 0.425) * std::pow(hCm, 0.725);
    const double expected = bsaM2 * 10000.0;

    mitk::SUVNormalizationInputs inputs;
    inputs.bodyWeightKg = w;
    inputs.heightM      = h;

    mitk::BodySurfaceAreaStrategy strategy;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(expected, strategy.ComputeScaleNumerator(inputs),
                                 expected * 1e-12);
  }

  void BSA_VariantId()
  {
    mitk::BodySurfaceAreaStrategy strategy;
    CPPUNIT_ASSERT_EQUAL(mitk::SUVVariant::BSA, strategy.Variant());
  }

  void BSA_MissingHeight_Throws()
  {
    mitk::SUVNormalizationInputs inputs;
    inputs.bodyWeightKg = 70.0;
    mitk::BodySurfaceAreaStrategy strategy;
    CPPUNIT_ASSERT_THROW(strategy.ComputeScaleNumerator(inputs),
                         mitk::MissingSUVInputException);
  }

  void BSA_MissingWeight_Throws()
  {
    mitk::SUVNormalizationInputs inputs;
    inputs.heightM = 1.75;
    mitk::BodySurfaceAreaStrategy strategy;
    CPPUNIT_ASSERT_THROW(strategy.ComputeScaleNumerator(inputs),
                         mitk::MissingSUVInputException);
  }

  // ---- Factory ----

  void Factory_BW_ReturnsBodyWeightStrategy()
  {
    auto s = mitk::MakeSUVNormalizationStrategy(mitk::SUVVariant::BW);
    CPPUNIT_ASSERT(s != nullptr);
    CPPUNIT_ASSERT_EQUAL(mitk::SUVVariant::BW, s->Variant());
  }

  void Factory_LBM_ReturnsLeanBodyMassStrategy()
  {
    auto s = mitk::MakeSUVNormalizationStrategy(mitk::SUVVariant::LBM);
    CPPUNIT_ASSERT(s != nullptr);
    CPPUNIT_ASSERT_EQUAL(mitk::SUVVariant::LBM, s->Variant());
  }

  void Factory_BSA_ReturnsBodySurfaceAreaStrategy()
  {
    auto s = mitk::MakeSUVNormalizationStrategy(mitk::SUVVariant::BSA);
    CPPUNIT_ASSERT(s != nullptr);
    CPPUNIT_ASSERT_EQUAL(mitk::SUVVariant::BSA, s->Variant());
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkSUVNormalizationStrategy)
