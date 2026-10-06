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
  MITK_TEST(LBMJanma_Male_KnownInputs);
  MITK_TEST(LBMJanma_Female_KnownInputs);
  MITK_TEST(LBMJanma_Other_Throws);
  MITK_TEST(LBMJanma_VariantId);
  MITK_TEST(LBMJanma_MissingHeight_Throws);
  MITK_TEST(LBMJanma_MissingSex_Throws);
  MITK_TEST(LBMJanma_MissingWeight_Throws);

  // Lean body mass (James 128)
  MITK_TEST(LBMJames128_Male_KnownInputs);
  MITK_TEST(LBMJames128_Female_KnownInputs);
  MITK_TEST(LBMJames128_Other_Throws);
  MITK_TEST(LBMJames128_VariantId);
  MITK_TEST(LBMMorgan_Male_KnownInputs);
  MITK_TEST(LBMMorgan_Female_KnownInputs);
  MITK_TEST(LBMMorgan_DiffersFromJames128_ForMalesOnly);
  MITK_TEST(LBMMorgan_Other_Throws);
  MITK_TEST(LBMMorgan_VariantId);
  MITK_TEST(LBMMorgan_RequiresHeightAndSex);

  // Ideal body weight (Sugawara)
  MITK_TEST(IBW_Male_KnownInputs);
  MITK_TEST(IBW_Female_KnownInputs);
  MITK_TEST(IBW_Other_Throws);
  MITK_TEST(IBW_VariantId);
  MITK_TEST(IBW_MissingHeight_Throws);

  // Body surface area (DuBois)
  MITK_TEST(BSA_KnownInputs);
  MITK_TEST(BSA_VariantId);
  MITK_TEST(BSA_MissingHeight_Throws);
  MITK_TEST(BSA_MissingWeight_Throws);

  // Factory
  MITK_TEST(Factory_BW_ReturnsBodyWeightStrategy);
  MITK_TEST(Factory_LBMJanma_ReturnsCorrectStrategy);
  MITK_TEST(Factory_LBMJames128_ReturnsCorrectStrategy);
  MITK_TEST(Factory_LBMMorgan_ReturnsCorrectStrategy);
  MITK_TEST(Factory_IBW_ReturnsCorrectStrategy);
  MITK_TEST(Factory_BSA_ReturnsBodySurfaceAreaStrategy);

  // Variant-requires queries (shared by SUVImageFilter and the GUI plugin)
  MITK_TEST(VariantRequiresPatientHeight_AllVariants);
  MITK_TEST(VariantRequiresPatientSex_AllVariants);

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

  void LBMJanma_Male_KnownInputs()
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

    mitk::LeanBodyMassJanmahasatianStrategy strategy;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(expected, strategy.ComputeScaleNumerator(inputs),
                                 expected * 1e-12);
    // Independent hard-coded anchor: a coefficient typo mirrored into the
    // formula above would slip past the recomputed check but not this one.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(58954.51, strategy.ComputeScaleNumerator(inputs), 0.1);
  }

  void LBMJanma_Female_KnownInputs()
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

    mitk::LeanBodyMassJanmahasatianStrategy strategy;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(expected, strategy.ComputeScaleNumerator(inputs),
                                 expected * 1e-12);
  }

  void LBMJanma_Other_Throws()
  {
    // Sex-closed contract: Sex::Other must be resolved upstream
    // (e.g. by SUVImageFilter under DICOMReadPolicy). The strategy
    // itself rejects it.
    mitk::SUVNormalizationInputs inputs;
    inputs.bodyWeightKg = 70.0;
    inputs.heightM      = 1.70;
    inputs.sex          = mitk::Sex::Other;

    mitk::LeanBodyMassJanmahasatianStrategy strategy;
    CPPUNIT_ASSERT_THROW(strategy.ComputeScaleNumerator(inputs),
                         mitk::MissingSUVInputException);
  }

  void LBMJanma_VariantId()
  {
    mitk::LeanBodyMassJanmahasatianStrategy strategy;
    CPPUNIT_ASSERT_EQUAL(mitk::SUVVariant::LBM_Janmahasatian, strategy.Variant());
  }

  void LBMJanma_MissingHeight_Throws()
  {
    mitk::SUVNormalizationInputs inputs;
    inputs.bodyWeightKg = 70.0;
    inputs.sex          = mitk::Sex::Male;
    mitk::LeanBodyMassJanmahasatianStrategy strategy;
    CPPUNIT_ASSERT_THROW(strategy.ComputeScaleNumerator(inputs),
                         mitk::MissingSUVInputException);
  }

  void LBMJanma_MissingSex_Throws()
  {
    mitk::SUVNormalizationInputs inputs;
    inputs.bodyWeightKg = 70.0;
    inputs.heightM      = 1.78;
    mitk::LeanBodyMassJanmahasatianStrategy strategy;
    CPPUNIT_ASSERT_THROW(strategy.ComputeScaleNumerator(inputs),
                         mitk::MissingSUVInputException);
  }

  void LBMJanma_MissingWeight_Throws()
  {
    mitk::SUVNormalizationInputs inputs;
    inputs.heightM = 1.78;
    inputs.sex     = mitk::Sex::Female;
    mitk::LeanBodyMassJanmahasatianStrategy strategy;
    CPPUNIT_ASSERT_THROW(strategy.ComputeScaleNumerator(inputs),
                         mitk::MissingSUVInputException);
  }

  // ---- Lean body mass (James 128) ----
  //
  //   LBM_male   [kg] = 1.10 * W - 0.0128 * W^2 / H^2
  //   LBM_female [kg] = 1.07 * W - 0.0148 * W^2 / H^2
  // with W in kg and H in m.

  void LBMJames128_Male_KnownInputs()
  {
    constexpr double w  = 75.0;
    constexpr double h  = 1.78;
    const double w2     = w * w;
    const double h2     = h * h;
    const double expectedKg = 1.10 * w - 0.0128 * w2 / h2;
    const double expected   = expectedKg * 1000.0;

    mitk::SUVNormalizationInputs inputs;
    inputs.bodyWeightKg = w;
    inputs.heightM      = h;
    inputs.sex          = mitk::Sex::Male;

    mitk::LeanBodyMassJames128Strategy strategy;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(expected, strategy.ComputeScaleNumerator(inputs),
                                 expected * 1e-12);
    // Independent hard-coded anchor (see LBMJanma_Male_KnownInputs).
    CPPUNIT_ASSERT_DOUBLES_EQUAL(59775.60, strategy.ComputeScaleNumerator(inputs), 0.1);
  }

  void LBMJames128_Female_KnownInputs()
  {
    constexpr double w  = 60.0;
    constexpr double h  = 1.65;
    const double w2     = w * w;
    const double h2     = h * h;
    const double expectedKg = 1.07 * w - 0.0148 * w2 / h2;
    const double expected   = expectedKg * 1000.0;

    mitk::SUVNormalizationInputs inputs;
    inputs.bodyWeightKg = w;
    inputs.heightM      = h;
    inputs.sex          = mitk::Sex::Female;

    mitk::LeanBodyMassJames128Strategy strategy;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(expected, strategy.ComputeScaleNumerator(inputs),
                                 expected * 1e-12);
  }

  void LBMJames128_Other_Throws()
  {
    mitk::SUVNormalizationInputs inputs;
    inputs.bodyWeightKg = 70.0;
    inputs.heightM      = 1.70;
    inputs.sex          = mitk::Sex::Other;

    mitk::LeanBodyMassJames128Strategy strategy;
    CPPUNIT_ASSERT_THROW(strategy.ComputeScaleNumerator(inputs),
                         mitk::MissingSUVInputException);
  }

  void LBMJames128_VariantId()
  {
    mitk::LeanBodyMassJames128Strategy strategy;
    CPPUNIT_ASSERT_EQUAL(mitk::SUVVariant::LBM_James128, strategy.Variant());
  }

  // ---- Lean body mass, Morgan (DICOM SUV Type "LBM") ----
  //
  // Morgan and James-128 differ in one coefficient, for males only
  // (0.0120 against 0.0128); their female formulas are identical. So the
  // male cases below are the only ones that can tell the two apart, and
  // the female case is deliberately written to assert that they agree --
  // an equality that would otherwise look like a copy-paste error to the
  // next reader.

  void LBMMorgan_Male_KnownInputs()
  {
    constexpr double w  = 75.0;
    constexpr double h  = 1.78;
    const double w2     = w * w;
    const double h2     = h * h;
    const double expectedKg = 1.10 * w - 0.0120 * w2 / h2;
    const double expected   = expectedKg * 1000.0;

    mitk::SUVNormalizationInputs inputs;
    inputs.bodyWeightKg = w;
    inputs.heightM      = h;
    inputs.sex          = mitk::Sex::Male;

    mitk::LeanBodyMassMorganStrategy strategy;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(expected, strategy.ComputeScaleNumerator(inputs),
                                 expected * 1e-12);
    // Independent hard-coded anchor (see LBMJanma_Male_KnownInputs).
    CPPUNIT_ASSERT_DOUBLES_EQUAL(61195.87, strategy.ComputeScaleNumerator(inputs), 0.1);
  }

  void LBMMorgan_Female_KnownInputs()
  {
    constexpr double w  = 60.0;
    constexpr double h  = 1.65;
    const double w2     = w * w;
    const double h2     = h * h;
    const double expectedKg = 1.07 * w - 0.0148 * w2 / h2;
    const double expected   = expectedKg * 1000.0;

    mitk::SUVNormalizationInputs inputs;
    inputs.bodyWeightKg = w;
    inputs.heightM      = h;
    inputs.sex          = mitk::Sex::Female;

    mitk::LeanBodyMassMorganStrategy strategy;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(expected, strategy.ComputeScaleNumerator(inputs),
                                 expected * 1e-12);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(44629.75, strategy.ComputeScaleNumerator(inputs), 0.1);
  }

  void LBMMorgan_DiffersFromJames128_ForMalesOnly()
  {
    // The whole risk in adding Morgan is that it gets confused with
    // James-128. Pin the difference where it exists and the equality where
    // it does not, so a future edit to either formula cannot quietly turn
    // one into the other.
    mitk::LeanBodyMassMorganStrategy   morgan;
    mitk::LeanBodyMassJames128Strategy james;

    mitk::SUVNormalizationInputs male;
    male.bodyWeightKg = 75.0;
    male.heightM      = 1.78;
    male.sex          = mitk::Sex::Male;

    const double morganMale = morgan.ComputeScaleNumerator(male);
    const double jamesMale  = james.ComputeScaleNumerator(male);
    CPPUNIT_ASSERT_MESSAGE("Morgan and James-128 must differ for male patients.",
                           std::abs(morganMale - jamesMale) > 1.0);
    // 0.0128 - 0.0120 = 0.0008, times W^2 / H^2, times 1000 g/kg.
    const double expectedDelta = 0.0008 * (75.0 * 75.0) / (1.78 * 1.78) * 1000.0;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(expectedDelta, morganMale - jamesMale, 1e-6);

    mitk::SUVNormalizationInputs female;
    female.bodyWeightKg = 60.0;
    female.heightM      = 1.65;
    female.sex          = mitk::Sex::Female;

    CPPUNIT_ASSERT_DOUBLES_EQUAL(james.ComputeScaleNumerator(female),
                                 morgan.ComputeScaleNumerator(female), 1e-9);
  }

  void LBMMorgan_Other_Throws()
  {
    mitk::SUVNormalizationInputs inputs;
    inputs.bodyWeightKg = 70.0;
    inputs.heightM      = 1.70;
    inputs.sex          = mitk::Sex::Other;

    mitk::LeanBodyMassMorganStrategy strategy;
    CPPUNIT_ASSERT_THROW(strategy.ComputeScaleNumerator(inputs),
                         mitk::MissingSUVInputException);
  }

  void LBMMorgan_VariantId()
  {
    mitk::LeanBodyMassMorganStrategy strategy;
    CPPUNIT_ASSERT_EQUAL(mitk::SUVVariant::LBM_Morgan, strategy.Variant());
  }

  void Factory_LBMMorgan_ReturnsCorrectStrategy()
  {
    const auto strategy = mitk::MakeSUVNormalizationStrategy(mitk::SUVVariant::LBM_Morgan);
    CPPUNIT_ASSERT(nullptr != strategy);
    CPPUNIT_ASSERT_EQUAL(mitk::SUVVariant::LBM_Morgan, strategy->Variant());
  }

  void LBMMorgan_RequiresHeightAndSex()
  {
    CPPUNIT_ASSERT(mitk::VariantRequiresPatientHeight(mitk::SUVVariant::LBM_Morgan));
    CPPUNIT_ASSERT(mitk::VariantRequiresPatientSex(mitk::SUVVariant::LBM_Morgan));
  }

  // ---- Ideal body weight (IBSI-SUV) ----
  //
  //   IBW_male   [kg] = 48.0 + 1.06 * (H_cm - 152)
  //   IBW_female [kg] = 45.5 + 0.91 * (H_cm - 152)
  // with H in cm.

  void IBW_Male_KnownInputs()
  {
    constexpr double h = 1.78;          // m
    const double hCm   = h * 100.0;
    const double expectedKg = 48.0 + 1.06 * (hCm - 152.0);
    const double expected   = expectedKg * 1000.0;

    mitk::SUVNormalizationInputs inputs;
    inputs.heightM = h;
    inputs.sex     = mitk::Sex::Male;

    mitk::IdealBodyWeightStrategy strategy;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(expected, strategy.ComputeScaleNumerator(inputs),
                                 expected * 1e-12);
    // Independent hard-coded anchor (see LBMJanma_Male_KnownInputs).
    CPPUNIT_ASSERT_DOUBLES_EQUAL(75560.0, strategy.ComputeScaleNumerator(inputs), 0.1);
  }

  void IBW_Female_KnownInputs()
  {
    constexpr double h = 1.65;
    const double hCm   = h * 100.0;
    const double expectedKg = 45.5 + 0.91 * (hCm - 152.0);
    const double expected   = expectedKg * 1000.0;

    mitk::SUVNormalizationInputs inputs;
    inputs.heightM = h;
    inputs.sex     = mitk::Sex::Female;

    mitk::IdealBodyWeightStrategy strategy;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(expected, strategy.ComputeScaleNumerator(inputs),
                                 expected * 1e-12);
  }

  void IBW_Other_Throws()
  {
    mitk::SUVNormalizationInputs inputs;
    inputs.heightM = 1.70;
    inputs.sex     = mitk::Sex::Other;

    mitk::IdealBodyWeightStrategy strategy;
    CPPUNIT_ASSERT_THROW(strategy.ComputeScaleNumerator(inputs),
                         mitk::MissingSUVInputException);
  }

  void IBW_VariantId()
  {
    mitk::IdealBodyWeightStrategy strategy;
    CPPUNIT_ASSERT_EQUAL(mitk::SUVVariant::IBW, strategy.Variant());
  }

  void IBW_MissingHeight_Throws()
  {
    mitk::SUVNormalizationInputs inputs;
    inputs.sex = mitk::Sex::Male;
    mitk::IdealBodyWeightStrategy strategy;
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
    // Independent hard-coded anchor (see LBMJanma_Male_KnownInputs).
    CPPUNIT_ASSERT_DOUBLES_EQUAL(18481.43, strategy.ComputeScaleNumerator(inputs), 0.1);
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

  void Factory_LBMJanma_ReturnsCorrectStrategy()
  {
    auto s = mitk::MakeSUVNormalizationStrategy(mitk::SUVVariant::LBM_Janmahasatian);
    CPPUNIT_ASSERT(s != nullptr);
    CPPUNIT_ASSERT_EQUAL(mitk::SUVVariant::LBM_Janmahasatian, s->Variant());
  }

  void Factory_LBMJames128_ReturnsCorrectStrategy()
  {
    auto s = mitk::MakeSUVNormalizationStrategy(mitk::SUVVariant::LBM_James128);
    CPPUNIT_ASSERT(s != nullptr);
    CPPUNIT_ASSERT_EQUAL(mitk::SUVVariant::LBM_James128, s->Variant());
  }

  void Factory_IBW_ReturnsCorrectStrategy()
  {
    auto s = mitk::MakeSUVNormalizationStrategy(mitk::SUVVariant::IBW);
    CPPUNIT_ASSERT(s != nullptr);
    CPPUNIT_ASSERT_EQUAL(mitk::SUVVariant::IBW, s->Variant());
  }

  void Factory_BSA_ReturnsBodySurfaceAreaStrategy()
  {
    auto s = mitk::MakeSUVNormalizationStrategy(mitk::SUVVariant::BSA);
    CPPUNIT_ASSERT(s != nullptr);
    CPPUNIT_ASSERT_EQUAL(mitk::SUVVariant::BSA, s->Variant());
  }

  // ---- Variant-requires queries ----

  void VariantRequiresPatientHeight_AllVariants()
  {
    CPPUNIT_ASSERT_EQUAL(false, mitk::VariantRequiresPatientHeight(mitk::SUVVariant::BW));
    CPPUNIT_ASSERT_EQUAL(true,  mitk::VariantRequiresPatientHeight(mitk::SUVVariant::LBM_Janmahasatian));
    CPPUNIT_ASSERT_EQUAL(true,  mitk::VariantRequiresPatientHeight(mitk::SUVVariant::LBM_James128));
    CPPUNIT_ASSERT_EQUAL(true,  mitk::VariantRequiresPatientHeight(mitk::SUVVariant::IBW));
    CPPUNIT_ASSERT_EQUAL(true,  mitk::VariantRequiresPatientHeight(mitk::SUVVariant::BSA));
  }

  void VariantRequiresPatientSex_AllVariants()
  {
    CPPUNIT_ASSERT_EQUAL(false, mitk::VariantRequiresPatientSex(mitk::SUVVariant::BW));
    CPPUNIT_ASSERT_EQUAL(true,  mitk::VariantRequiresPatientSex(mitk::SUVVariant::LBM_Janmahasatian));
    CPPUNIT_ASSERT_EQUAL(true,  mitk::VariantRequiresPatientSex(mitk::SUVVariant::LBM_James128));
    CPPUNIT_ASSERT_EQUAL(true,  mitk::VariantRequiresPatientSex(mitk::SUVVariant::IBW));
    CPPUNIT_ASSERT_EQUAL(false, mitk::VariantRequiresPatientSex(mitk::SUVVariant::BSA));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkSUVNormalizationStrategy)
