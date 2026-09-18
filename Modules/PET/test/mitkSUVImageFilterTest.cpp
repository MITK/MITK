/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkDICOMProperty.h>
#include <mitkDICOMTagPath.h>
#include <mitkIOUtil.h>
#include <mitkImage.h>
#include <mitkPixelType.h>

#include <mitkSUVCalculationHelper.h>
#include <mitkSUVImageFilter.h>
#include <mitkSUVInputModel.h>
#include <mitkSUVNormalizationStrategy.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <cstdio>
#include <string>
#include <vector>

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

  // Ambiguous-sex policy gate (Sex::Other)
  MITK_TEST(SexPolicyGate_StrictWithSexOther_Throws);
  MITK_TEST(SexPolicyGate_LenientWithSexOther_Succeeds);

  // Reconfigure-on-change (M1): a target-variant change after configure
  // re-resolves at Update instead of reusing stale resolved state.
  MITK_TEST(TargetVariantChange_AfterConfigure_RequiresSex);

  // Output DICOM tags
  MITK_TEST(OutputTags_EachVariantWritesItsOwnSUVType);
  MITK_TEST(OutputTags_RoundTripThroughClassifier);
  MITK_TEST(GramEncodedWeight_YieldsSameEffectiveWeightAsKilograms);

  // Adaptation record on the output, and across a save
  MITK_TEST(AdaptationRecord_IsWrittenToTheOutput);
  MITK_TEST(AdaptationRecord_SurvivesSaveAndReload);
  MITK_TEST(AdaptationRecord_UnadaptedInput_RecordsEmptyAndStillDescribesTheDerivation);

  // Per-(timestep, slice) decay-time override map
  MITK_TEST(SetClear_DecayTimeOverrideMap);
  MITK_TEST(DecayMap_SetUniformThenMap_Throws);
  MITK_TEST(DecayMap_SetMapThenUniform_Throws);
  MITK_TEST(DecayMap_ConfigureWithCompleteMap_EffectiveStrategyIsManual);
  MITK_TEST(DecayMap_ConfigureWithCompleteMap_EffectiveDecayTimesMatch);
  MITK_TEST(DecayMap_ConfiguredWithoutUniform_MapValuesSurface);
  MITK_TEST(DecayMap_MissingSliceEntry_Throws);
  MITK_TEST(DecayMap_OutOfRangeSlice_Throws);
  MITK_TEST(DecayMap_OutOfRangeTimestep_Throws);

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

  // A filter over a one-voxel image whose Patient's Weight is gram-encoded,
  // which is the cheapest input that makes the pipeline record an adaptation
  // without needing a DRO.
  static mitk::SUVImageFilter::Pointer MakeFilterWithGramEncodedWeight()
  {
    auto img = MakeMinimalImage();
    const std::string key =
      mitk::DICOMTagPathToPropertyName(mitk::DICOMTagPath(0x0010, 0x1030));
    auto prop = mitk::DICOMProperty::New();
    prop->SetValue(0, 0, "70000");
    img->SetProperty(key.c_str(), prop);

    auto f = mitk::SUVImageFilter::New();
    f->SetInput(img);
    f->SetTargetVariant(mitk::SUVVariant::BW);
    f->SetDICOMReadPolicy(mitk::DICOMReadPolicy::Lenient);
    f->SetInputModelOverride(MakePrenormalizedBwInputModel());
    return f;
  }

  static std::string ReadStringProperty(const mitk::Image* image, const std::string& name)
  {
    auto prop = image->GetProperty(name.c_str());
    return prop.IsNull() ? std::string() : prop->GetValueAsString();
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

  // Build a filter configured for a sex-independent target (BSA), then
  // switched to a sex-specific target (LBM_Janma) with no explicit sex and
  // against an image carrying no sex tag. With the M1 reconfigure-on-change
  // guard, Update() re-resolves against the new variant and surfaces the
  // now-missing sex rather than silently reusing the BSA-time (sex-
  // unresolved) state.
  static mitk::SUVImageFilter::Pointer MakeFilterSwitchedToSexSpecificTarget(
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
    // Eager validation: the gate fires inside ConfigureFromProperties,
    // not only at Update time, so the plugin can disable the Calculate
    // button before the user clicks it.
    auto f   = mitk::SUVImageFilter::New();
    auto img = MakeMinimalImage();
    f->SetInput(img);
    f->SetTargetVariant(mitk::SUVVariant::LBM_Janmahasatian);
    f->SetDICOMReadPolicy(mitk::DICOMReadPolicy::Strict);
    f->SetPatientWeightInGram(70000.0);
    f->SetPatientHeightInCm(170.0);
    f->SetPatientSex(mitk::Sex::Other);
    f->SetInputModelOverride(MakePrenormalizedBwInputModel());
    CPPUNIT_ASSERT_THROW(f->ConfigureFromProperties(img.GetPointer()),
                         mitk::AmbiguousPatientSexAdaptationRefusedException);
  }

  void SexPolicyGate_LenientWithSexOther_Succeeds()
  {
    auto f = MakeFilterWithExplicitSex(mitk::Sex::Other,
                                       mitk::DICOMReadPolicy::Lenient);
    CPPUNIT_ASSERT_NO_THROW(f->Update());
  }

  void TargetVariantChange_AfterConfigure_RequiresSex()
  {
    // M1: a target-variant change after ConfigureFromProperties must not be
    // silently ignored at Update(). Switching from BSA (sex-independent) to
    // LBM (sex-specific) triggers a reconfigure that needs patient sex; the
    // minimal image carries none, so Update() throws. It throws for both
    // policies because the missing-sex error precedes the policy gate (so an
    // unset effective sex can no longer leak into the gate via stale config).
    {
      auto f = MakeFilterSwitchedToSexSpecificTarget(mitk::DICOMReadPolicy::Strict);
      CPPUNIT_ASSERT_THROW(f->Update(), mitk::MissingDICOMPropertyException);
    }
    {
      auto f = MakeFilterSwitchedToSexSpecificTarget(mitk::DICOMReadPolicy::Lenient);
      CPPUNIT_ASSERT_THROW(f->Update(), mitk::MissingDICOMPropertyException);
    }
  }

  // ---- Output DICOM tags ----
  //
  // ApplyOutputTagPolicy stamps (0054,1001) Units and (0054,1006) SUV Type
  // on every image the filter produces, so the output describes itself
  // rather than inheriting the input's now-stale tags. Nothing covered
  // those tags before, which is how they came to be written lossily: all
  // three lean-body-mass variants collapsed onto the generic "LBM".
  //
  // That matters because an SUV image is a legitimate input. Units = GML
  // plus a SUV Type is exactly the pre-normalized case DRO_2_1_x and
  // DRO_2_6_x exercise, so the classifier must be able to read back what
  // the filter wrote. These cases assert the full round trip: compute with
  // a variant, then classify the output and get the same variant.

  static std::string ReadOutputTag(const mitk::Image* image,
                                   unsigned int group, unsigned int element)
  {
    const std::string key = mitk::DICOMTagPathToPropertyName(
      mitk::DICOMTagPath(group, element));
    const auto prop = image->GetConstProperty(key.c_str());
    const auto* dicomProp = dynamic_cast<const mitk::DICOMProperty*>(prop.GetPointer());
    return (nullptr != dicomProp) ? dicomProp->GetValue(0, 0, true, true) : std::string();
  }

  static mitk::Image::Pointer ComputeWithTarget(mitk::SUVVariant target)
  {
    auto f   = mitk::SUVImageFilter::New();
    auto img = MakeMinimalImage();
    f->SetInput(img);
    f->SetTargetVariant(target);
    f->SetDICOMReadPolicy(mitk::DICOMReadPolicy::Lenient);
    f->SetPatientWeightInGram(70000.0);
    f->SetPatientHeightInCm(170.0);
    f->SetPatientSex(mitk::Sex::Male);
    f->SetInputModelOverride(MakePrenormalizedBwInputModel());
    f->Update();
    return f->GetOutput();
  }

  void OutputTags_EachVariantWritesItsOwnSUVType()
  {
    // The three lean-body-mass codes must stay distinct. DICOM defines one
    // per formula, and Morgan differs from James-128 by about 2 % of lean
    // body mass in males -- small enough that a collapsed code would be
    // read back as the wrong formula without anything looking wrong.
    const struct
    {
      mitk::SUVVariant variant;
      const char*      units;
      const char*      suvType;
    } expectations[] = {
      { mitk::SUVVariant::BW,                "GML",   "BW"          },
      { mitk::SUVVariant::LBM_Janmahasatian, "GML",   "LBMJANMA"    },
      { mitk::SUVVariant::LBM_James128,      "GML",   "LBMJAMES128" },
      { mitk::SUVVariant::LBM_Morgan,        "GML",   "LBM"         },
      { mitk::SUVVariant::IBW,               "GML",   "IBW"         },
      { mitk::SUVVariant::BSA,               "CM2ML", "BSA"         },
    };

    for (const auto& e : expectations)
    {
      const auto output = ComputeWithTarget(e.variant);
      CPPUNIT_ASSERT(output.IsNotNull());
      CPPUNIT_ASSERT_EQUAL(std::string(e.units),
                           ReadOutputTag(output, 0x0054, 0x1001));
      CPPUNIT_ASSERT_EQUAL(std::string(e.suvType),
                           ReadOutputTag(output, 0x0054, 0x1006));
    }
  }

  void OutputTags_RoundTripThroughClassifier()
  {
    // The assertion that actually matters: feed each output back through
    // the classifier and recover the variant it was computed with. A
    // missing enumerator in OutputSUVTypeValue would write no tag at all
    // (it returns nullptr and the setter skips), and a collapsed code
    // would resolve to the wrong formula; both fail here.
    const mitk::SUVVariant variants[] = {
      mitk::SUVVariant::BW,
      mitk::SUVVariant::LBM_Janmahasatian,
      mitk::SUVVariant::LBM_James128,
      mitk::SUVVariant::LBM_Morgan,
      mitk::SUVVariant::IBW,
      mitk::SUVVariant::BSA,
    };

    for (const auto variant : variants)
    {
      const auto output = ComputeWithTarget(variant);
      const auto model  = mitk::ClassifyPETInput(output.GetPointer(),
                                                 mitk::DICOMReadPolicy::Lenient);
      CPPUNIT_ASSERT(mitk::SUVPixelSemantics::PrenormalizedSUV == model.semantics);
      CPPUNIT_ASSERT(variant == model.sourceVariant);
    }
  }

  // ---- Patient's Weight reaching the computation ----

  // ---- The adaptation record survives a save ----

  void AdaptationRecord_IsWrittenToTheOutput()
  {
    auto f = MakeFilterWithGramEncodedWeight();
    f->Update();
    auto output = f->GetOutput();

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), f->GetAdaptations().size());

    CPPUNIT_ASSERT_EQUAL(
      mitk::SerializeAdaptations(f->GetAdaptations()),
      ReadStringProperty(output, mitk::SUV_ADAPTATIONS_PROPERTY_NAME));

    const std::string derivation = ReadStringProperty(
      output, mitk::DICOMTagPathToPropertyName(mitk::DICOMTagPath(0x0008, 0x2111)));
    CPPUNIT_ASSERT(derivation.find("1 IBSI-SUV input adaptation applied") != std::string::npos);

    // The corrected weight, not the input's gram-encoded original: a reader
    // taking (0010,1030) at face value must not conclude the SUV came from
    // a one-tonne patient.
    const std::string weight = ReadStringProperty(
      output, mitk::DICOMTagPathToPropertyName(mitk::DICOMTagPath(0x0010, 0x1030)));
    CPPUNIT_ASSERT(weight.find("70") != std::string::npos);
    CPPUNIT_ASSERT(weight.find("70000") == std::string::npos);
  }

  void AdaptationRecord_SurvivesSaveAndReload()
  {
    // The stage's real assertion. mitk::ItkImageIO drops a property whose
    // name has no registered persistence info without an error, a warning
    // or a log line, so an unregistered record would pass every in-memory
    // test above and vanish on disk. Only a round-trip catches that.
    auto f = MakeFilterWithGramEncodedWeight();
    f->Update();

    const std::string expectedRecord =
      ReadStringProperty(f->GetOutput(), mitk::SUV_ADAPTATIONS_PROPERTY_NAME);
    const std::string derivationName =
      mitk::DICOMTagPathToPropertyName(mitk::DICOMTagPath(0x0008, 0x2111));
    const std::string expectedDerivation = ReadStringProperty(f->GetOutput(), derivationName);
    CPPUNIT_ASSERT(!expectedRecord.empty());
    CPPUNIT_ASSERT(!expectedDerivation.empty());

    const std::string path =
      mitk::IOUtil::CreateTemporaryFile("mitkSUVAdaptationsXXXXXX.nrrd");
    mitk::IOUtil::Save(f->GetOutput(), path);
    auto reloaded = mitk::IOUtil::Load<mitk::Image>(path);
    CPPUNIT_ASSERT(reloaded.IsNotNull());

    // Asserted first and separately because this one is registered by the
    // PET module's own activator, which linking the module guarantees has
    // run. If it survives and a DICOM-homed property does not, the fault is
    // in the test binary's module context, not in the filter.
    CPPUNIT_ASSERT_EQUAL(expectedRecord,
                         ReadStringProperty(reloaded, mitk::SUV_ADAPTATIONS_PROPERTY_NAME));
    CPPUNIT_ASSERT_EQUAL(expectedDerivation, ReadStringProperty(reloaded, derivationName));

    std::remove(path.c_str());
  }

  void AdaptationRecord_UnadaptedInput_RecordsEmptyAndStillDescribesTheDerivation()
  {
    auto img = MakeMinimalImage();
    const std::string key =
      mitk::DICOMTagPathToPropertyName(mitk::DICOMTagPath(0x0010, 0x1030));
    auto prop = mitk::DICOMProperty::New();
    prop->SetValue(0, 0, "70");
    img->SetProperty(key.c_str(), prop);

    auto f = mitk::SUVImageFilter::New();
    f->SetInput(img);
    f->SetTargetVariant(mitk::SUVVariant::BW);
    f->SetDICOMReadPolicy(mitk::DICOMReadPolicy::Lenient);
    f->SetInputModelOverride(MakePrenormalizedBwInputModel());
    f->Update();

    CPPUNIT_ASSERT(f->GetAdaptations().empty());
    CPPUNIT_ASSERT_EQUAL(std::string("[]"),
                         ReadStringProperty(f->GetOutput(), mitk::SUV_ADAPTATIONS_PROPERTY_NAME));
    // Derived either way, so the description is written either way -- its
    // absence must not be the signal that nothing was adapted.
    CPPUNIT_ASSERT_EQUAL(
      std::string("MITK SUV"),
      ReadStringProperty(f->GetOutput(),
                         mitk::DICOMTagPathToPropertyName(mitk::DICOMTagPath(0x0008, 0x2111))));
  }

  void GramEncodedWeight_YieldsSameEffectiveWeightAsKilograms()
  {
    // The helper returns kilograms and the filter multiplies by 1000, so a
    // gram-encoded export and its kilogram equivalent have to arrive at the
    // same effective weight. Asserting at the filter rather than the helper
    // is what shows the reinterpretation actually reaches the SUV -- the
    // 1000x error it prevents lives in this multiplication.
    const auto effectiveFor = [](const char* storedWeight) {
      auto img = MakeMinimalImage();
      const std::string key = mitk::DICOMTagPathToPropertyName(
        mitk::DICOMTagPath(0x0010, 0x1030));
      auto prop = mitk::DICOMProperty::New();
      prop->SetValue(0, 0, storedWeight);
      img->SetProperty(key.c_str(), prop);

      auto f = mitk::SUVImageFilter::New();
      f->SetInput(img);
      f->SetTargetVariant(mitk::SUVVariant::BW);
      f->SetDICOMReadPolicy(mitk::DICOMReadPolicy::Lenient);
      f->SetInputModelOverride(MakePrenormalizedBwInputModel());
      f->ConfigureFromProperties(img.GetPointer());
      return f->GetEffectivePatientWeightInGram();
    };

    CPPUNIT_ASSERT_DOUBLES_EQUAL(70000.0, effectiveFor("70"), 1e-6);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(70000.0, effectiveFor("70000"), 1e-6);
  }

  // ---- Per-(timestep, slice) decay-time override map ----
  //
  // The map slot lets the caller pin decay durations on a per-slice basis,
  // bypassing the DICOM-derived pipeline. The filter's contract:
  //   - mutually exclusive with the uniform override at Set-time;
  //   - must be a complete and well-formed match for the input image
  //     geometry at Configure-time (no silent DICOM fallback);
  //   - strategy reported back as DecayCorrectionStrategy::Manual.
  //
  // The image fixture is single-timestep 1x1xN so timestep / slice failure
  // modes can be exercised independently.

private:
  static mitk::Image::Pointer MakeMultiSliceImage(unsigned int nSlices)
  {
    auto img = mitk::Image::New();
    const auto pixelType = mitk::MakeScalarPixelType<float>();
    const unsigned int dims[3] = { 1u, 1u, nSlices };
    img->Initialize(pixelType, 3, dims);
    std::vector<float> data(nSlices, 0.0f);
    img->SetVolume(data.data());
    return img;
  }

  static mitk::SUVInputModel MakeActivityInputModel()
  {
    mitk::SUVInputModel m;
    m.semantics     = mitk::SUVPixelSemantics::ActivityConcentration;
    m.activityScale = 1.0;
    return m;
  }

  // Build a filter that can be Configured without DICOM properties: BW
  // variant + explicit overrides for everything Configure needs on the
  // activity-to-SUV path. The decay-time override slot is left untouched
  // and must be set by the test before Configure.
  static mitk::SUVImageFilter::Pointer MakeFilterWithActivityOverrides(
    const mitk::Image::Pointer& img)
  {
    auto f = mitk::SUVImageFilter::New();
    f->SetInput(img);
    f->SetTargetVariant(mitk::SUVVariant::BW);
    f->SetPatientWeightInGram(70000.0);
    f->SetInjectedActivityInBq(3.7e8);
    f->SetHalfLifeInSec(6586.2);
    f->SetInputModelOverride(MakeActivityInputModel());
    return f;
  }

  static mitk::DecayTimeMapType MakeCompleteMap(unsigned int nSlices,
                                                double         value)
  {
    mitk::DecayTimeMapType m;
    for (unsigned int z = 0; z < nSlices; ++z)
    {
      m[0][z] = value;
    }
    return m;
  }

public:

  void SetClear_DecayTimeOverrideMap()
  {
    auto f = mitk::SUVImageFilter::New();
    CPPUNIT_ASSERT(!f->GetDecayTimeOverrideMap().has_value());

    auto map = MakeCompleteMap(3, 1200.0);
    f->SetDecayTimeOverrideMap(map);
    CPPUNIT_ASSERT(f->GetDecayTimeOverrideMap().has_value());
    CPPUNIT_ASSERT_EQUAL(std::size_t{1}, f->GetDecayTimeOverrideMap().value().size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1200.0,
      f->GetDecayTimeOverrideMap().value().at(0).at(1), 1e-9);

    f->ClearDecayTimeOverrideMap();
    CPPUNIT_ASSERT(!f->GetDecayTimeOverrideMap().has_value());
  }

  void DecayMap_SetUniformThenMap_Throws()
  {
    auto f = mitk::SUVImageFilter::New();
    f->SetDecayTimeOverrideInSec(1200.0);
    CPPUNIT_ASSERT_THROW(f->SetDecayTimeOverrideMap(MakeCompleteMap(1, 1200.0)),
                         mitk::ConflictingDecayTimeOverrideException);
    // Uniform slot is unchanged after the rejected map set.
    CPPUNIT_ASSERT(f->GetDecayTimeOverrideInSec().has_value());
    CPPUNIT_ASSERT(!f->GetDecayTimeOverrideMap().has_value());
  }

  void DecayMap_SetMapThenUniform_Throws()
  {
    auto f = mitk::SUVImageFilter::New();
    f->SetDecayTimeOverrideMap(MakeCompleteMap(1, 1200.0));
    CPPUNIT_ASSERT_THROW(f->SetDecayTimeOverrideInSec(1200.0),
                         mitk::ConflictingDecayTimeOverrideException);
    // Map slot is unchanged after the rejected uniform set.
    CPPUNIT_ASSERT(f->GetDecayTimeOverrideMap().has_value());
    CPPUNIT_ASSERT(!f->GetDecayTimeOverrideInSec().has_value());
  }

  void DecayMap_ConfigureWithCompleteMap_EffectiveStrategyIsManual()
  {
    auto img = MakeMultiSliceImage(3);
    auto f   = MakeFilterWithActivityOverrides(img);
    f->SetDecayTimeOverrideMap(MakeCompleteMap(3, 1500.0));
    CPPUNIT_ASSERT_NO_THROW(f->ConfigureFromProperties(img.GetPointer()));
    CPPUNIT_ASSERT(mitk::DecayCorrectionStrategy::Manual ==
                   f->GetEffectiveDecayCorrection().strategy);
  }

  void DecayMap_ConfigureWithCompleteMap_EffectiveDecayTimesMatch()
  {
    auto img = MakeMultiSliceImage(3);
    auto f   = MakeFilterWithActivityOverrides(img);
    mitk::DecayTimeMapType map;
    map[0][0] = 1000.0;
    map[0][1] = 2000.0;
    map[0][2] = 3000.0;
    f->SetDecayTimeOverrideMap(map);
    f->ConfigureFromProperties(img.GetPointer());
    const auto& eff = f->GetEffectiveDecayCorrection().decayTimes;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1000.0, eff.at(0).at(0), 1e-9);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2000.0, eff.at(0).at(1), 1e-9);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3000.0, eff.at(0).at(2), 1e-9);
  }

  // A map+uniform combination is unreachable through the public API (the
  // Set-time mutex rejects setting both), so the map-over-uniform precedence
  // cannot be exercised directly; this covers the reachable case: a decay map
  // alone resolves to the effective decay times.
  void DecayMap_ConfiguredWithoutUniform_MapValuesSurface()
  {
    auto img = MakeMultiSliceImage(2);
    auto f   = MakeFilterWithActivityOverrides(img);
    mitk::DecayTimeMapType map;
    map[0][0] = 4242.0;
    map[0][1] = 4243.0;
    f->SetDecayTimeOverrideMap(map);
    f->ConfigureFromProperties(img.GetPointer());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4242.0,
      f->GetEffectiveDecayCorrection().decayTimes.at(0).at(0), 1e-9);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4243.0,
      f->GetEffectiveDecayCorrection().decayTimes.at(0).at(1), 1e-9);
  }

  void DecayMap_MissingSliceEntry_Throws()
  {
    auto img = MakeMultiSliceImage(3);
    auto f   = MakeFilterWithActivityOverrides(img);
    mitk::DecayTimeMapType map;
    map[0][0] = 1000.0;
    // slice 1 missing
    map[0][2] = 3000.0;
    f->SetDecayTimeOverrideMap(map);
    CPPUNIT_ASSERT_THROW(f->ConfigureFromProperties(img.GetPointer()),
                         mitk::InvalidDecayTimeMapException);
  }

  void DecayMap_OutOfRangeSlice_Throws()
  {
    auto img = MakeMultiSliceImage(3);
    auto f   = MakeFilterWithActivityOverrides(img);
    auto map = MakeCompleteMap(3, 1500.0);
    map[0][9] = 9999.0;
    f->SetDecayTimeOverrideMap(map);
    CPPUNIT_ASSERT_THROW(f->ConfigureFromProperties(img.GetPointer()),
                         mitk::InvalidDecayTimeMapException);
  }

  void DecayMap_OutOfRangeTimestep_Throws()
  {
    auto img = MakeMultiSliceImage(3);
    auto f   = MakeFilterWithActivityOverrides(img);
    auto map = MakeCompleteMap(3, 1500.0);
    map[1][0] = 100.0;  // image has only timestep 0
    f->SetDecayTimeOverrideMap(map);
    CPPUNIT_ASSERT_THROW(f->ConfigureFromProperties(img.GetPointer()),
                         mitk::InvalidDecayTimeMapException);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkSUVImageFilter)
