/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkImage.h>
#include <mitkPixelType.h>

#include <mitkSUVCalculationHelper.h>
#include <mitkSUVImageFilter.h>
#include <mitkSUVInputModel.h>
#include <mitkSUVNormalizationStrategy.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

class mitkSUVImageFilterTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkSUVImageFilterTestSuite);

  // Defaults
  MITK_TEST(Defaults_TargetVariantBW_PolicyLenient);
  MITK_TEST(Defaults_AllOverridesEmpty);

  // Set / Get / Clear round-trip per override field
  MITK_TEST(SetClear_PatientWeightInGram);
  MITK_TEST(SetClear_PatientHeightInCm);
  MITK_TEST(SetClear_PatientSex);
  MITK_TEST(SetClear_InjectedActivityInBq);
  MITK_TEST(SetClear_HalfLifeInSec);
  MITK_TEST(SetClear_DecayTimeOverrideInSec);
  MITK_TEST(SetClear_TracerIndex);
  MITK_TEST(SetClear_InputModelOverride);

  // GetEffective* precondition: throws before Configure has succeeded
  MITK_TEST(GetEffectivePatientWeight_BeforeConfigure_Throws);
  MITK_TEST(GetEffectivePatientHeight_BeforeConfigure_Throws);
  MITK_TEST(GetEffectivePatientSex_BeforeConfigure_Throws);
  MITK_TEST(GetEffectiveInjectedActivity_BeforeConfigure_Throws);
  MITK_TEST(GetEffectiveHalfLife_BeforeConfigure_Throws);
  MITK_TEST(GetEffectiveDecayCorrection_BeforeConfigure_Throws);
  MITK_TEST(GetEffectiveInputModel_BeforeConfigure_Throws);

  // ConfigureFromProperties precondition
  MITK_TEST(ConfigureFromProperties_NullProvider_Throws);
  MITK_TEST(ConfigureFromProperties_NoInput_Throws);

  // Ambiguous-sex policy gate (Sex::Other and unset effective sex)
  MITK_TEST(SexPolicyGate_StrictWithSexOther_Throws);
  MITK_TEST(SexPolicyGate_LenientWithSexOther_Succeeds);
  MITK_TEST(SexPolicyGate_StrictWithUnsetEffectiveSex_Throws);
  MITK_TEST(SexPolicyGate_LenientWithUnsetEffectiveSex_Succeeds);

  CPPUNIT_TEST_SUITE_END();

public:

  // ---- Defaults ----

  void Defaults_TargetVariantBW_PolicyLenient()
  {
    auto f = mitk::SUVImageFilter::New();
    CPPUNIT_ASSERT_EQUAL(mitk::SUVVariant::BW, f->GetTargetVariant());
    CPPUNIT_ASSERT(mitk::DICOMReadPolicy::Lenient == f->GetDICOMReadPolicy());
  }

  void Defaults_AllOverridesEmpty()
  {
    auto f = mitk::SUVImageFilter::New();
    CPPUNIT_ASSERT(!f->GetPatientWeightInGram().has_value());
    CPPUNIT_ASSERT(!f->GetPatientHeightInCm().has_value());
    CPPUNIT_ASSERT(!f->GetPatientSex().has_value());
    CPPUNIT_ASSERT(!f->GetInjectedActivityInBq().has_value());
    CPPUNIT_ASSERT(!f->GetHalfLifeInSec().has_value());
    CPPUNIT_ASSERT(!f->GetDecayTimeOverrideInSec().has_value());
    CPPUNIT_ASSERT(!f->GetTracerIndex().has_value());
    CPPUNIT_ASSERT(!f->GetInputModelOverride().has_value());
  }

  // ---- Set / Get / Clear round-trip ----

  void SetClear_PatientWeightInGram()
  {
    auto f = mitk::SUVImageFilter::New();
    f->SetPatientWeightInGram(70000.0);
    CPPUNIT_ASSERT(f->GetPatientWeightInGram().has_value());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(70000.0, f->GetPatientWeightInGram().value(), 1e-9);
    f->ClearPatientWeightInGram();
    CPPUNIT_ASSERT(!f->GetPatientWeightInGram().has_value());
  }

  void SetClear_PatientHeightInCm()
  {
    auto f = mitk::SUVImageFilter::New();
    f->SetPatientHeightInCm(178.0);
    CPPUNIT_ASSERT(f->GetPatientHeightInCm().has_value());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(178.0, f->GetPatientHeightInCm().value(), 1e-9);
    f->ClearPatientHeightInCm();
    CPPUNIT_ASSERT(!f->GetPatientHeightInCm().has_value());
  }

  void SetClear_PatientSex()
  {
    auto f = mitk::SUVImageFilter::New();
    f->SetPatientSex(mitk::Sex::Female);
    CPPUNIT_ASSERT(f->GetPatientSex().has_value());
    CPPUNIT_ASSERT(mitk::Sex::Female == f->GetPatientSex().value());
    f->ClearPatientSex();
    CPPUNIT_ASSERT(!f->GetPatientSex().has_value());
  }

  void SetClear_InjectedActivityInBq()
  {
    auto f = mitk::SUVImageFilter::New();
    f->SetInjectedActivityInBq(3.5e8);
    CPPUNIT_ASSERT(f->GetInjectedActivityInBq().has_value());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.5e8, f->GetInjectedActivityInBq().value(), 1e-3);
    f->ClearInjectedActivityInBq();
    CPPUNIT_ASSERT(!f->GetInjectedActivityInBq().has_value());
  }

  void SetClear_HalfLifeInSec()
  {
    auto f = mitk::SUVImageFilter::New();
    f->SetHalfLifeInSec(6586.2);
    CPPUNIT_ASSERT(f->GetHalfLifeInSec().has_value());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6586.2, f->GetHalfLifeInSec().value(), 1e-9);
    f->ClearHalfLifeInSec();
    CPPUNIT_ASSERT(!f->GetHalfLifeInSec().has_value());
  }

  void SetClear_DecayTimeOverrideInSec()
  {
    auto f = mitk::SUVImageFilter::New();
    f->SetDecayTimeOverrideInSec(3600.0);
    CPPUNIT_ASSERT(f->GetDecayTimeOverrideInSec().has_value());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3600.0, f->GetDecayTimeOverrideInSec().value(), 1e-9);
    f->ClearDecayTimeOverrideInSec();
    CPPUNIT_ASSERT(!f->GetDecayTimeOverrideInSec().has_value());
  }

  void SetClear_TracerIndex()
  {
    auto f = mitk::SUVImageFilter::New();
    f->SetTracerIndex(2);
    CPPUNIT_ASSERT(f->GetTracerIndex().has_value());
    CPPUNIT_ASSERT_EQUAL(2, f->GetTracerIndex().value());
    f->ClearTracerIndex();
    CPPUNIT_ASSERT(!f->GetTracerIndex().has_value());
  }

  void SetClear_InputModelOverride()
  {
    auto f = mitk::SUVImageFilter::New();
    mitk::SUVInputModel m;
    m.semantics    = mitk::SUVPixelSemantics::ActivityConcentration;
    m.activityScale = 1.0;
    f->SetInputModelOverride(m);
    CPPUNIT_ASSERT(f->GetInputModelOverride().has_value());
    CPPUNIT_ASSERT(mitk::SUVPixelSemantics::ActivityConcentration ==
                   f->GetInputModelOverride().value().semantics);
    f->ClearInputModelOverride();
    CPPUNIT_ASSERT(!f->GetInputModelOverride().has_value());
  }

  // ---- GetEffective* precondition ----

  void GetEffectivePatientWeight_BeforeConfigure_Throws()
  {
    auto f = mitk::SUVImageFilter::New();
    CPPUNIT_ASSERT_THROW(f->GetEffectivePatientWeightInGram(), mitk::Exception);
  }

  void GetEffectivePatientHeight_BeforeConfigure_Throws()
  {
    auto f = mitk::SUVImageFilter::New();
    CPPUNIT_ASSERT_THROW(f->GetEffectivePatientHeightInCm(), mitk::Exception);
  }

  void GetEffectivePatientSex_BeforeConfigure_Throws()
  {
    auto f = mitk::SUVImageFilter::New();
    CPPUNIT_ASSERT_THROW(f->GetEffectivePatientSex(), mitk::Exception);
  }

  void GetEffectiveInjectedActivity_BeforeConfigure_Throws()
  {
    auto f = mitk::SUVImageFilter::New();
    CPPUNIT_ASSERT_THROW(f->GetEffectiveInjectedActivityInBq(), mitk::Exception);
  }

  void GetEffectiveHalfLife_BeforeConfigure_Throws()
  {
    auto f = mitk::SUVImageFilter::New();
    CPPUNIT_ASSERT_THROW(f->GetEffectiveHalfLifeInSec(), mitk::Exception);
  }

  void GetEffectiveDecayCorrection_BeforeConfigure_Throws()
  {
    auto f = mitk::SUVImageFilter::New();
    CPPUNIT_ASSERT_THROW(f->GetEffectiveDecayCorrection(), mitk::Exception);
  }

  void GetEffectiveInputModel_BeforeConfigure_Throws()
  {
    auto f = mitk::SUVImageFilter::New();
    CPPUNIT_ASSERT_THROW(f->GetEffectiveInputModel(), mitk::Exception);
  }

  // ---- ConfigureFromProperties preconditions ----

  void ConfigureFromProperties_NullProvider_Throws()
  {
    auto f = mitk::SUVImageFilter::New();
    CPPUNIT_ASSERT_THROW(f->ConfigureFromProperties(nullptr), mitk::Exception);
  }

  void ConfigureFromProperties_NoInput_Throws()
  {
    auto f = mitk::SUVImageFilter::New();
    auto img = mitk::Image::New();
    const auto pixelType = mitk::MakeScalarPixelType<float>();
    const unsigned int dims[3] = { 1u, 1u, 1u };
    img->Initialize(pixelType, 3, dims);
    // Provide a property provider but no input image set on the filter.
    CPPUNIT_ASSERT_THROW(f->ConfigureFromProperties(img.GetPointer()), mitk::Exception);
  }

  // ---- Ambiguous-sex policy gate ----
  //
  // The policy gate fires for sex-specific target variants when the
  // effective patient sex is ambiguous: either explicitly Sex::Other or
  // unset (no override and the upstream resolution did not populate it).
  // Strict refuses with AmbiguousPatientSexAdaptationRefusedException;
  // Lenient applies the IBSI mean-of-M-and-F adaptation.
  //
  // The "unset effective sex" branch is reached via a variant-switch:
  // configure with a sex-independent target (BSA) so resolution leaves
  // m_EffectivePatientSex empty, then switch to a sex-specific target
  // before Update. The renormalization input model lets us bypass the
  // activity / decay path so the test does not need DICOM fixtures.

private:
  static mitk::Image::Pointer MakeMinimalImage()
  {
    auto img = mitk::Image::New();
    const auto pixelType = mitk::MakeScalarPixelType<float>();
    const unsigned int dims[3] = { 1u, 1u, 1u };
    img->Initialize(pixelType, 3, dims);
    const float zero = 0.0f;
    img->SetVolume(&zero);
    return img;
  }

  static mitk::SUVInputModel MakePrenormalizedBwInputModel()
  {
    mitk::SUVInputModel m;
    m.semantics      = mitk::SUVPixelSemantics::PrenormalizedSUV;
    m.activityScale  = 1.0;
    m.sourceVariant  = mitk::SUVVariant::BW;
    m.prenormScale   = 1.0;
    return m;
  }

  // Build a filter with target=LBM_Janma, sex override = explicitSex,
  // configured against an image with no DICOM properties (overrides
  // cover everything Configure needs).
  static mitk::SUVImageFilter::Pointer MakeFilterWithExplicitSex(
    mitk::Sex explicitSex, mitk::DICOMReadPolicy policy)
  {
    auto f   = mitk::SUVImageFilter::New();
    auto img = MakeMinimalImage();
    f->SetInput(img);
    f->SetTargetVariant(mitk::SUVVariant::LBM_Janmahasatian);
    f->SetDICOMReadPolicy(policy);
    f->SetPatientWeightInGram(70000.0);
    f->SetPatientHeightInCm(170.0);
    f->SetPatientSex(explicitSex);
    f->SetInputModelOverride(MakePrenormalizedBwInputModel());
    f->ConfigureFromProperties(img.GetPointer());
    return f;
  }

  // Build a filter whose effective sex is left empty even though the
  // active target variant is sex-specific. Configures with target=BSA
  // (sex-independent) so Configure does not resolve sex, then switches
  // to LBM_Janma before Update. Tests of the policy gate's defensive
  // behavior in this state.
  static mitk::SUVImageFilter::Pointer MakeFilterWithUnsetEffectiveSex(
    mitk::DICOMReadPolicy policy)
  {
    auto f   = mitk::SUVImageFilter::New();
    auto img = MakeMinimalImage();
    f->SetInput(img);
    f->SetTargetVariant(mitk::SUVVariant::BSA);
    f->SetDICOMReadPolicy(policy);
    f->SetPatientWeightInGram(70000.0);
    f->SetPatientHeightInCm(170.0);
    f->SetInputModelOverride(MakePrenormalizedBwInputModel());
    f->ConfigureFromProperties(img.GetPointer());
    f->SetTargetVariant(mitk::SUVVariant::LBM_Janmahasatian);
    return f;
  }

public:

  void SexPolicyGate_StrictWithSexOther_Throws()
  {
    auto f = MakeFilterWithExplicitSex(mitk::Sex::Other,
                                       mitk::DICOMReadPolicy::Strict);
    CPPUNIT_ASSERT_THROW(f->Update(),
                         mitk::AmbiguousPatientSexAdaptationRefusedException);
  }

  void SexPolicyGate_LenientWithSexOther_Succeeds()
  {
    auto f = MakeFilterWithExplicitSex(mitk::Sex::Other,
                                       mitk::DICOMReadPolicy::Lenient);
    CPPUNIT_ASSERT_NO_THROW(f->Update());
  }

  void SexPolicyGate_StrictWithUnsetEffectiveSex_Throws()
  {
    auto f = MakeFilterWithUnsetEffectiveSex(mitk::DICOMReadPolicy::Strict);
    CPPUNIT_ASSERT_THROW(f->Update(),
                         mitk::AmbiguousPatientSexAdaptationRefusedException);
  }

  void SexPolicyGate_LenientWithUnsetEffectiveSex_Succeeds()
  {
    auto f = MakeFilterWithUnsetEffectiveSex(mitk::DICOMReadPolicy::Lenient);
    CPPUNIT_ASSERT_NO_THROW(f->Update());
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkSUVImageFilter)
