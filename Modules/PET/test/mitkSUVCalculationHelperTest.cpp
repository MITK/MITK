/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <cmath>
#include <string>

#include <mitkDICOMProperty.h>
#include <mitkDICOMTagPath.h>
#include <mitkImage.h>
#include <mitkPixelType.h>
#include <mitkProperties.h>
#include <mitkSUVCalculationHelper.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

namespace
{
  // Build a 1x1xnSlicesxnTimeSteps mitk::Image with float pixels. The data
  // buffer is irrelevant for these tests; only the time geometry / slice
  // count is consulted by DeduceDecayCorrection's iteration loop.
  mitk::Image::Pointer MakeSyntheticImage(unsigned int nSlices, unsigned int nTimeSteps)
  {
    mitk::Image::Pointer image = mitk::Image::New();
    const mitk::PixelType pixelType = mitk::MakeScalarPixelType<float>();

    if (nTimeSteps > 1)
    {
      const unsigned int dims[4] = { 1u, 1u, nSlices, nTimeSteps };
      image->Initialize(pixelType, 4, dims);
    }
    else
    {
      const unsigned int dims[3] = { 1u, 1u, nSlices };
      image->Initialize(pixelType, 3, dims);
    }
    return image;
  }

  // Attach a DICOMProperty to the image's PropertyList using the property
  // name format produced by MitkDICOM (DICOM.GGGG.EEEE for top-level tags,
  // DICOM.GGGG.EEEE.[N].GGGG.EEEE for sequence items with concrete index N).
  void SetDicomProperty(mitk::Image* image,
                        const std::string& propertyName,
                        const std::string& value,
                        mitk::TimeStepType timeStep = 0,
                        mitk::TemporoSpatialStringProperty::IndexValueType slice = 0)
  {
    auto existing = image->GetProperty(propertyName.c_str());
    auto* dicomProp = dynamic_cast<mitk::DICOMProperty*>(existing.GetPointer());

    if (nullptr == dicomProp)
    {
      auto newProp = mitk::DICOMProperty::New();
      newProp->SetValue(timeStep, slice, value);
      image->SetProperty(propertyName.c_str(), newProp);
    }
    else
    {
      dicomProp->SetValue(timeStep, slice, value);
    }
  }

  std::string PropName(unsigned int group, unsigned int element)
  {
    mitk::DICOMTagPath path(group, element);
    return mitk::DICOMTagPathToPropertyName(path);
  }

  // Property name for a sequence item with concrete index 0.
  std::string SeqPropName(unsigned int seqGroup, unsigned int seqElement,
                          unsigned int group, unsigned int element)
  {
    mitk::DICOMTagPath path;
    path.AddSelection(seqGroup, seqElement, 0).AddElement(group, element);
    return mitk::DICOMTagPathToPropertyName(path);
  }
}

class mitkSUVCalculationHelperTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkSUVCalculationHelperTestSuite);

  // GetDecayCorrectionStrategy
  MITK_TEST(Strategy_Admin);
  MITK_TEST(Strategy_Start);
  MITK_TEST(Strategy_None);
  MITK_TEST(Strategy_CaseInsensitive);
  MITK_TEST(Strategy_Missing_Throws_MissingDICOMPropertyException);
  MITK_TEST(Strategy_UnknownValue_Throws_InvalidDICOMPropertyValueException);

  // Radiopharmaceutical info helper
  MITK_TEST(Radiopharm_Empty_ReturnsEmpty);
  MITK_TEST(Radiopharm_SingleItem_FullyPopulated);
  MITK_TEST(Radiopharm_TwoItems_PreserveIndexPairing);
  MITK_TEST(Radiopharm_PartiallyPopulatedItem_NaNFields);
  MITK_TEST(RadionuclideTotalDose_BelowThreshold_ConvertsFromMBq);
  MITK_TEST(RadionuclideTotalDose_AboveThreshold_PassesThrough);
  MITK_TEST(RadionuclideTotalDose_AtThresholdExactly_PassesThrough);
  MITK_TEST(RadionuclideTotalDose_ZeroOrNegative_PassesThrough);
  MITK_TEST(RadionuclideTotalDose_BelowThreshold_StrictPolicy_Throws);
  MITK_TEST(RadionuclideTotalDose_BelowThreshold_StrictPolicy_CatchableAsBaseException);
  MITK_TEST(RadionuclideTotalDose_AboveThreshold_StrictPolicy_PassesThrough);

  // Patient weight
  MITK_TEST(PatientWeight_Found);
  MITK_TEST(PatientWeight_Missing_Throws_MissingDICOMPropertyException);

  // Patient height
  MITK_TEST(PatientHeight_Found);
  MITK_TEST(PatientHeight_Missing_Throws_MissingDICOMPropertyException);

  // Patient sex
  MITK_TEST(PatientSex_Male);
  MITK_TEST(PatientSex_Female);
  MITK_TEST(PatientSex_TrimAndCaseInsensitive);
  MITK_TEST(PatientSex_Missing_Throws_MissingDICOMPropertyException);
  MITK_TEST(PatientSex_Other_ReturnsOther);
  MITK_TEST(PatientSex_Unknown_Throws_InvalidDICOMPropertyValueException);
  MITK_TEST(PatientSex_Garbage_Throws_InvalidDICOMPropertyValueException);

  // GetManufacturerFamily
  MITK_TEST(GetManufacturerFamily_Siemens_RecognizesSubstring);
  MITK_TEST(GetManufacturerFamily_GE_RecognizesGEMedicalSystems);
  MITK_TEST(GetManufacturerFamily_Philips_RecognizesSubstring);
  MITK_TEST(GetManufacturerFamily_CaseInsensitive);
  MITK_TEST(GetManufacturerFamily_OtherFallsThroughToOther);
  MITK_TEST(GetManufacturerFamily_NullProvider_ReturnsOther);

  // DeduceDecayCorrection
  MITK_TEST(Admin_AllZeros_SingleTimestep);
  MITK_TEST(Admin_FromImageGeometry_NoAcquisitionTags);
  MITK_TEST(Start_Step2_AcqTimeEqualsSeriesTime_UsesAcqTime_SingleTimestep);
  MITK_TEST(Start_Step2_AcqTimeEqualsSeriesTime_UsesAcqTime_MultiTimestep);
  MITK_TEST(Start_Step1_SiemensPrivateDateTime_UsesPrivate);
  MITK_TEST(Start_Step1_GEPrivateDateTime_UsesPrivate);
  MITK_TEST(Start_Step1_SiemensPrivateDateTime_PerSliceVariation);
  MITK_TEST(Start_Step3_Siemens_TaveFormula);
  MITK_TEST(Start_Step3_Siemens_StrictPolicy_Refused);
  MITK_TEST(Start_Step4_GE_DeltaFormula);
  MITK_TEST(Start_Step4_GE_StrictPolicy_Refused);
  MITK_TEST(Start_Step5_OtherManufacturer_Throws_AmbiguousDecayTimingException);
  MITK_TEST(None_PerSliceDecayTime);
  MITK_TEST(None_TaveCorrection_Applied);
  MITK_TEST(None_MissingFrameDuration_Throws_MissingDICOMPropertyException);
  MITK_TEST(Prefers_1078_Over_1072);
  MITK_TEST(Rollover_1072_Recovered);
  MITK_TEST(Rollover_1078_NoSilentCorrection_Throws_AmbiguousDecayTimingException);
  MITK_TEST(Strategy_Start_MissingSeriesTime_Throws_MissingDICOMPropertyException);
  MITK_TEST(Strategy_None_MissingAcqTime_Throws_MissingDICOMPropertyException);

  CPPUNIT_TEST_SUITE_END();

private:

  // Most decay-correction tests share a common acquisition-day setup.
  // Series Time 12:15:30, injection (1078) on the same day at 11:00:00 ->
  // SeriesTime - InjectionTime = (12*3600+15*60+30) - (11*3600) = 4530 s.
  // After the IBSI-SUV-conformant DC=START rewrite this fixture exercises
  // Step 2 of the fallback chain: Manufacturer is set to "SIEMENS" and
  // AcquisitionDate / AcquisitionTime equal SeriesDate / SeriesTime, so
  // the helper picks per-slice AcquisitionTime as the reference. The
  // numeric expectation is unchanged because AcqTime equals SeriesTime.
  static constexpr double kStartExpectedDecaySeconds = 4530.0;

  void SetupCommonStartCase(mitk::Image* image)
  {
    SetDicomProperty(image, PropName(0x0054, 0x1102), "START");
    SetDicomProperty(image, PropName(0x0008, 0x0021), "20260430");          // SeriesDate
    SetDicomProperty(image, PropName(0x0008, 0x0031), "121530");            // SeriesTime
    SetDicomProperty(image, PropName(0x0008, 0x0070), "SIEMENS");           // Manufacturer
    SetDicomProperty(image, PropName(0x0008, 0x0022), "20260430");          // AcqDate (== SeriesDate)
    SetDicomProperty(image, PropName(0x0008, 0x0032), "121530");            // AcqTime (== SeriesTime)
    SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1078),
                     "20260430110000");                                     // RP Start DT
  }

public:

  // ---- GetDecayCorrectionStrategy ----

  void Strategy_Admin()
  {
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, PropName(0x0054, 0x1102), "ADMIN");

    CPPUNIT_ASSERT_EQUAL(mitk::DecayCorrectionStrategy::Admin,
                         mitk::GetDecayCorrectionStrategy(image));
  }

  void Strategy_Start()
  {
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, PropName(0x0054, 0x1102), "START");

    CPPUNIT_ASSERT_EQUAL(mitk::DecayCorrectionStrategy::Start,
                         mitk::GetDecayCorrectionStrategy(image));
  }

  void Strategy_None()
  {
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, PropName(0x0054, 0x1102), "NONE");

    CPPUNIT_ASSERT_EQUAL(mitk::DecayCorrectionStrategy::None,
                         mitk::GetDecayCorrectionStrategy(image));
  }

  void Strategy_CaseInsensitive()
  {
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, PropName(0x0054, 0x1102), " start ");

    CPPUNIT_ASSERT_EQUAL(mitk::DecayCorrectionStrategy::Start,
                         mitk::GetDecayCorrectionStrategy(image));
  }

  void Strategy_Missing_Throws_MissingDICOMPropertyException()
  {
    auto image = MakeSyntheticImage(1, 1);
    CPPUNIT_ASSERT_THROW(mitk::GetDecayCorrectionStrategy(image),
                         mitk::MissingDICOMPropertyException);
  }

  void Strategy_UnknownValue_Throws_InvalidDICOMPropertyValueException()
  {
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, PropName(0x0054, 0x1102), "FOO");

    CPPUNIT_ASSERT_THROW(mitk::GetDecayCorrectionStrategy(image),
                         mitk::InvalidDICOMPropertyValueException);
  }

  // ---- Radiopharmaceutical info helper ----

  void Radiopharm_Empty_ReturnsEmpty()
  {
    auto image = MakeSyntheticImage(1, 1);
    CPPUNIT_ASSERT(mitk::GetRadiopharmaceuticalInfos(image).empty());
  }

  void Radiopharm_SingleItem_FullyPopulated()
  {
    auto image = MakeSyntheticImage(1, 1);
    // (0054,0016).[0].(0018,1075) Half-Life
    SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1075), "6586.26");
    // (0054,0016).[0].(0018,1074) Total Dose
    SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1074), "1.85e8");
    // (0054,0016).[0].(0054,0300).[0].(0008,0104) Code Meaning
    {
      mitk::DICOMTagPath path;
      path.AddSelection(0x0054, 0x0016, 0).AddSelection(0x0054, 0x0300, 0).AddElement(0x0008, 0x0104);
      SetDicomProperty(image, mitk::DICOMTagPathToPropertyName(path), "^18F^");
    }

    const auto infos = mitk::GetRadiopharmaceuticalInfos(image);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), infos.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6586.26, infos[0].halfLifeSeconds, 1e-9);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.85e8,  infos[0].totalDoseBq,     1.0);
    CPPUNIT_ASSERT_EQUAL(std::string("18F"), infos[0].name);
  }

  void Radiopharm_TwoItems_PreserveIndexPairing()
  {
    auto image = MakeSyntheticImage(1, 1);

    // Item [0]: 18F-style values.
    SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1075), "6586.26");
    SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1074), "1.85e8");
    {
      mitk::DICOMTagPath path;
      path.AddSelection(0x0054, 0x0016, 0).AddSelection(0x0054, 0x0300, 0).AddElement(0x0008, 0x0104);
      SetDicomProperty(image, mitk::DICOMTagPathToPropertyName(path), "^18F^");
    }

    // Item [1]: 68Ga-style values, deliberately *different* so a pairing bug
    // would surface as a swap.
    {
      mitk::DICOMTagPath path1;
      path1.AddSelection(0x0054, 0x0016, 1).AddElement(0x0018, 0x1075);
      SetDicomProperty(image, mitk::DICOMTagPathToPropertyName(path1), "4062.6");
    }
    {
      mitk::DICOMTagPath path2;
      path2.AddSelection(0x0054, 0x0016, 1).AddElement(0x0018, 0x1074);
      SetDicomProperty(image, mitk::DICOMTagPathToPropertyName(path2), "5.0e7");
    }
    {
      mitk::DICOMTagPath path3;
      path3.AddSelection(0x0054, 0x0016, 1).AddSelection(0x0054, 0x0300, 0).AddElement(0x0008, 0x0104);
      SetDicomProperty(image, mitk::DICOMTagPathToPropertyName(path3), "^68Ga^");
    }

    const auto infos = mitk::GetRadiopharmaceuticalInfos(image);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), infos.size());

    // Pairing: each index must hold values from the same source item.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6586.26, infos[0].halfLifeSeconds, 1e-9);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.85e8,  infos[0].totalDoseBq,     1.0);
    CPPUNIT_ASSERT_EQUAL(std::string("18F"), infos[0].name);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(4062.6,  infos[1].halfLifeSeconds, 1e-9);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0e7,   infos[1].totalDoseBq,     1.0);
    CPPUNIT_ASSERT_EQUAL(std::string("68Ga"), infos[1].name);
  }

  void Radiopharm_PartiallyPopulatedItem_NaNFields()
  {
    auto image = MakeSyntheticImage(1, 1);

    // Item [0]: only dose populated.
    SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1074), "1.85e8");

    // Item [1]: only half-life populated.
    {
      mitk::DICOMTagPath path;
      path.AddSelection(0x0054, 0x0016, 1).AddElement(0x0018, 0x1075);
      SetDicomProperty(image, mitk::DICOMTagPathToPropertyName(path), "6586.26");
    }

    const auto infos = mitk::GetRadiopharmaceuticalInfos(image);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), infos.size());

    // Item [0]: dose set, half-life NaN, name empty.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.85e8, infos[0].totalDoseBq, 1.0);
    CPPUNIT_ASSERT(std::isnan(infos[0].halfLifeSeconds));
    CPPUNIT_ASSERT(infos[0].name.empty());

    // Item [1]: half-life set, dose NaN, name empty.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6586.26, infos[1].halfLifeSeconds, 1e-9);
    CPPUNIT_ASSERT(std::isnan(infos[1].totalDoseBq));
    CPPUNIT_ASSERT(infos[1].name.empty());
  }

  // (0018,1074) is prescribed in Bq by the DICOM standard, but some
  // scanners and post-processing pipelines store it in MBq. Per the
  // IBSI-SUV recommendation, values strictly below 1e4 are interpreted
  // as MBq and converted to Bq. The three tests below pin the
  // boundary: convert if and only if 0 < raw < 1e4.

  void RadionuclideTotalDose_BelowThreshold_ConvertsFromMBq()
  {
    // DRO_3_0 of the IBSI-SUV benchmark uses 368.08 (MBq), which after
    // conversion must be 3.6808e8 Bq.
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1074), "368.08");

    const auto infos = mitk::GetRadiopharmaceuticalInfos(image);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), infos.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.6808e8, infos[0].totalDoseBq, 1.0);
  }

  void RadionuclideTotalDose_AboveThreshold_PassesThrough()
  {
    // A plausible Bq-magnitude FDG dose must not be touched.
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1074), "3.6808e8");

    const auto infos = mitk::GetRadiopharmaceuticalInfos(image);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), infos.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.6808e8, infos[0].totalDoseBq, 1.0);
  }

  void RadionuclideTotalDose_AtThresholdExactly_PassesThrough()
  {
    // Pin the strict-less-than boundary: exactly 1e4 is *not* converted.
    // The IBSI-SUV recommendation reads "lower than 10^4", which makes
    // the upper edge exclusive. A future change to `<=` would silently
    // multiply this value by 1e6.
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1074), "10000");

    const auto infos = mitk::GetRadiopharmaceuticalInfos(image);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), infos.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0e4, infos[0].totalDoseBq, 0.0);
  }

  void RadionuclideTotalDose_ZeroOrNegative_PassesThrough()
  {
    // Zero and negative values must be propagated verbatim so that the
    // CLI's downstream positive-value validation still rejects them as
    // invalid input. Multiplying them by 1e6 would either lose
    // information (0 -> 0) or paper over an upstream error in a way
    // that would mask the original value in logs.

    {
      auto image = MakeSyntheticImage(1, 1);
      SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1074), "0");
      const auto infos = mitk::GetRadiopharmaceuticalInfos(image);
      CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), infos.size());
      CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, infos[0].totalDoseBq, 0.0);
    }

    {
      auto image = MakeSyntheticImage(1, 1);
      SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1074), "-5");
      const auto infos = mitk::GetRadiopharmaceuticalInfos(image);
      CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), infos.size());
      CPPUNIT_ASSERT_DOUBLES_EQUAL(-5.0, infos[0].totalDoseBq, 0.0);
    }
  }

  // DICOMReadPolicy::Strict: refuse to apply benchmark-recommended
  // adaptations. Each rule must (a) raise its category-specific
  // exception type so callers can react precisely, and (b) be catchable
  // via the BenchmarkAdaptationRequiredException base type so the
  // policy can be mapped to one CLI exit code or GUI message.

  void RadionuclideTotalDose_BelowThreshold_StrictPolicy_Throws()
  {
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1074), "368.08");

    CPPUNIT_ASSERT_THROW(
      mitk::GetRadiopharmaceuticalInfos(image, mitk::DICOMReadPolicy::Strict),
      mitk::ImplausibleRadionuclideDoseException);
  }

  void RadionuclideTotalDose_BelowThreshold_StrictPolicy_CatchableAsBaseException()
  {
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1074), "368.08");

    CPPUNIT_ASSERT_THROW(
      mitk::GetRadiopharmaceuticalInfos(image, mitk::DICOMReadPolicy::Strict),
      mitk::BenchmarkAdaptationRequiredException);
  }

  void RadionuclideTotalDose_AboveThreshold_StrictPolicy_PassesThrough()
  {
    // Strict policy must not penalise plausible Bq-magnitude inputs.
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1074), "3.6808e8");

    const auto infos = mitk::GetRadiopharmaceuticalInfos(image, mitk::DICOMReadPolicy::Strict);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), infos.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.6808e8, infos[0].totalDoseBq, 1.0);
  }

  void PatientWeight_Found()
  {
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, PropName(0x0010, 0x1030), "70.5");

    CPPUNIT_ASSERT_DOUBLES_EQUAL(70.5, mitk::GetPatientsWeight(image), 1e-9);
  }

  void PatientWeight_Missing_Throws_MissingDICOMPropertyException()
  {
    auto image = MakeSyntheticImage(1, 1);
    CPPUNIT_ASSERT_THROW(mitk::GetPatientsWeight(image),
                         mitk::MissingDICOMPropertyException);
  }

  // ---- Patient height ----

  void PatientHeight_Found()
  {
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, PropName(0x0010, 0x1020), "1.78");

    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.78, mitk::GetPatientsHeight(image), 1e-9);
  }

  void PatientHeight_Missing_Throws_MissingDICOMPropertyException()
  {
    auto image = MakeSyntheticImage(1, 1);
    CPPUNIT_ASSERT_THROW(mitk::GetPatientsHeight(image),
                         mitk::MissingDICOMPropertyException);
  }

  // ---- Patient sex ----

  void PatientSex_Male()
  {
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, PropName(0x0010, 0x0040), "M");
    CPPUNIT_ASSERT_EQUAL(mitk::Sex::Male, mitk::GetPatientsSex(image));
  }

  void PatientSex_Female()
  {
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, PropName(0x0010, 0x0040), "F");
    CPPUNIT_ASSERT_EQUAL(mitk::Sex::Female, mitk::GetPatientsSex(image));
  }

  void PatientSex_TrimAndCaseInsensitive()
  {
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, PropName(0x0010, 0x0040), " f ");
    CPPUNIT_ASSERT_EQUAL(mitk::Sex::Female, mitk::GetPatientsSex(image));
  }

  void PatientSex_Missing_Throws_MissingDICOMPropertyException()
  {
    auto image = MakeSyntheticImage(1, 1);
    CPPUNIT_ASSERT_THROW(mitk::GetPatientsSex(image),
                         mitk::MissingDICOMPropertyException);
  }

  void PatientSex_Other_ReturnsOther()
  {
    // DICOM "O" (Other) is accepted; consuming strategies define the
    // policy (IBSI-SUV benchmark convention: mean of M and F factors).
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, PropName(0x0010, 0x0040), "O");
    MITK_TEST_CONDITION_REQUIRED(mitk::GetPatientsSex(image) == mitk::Sex::Other,
                                 "GetPatientsSex returns Sex::Other for DICOM 'O'.");
  }

  void PatientSex_Unknown_Throws_InvalidDICOMPropertyValueException()
  {
    // DICOM "U" (Unknown) is also rejected.
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, PropName(0x0010, 0x0040), "U");
    CPPUNIT_ASSERT_THROW(mitk::GetPatientsSex(image),
                         mitk::InvalidDICOMPropertyValueException);
  }

  void PatientSex_Garbage_Throws_InvalidDICOMPropertyValueException()
  {
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, PropName(0x0010, 0x0040), "yes");
    CPPUNIT_ASSERT_THROW(mitk::GetPatientsSex(image),
                         mitk::InvalidDICOMPropertyValueException);
  }

  // ---- DeduceDecayCorrection: ADMIN ----

  void Admin_AllZeros_SingleTimestep()
  {
    auto image = MakeSyntheticImage(/*nSlices=*/3, /*nTimeSteps=*/1);
    SetDicomProperty(image, PropName(0x0054, 0x1102), "ADMIN");
    // No injection-time tags attached on purpose: ADMIN must not require them.

    const auto info = mitk::DeduceDecayCorrection(image);
    CPPUNIT_ASSERT_EQUAL(mitk::DecayCorrectionStrategy::Admin, info.strategy);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), info.decayTimes.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), info.decayTimes.at(0).size());
    for (const auto& slice : info.decayTimes.at(0))
    {
      CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, slice.second, 1e-15);
    }
  }

  void Admin_FromImageGeometry_NoAcquisitionTags()
  {
    // Pin the contract: ADMIN populates the map purely from the image's
    // SlicedData time geometry, NOT from any DICOM acquisition iteration tag.
    auto image = MakeSyntheticImage(/*nSlices=*/4, /*nTimeSteps=*/2);
    SetDicomProperty(image, PropName(0x0054, 0x1102), "ADMIN");

    const auto info = mitk::DeduceDecayCorrection(image);
    CPPUNIT_ASSERT_EQUAL(mitk::DecayCorrectionStrategy::Admin, info.strategy);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), info.decayTimes.size());
    for (mitk::TimeStepType t = 0; t < 2; ++t)
    {
      CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(4), info.decayTimes.at(t).size());
    }
  }

  // ---- DeduceDecayCorrection: START ----
  //
  // The fallback chain (IBSI-SUV recommendation) tested below in priority
  // order: Step 1 (vendor private datetime), Step 2 (AcqTime == SeriesTime),
  // Step 3 (Siemens/Philips T_ave), Step 4 (GE -deltat), Step 5 (throw).

  void Start_Step2_AcqTimeEqualsSeriesTime_UsesAcqTime_SingleTimestep()
  {
    // SetupCommonStartCase configures Manufacturer=SIEMENS and AcqTime
    // equal to SeriesTime, so Step 2 of the fallback chain fires. The
    // numeric expectation is unchanged from the pre-rewrite baseline
    // because AcqTime == SeriesTime here.
    auto image = MakeSyntheticImage(/*nSlices=*/3, /*nTimeSteps=*/1);
    SetupCommonStartCase(image);

    const auto info = mitk::DeduceDecayCorrection(image);
    CPPUNIT_ASSERT_EQUAL(mitk::DecayCorrectionStrategy::Start, info.strategy);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), info.decayTimes.size());
    for (const auto& slice : info.decayTimes.at(0))
    {
      CPPUNIT_ASSERT_DOUBLES_EQUAL(kStartExpectedDecaySeconds, slice.second, 1e-3);
    }
  }

  void Start_Step2_AcqTimeEqualsSeriesTime_UsesAcqTime_MultiTimestep()
  {
    auto image = MakeSyntheticImage(/*nSlices=*/2, /*nTimeSteps=*/3);
    SetupCommonStartCase(image);
    // SetupCommonStartCase populates (0,0) only. The DC=START Step 2 path
    // reads AcqDate/AcqTime per (t, s); explicitly populate every slot
    // rather than rely on default-context fallback semantics that aren't
    // exercised elsewhere in the test suite.
    for (mitk::TimeStepType t = 0; t < 3; ++t)
    {
      for (unsigned int s = 0; s < 2; ++s)
      {
        SetDicomProperty(image, PropName(0x0008, 0x0022), "20260430", t, s);
        SetDicomProperty(image, PropName(0x0008, 0x0032), "121530", t, s);
      }
    }

    const auto info = mitk::DeduceDecayCorrection(image);
    CPPUNIT_ASSERT_EQUAL(mitk::DecayCorrectionStrategy::Start, info.strategy);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), info.decayTimes.size());
    for (mitk::TimeStepType t = 0; t < 3; ++t)
    {
      CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), info.decayTimes.at(t).size());
      for (const auto& slice : info.decayTimes.at(t))
      {
        CPPUNIT_ASSERT_DOUBLES_EQUAL(kStartExpectedDecaySeconds, slice.second, 1e-3);
      }
    }
  }

  void Start_Step1_SiemensPrivateDateTime_UsesPrivate()
  {
    // Manufacturer Siemens with mitk.pet.SiemensDecayDateTime attached
    // (lifted out of (0071,0x22) by the PET reader as a DICOMProperty;
    // see issue #783). Step 1 fires before Step 2 even though SeriesTime
    // is also valid. The private datetime is at 12:00:00, deliberately
    // different from SeriesTime = 12:15:30 so we can tell which one was
    // used. Single-slice -> uniform value.
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, PropName(0x0054, 0x1102), "START");
    SetDicomProperty(image, PropName(0x0008, 0x0021), "20260430");
    SetDicomProperty(image, PropName(0x0008, 0x0031), "121530");
    SetDicomProperty(image, PropName(0x0008, 0x0070), "SIEMENS Healthineers");
    SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1078),
                     "20260430110000");

    SetDicomProperty(image, "mitk.pet.SiemensDecayDateTime", "20260430120000");

    const auto info = mitk::DeduceDecayCorrection(image);
    // privateDT - injection = 12:00:00 - 11:00:00 = 3600 s.
    // (Step 2 would yield 4530 s -- a different number -- so this assertion
    // verifies Step 1's precedence.)
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3600.0, info.decayTimes.at(0).at(0), 1e-3);
  }

  void Start_Step1_GEPrivateDateTime_UsesPrivate()
  {
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, PropName(0x0054, 0x1102), "START");
    SetDicomProperty(image, PropName(0x0008, 0x0021), "20260430");
    SetDicomProperty(image, PropName(0x0008, 0x0031), "121530");
    SetDicomProperty(image, PropName(0x0008, 0x0070), "GE MEDICAL SYSTEMS");
    SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1078),
                     "20260430110000");

    SetDicomProperty(image, "mitk.pet.GEScanDateTime", "20260430120000");

    const auto info = mitk::DeduceDecayCorrection(image);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3600.0, info.decayTimes.at(0).at(0), 1e-3);
  }

  void Start_Step1_SiemensPrivateDateTime_PerSliceVariation()
  {
    // Pin the per-slice path: when the lifted private datetime varies per
    // slice (multi-bed acquisitions can produce this), each slice must
    // get its own decay duration. Slice 0 carries 12:00:00, slice 1
    // carries 12:30:00; injection at 11:00:00 gives 3600 s and 5400 s.
    auto image = MakeSyntheticImage(/*nSlices=*/2, /*nTimeSteps=*/1);
    SetDicomProperty(image, PropName(0x0054, 0x1102), "START");
    SetDicomProperty(image, PropName(0x0008, 0x0021), "20260430");
    SetDicomProperty(image, PropName(0x0008, 0x0031), "121530");
    SetDicomProperty(image, PropName(0x0008, 0x0070), "SIEMENS Healthineers");
    SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1078),
                     "20260430110000");

    SetDicomProperty(image, "mitk.pet.SiemensDecayDateTime", "20260430120000", 0, 0);
    SetDicomProperty(image, "mitk.pet.SiemensDecayDateTime", "20260430123000", 0, 1);

    const auto info = mitk::DeduceDecayCorrection(image);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3600.0, info.decayTimes.at(0).at(0), 1e-3);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5400.0, info.decayTimes.at(0).at(1), 1e-3);
  }

  void Start_Step3_Siemens_TaveFormula()
  {
    // Manufacturer Siemens, AcqTime != SeriesTime (so Step 2 doesn't fire),
    // ActualFrameDuration and FrameReferenceTime present (so Step 3 fires).
    // SeriesTime = 12:15:30, AcqTime = 12:16:00 -- differs by 30 s, hence
    // not "equal in seconds" -> Step 2 declined.
    // ActualFrameDuration = 60 s -> T_ave ~= 29.984 s for 18F (T_half = 6586.26 s).
    // T_ave hand-computed via Taylor: T/2 - lambda*T^2/24 = 30 - 0.01578.
    // FrameReferenceTime = 1500 ms = 1.5 s; the IBSI-SUV formula subtracts
    // FrameReferenceTime to undo the scanner-applied offset from the per-frame
    // midpoint back to the start of acquisition.
    auto image = MakeSyntheticImage(/*nSlices=*/1, /*nTimeSteps=*/1);
    SetDicomProperty(image, PropName(0x0054, 0x1102), "START");
    SetDicomProperty(image, PropName(0x0008, 0x0021), "20260430");
    SetDicomProperty(image, PropName(0x0008, 0x0031), "121530");
    SetDicomProperty(image, PropName(0x0008, 0x0070), "SIEMENS Healthineers");
    SetDicomProperty(image, PropName(0x0008, 0x0022), "20260430");
    SetDicomProperty(image, PropName(0x0008, 0x0032), "121600");
    SetDicomProperty(image, PropName(0x0018, 0x1242), "60000");   // 60 s frame
    SetDicomProperty(image, PropName(0x0054, 0x1300), "1500");    // 1.5 s
    SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1078),
                     "20260430110000");

    const auto info = mitk::DeduceDecayCorrection(image, /*halfLife=*/6586.26);
    // (AcqTime + T_ave - FrameReferenceTime) - Injection
    //   = (12:16:00 + 29.984 s - 1.5 s) - 11:00:00
    //   = 4560 + 29.984 - 1.5 = 4588.484 s.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4588.484, info.decayTimes.at(0).at(0), 0.1);
  }

  void Start_Step4_GE_DeltaFormula()
  {
    // Manufacturer GE, AcqTime != SeriesTime, no private tag.
    // Step 4: reference time = AcqTime - FrameReferenceTime.
    // AcqTime = 12:16:00, FrameRefTime = 30000 ms = 30 s.
    // Reference = 12:16:00 - 30 s = 12:15:30. Decay = 4530 s.
    // (ActualFrameDuration is required as a precondition but not used by
    // the GE formula itself; the half-life is therefore also irrelevant.)
    auto image = MakeSyntheticImage(/*nSlices=*/1, /*nTimeSteps=*/1);
    SetDicomProperty(image, PropName(0x0054, 0x1102), "START");
    SetDicomProperty(image, PropName(0x0008, 0x0021), "20260430");
    SetDicomProperty(image, PropName(0x0008, 0x0031), "121530");
    SetDicomProperty(image, PropName(0x0008, 0x0070), "GE MEDICAL SYSTEMS");
    SetDicomProperty(image, PropName(0x0008, 0x0022), "20260430");
    SetDicomProperty(image, PropName(0x0008, 0x0032), "121600");
    SetDicomProperty(image, PropName(0x0018, 0x1242), "1");        // 1 ms (precondition)
    SetDicomProperty(image, PropName(0x0054, 0x1300), "30000");    // 30 s
    SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1078),
                     "20260430110000");

    const auto info = mitk::DeduceDecayCorrection(image, /*halfLife=*/6586.26);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4530.0, info.decayTimes.at(0).at(0), 1e-3);
  }

  void Start_Step3_Siemens_StrictPolicy_Refused()
  {
    // Same input that drives Start_Step3_Siemens_TaveFormula to a clean
    // result under Lenient. Strict mode must refuse the empirical
    // fallback and raise VendorEmpiricalDecayFallbackRefusedException.
    auto image = MakeSyntheticImage(/*nSlices=*/1, /*nTimeSteps=*/1);
    SetDicomProperty(image, PropName(0x0054, 0x1102), "START");
    SetDicomProperty(image, PropName(0x0008, 0x0021), "20260430");
    SetDicomProperty(image, PropName(0x0008, 0x0031), "121530");
    SetDicomProperty(image, PropName(0x0008, 0x0070), "SIEMENS Healthineers");
    SetDicomProperty(image, PropName(0x0008, 0x0022), "20260430");
    SetDicomProperty(image, PropName(0x0008, 0x0032), "121600");
    SetDicomProperty(image, PropName(0x0018, 0x1242), "60000");
    SetDicomProperty(image, PropName(0x0054, 0x1300), "1500");
    SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1078),
                     "20260430110000");

    CPPUNIT_ASSERT_THROW(
      mitk::DeduceDecayCorrection(image, /*halfLife=*/6586.26,
                                  mitk::DICOMReadPolicy::Strict),
      mitk::VendorEmpiricalDecayFallbackRefusedException);
  }

  void Start_Step4_GE_StrictPolicy_Refused()
  {
    // GE Step 4 (-FrameReferenceTime) under Strict must also be refused.
    auto image = MakeSyntheticImage(/*nSlices=*/1, /*nTimeSteps=*/1);
    SetDicomProperty(image, PropName(0x0054, 0x1102), "START");
    SetDicomProperty(image, PropName(0x0008, 0x0021), "20260430");
    SetDicomProperty(image, PropName(0x0008, 0x0031), "121530");
    SetDicomProperty(image, PropName(0x0008, 0x0070), "GE MEDICAL SYSTEMS");
    SetDicomProperty(image, PropName(0x0008, 0x0022), "20260430");
    SetDicomProperty(image, PropName(0x0008, 0x0032), "121600");
    SetDicomProperty(image, PropName(0x0018, 0x1242), "1");
    SetDicomProperty(image, PropName(0x0054, 0x1300), "30000");
    SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1078),
                     "20260430110000");

    CPPUNIT_ASSERT_THROW(
      mitk::DeduceDecayCorrection(image, /*halfLife=*/6586.26,
                                  mitk::DICOMReadPolicy::Strict),
      mitk::VendorEmpiricalDecayFallbackRefusedException);
  }

  void Start_Step5_OtherManufacturer_Throws_AmbiguousDecayTimingException()
  {
    // Unknown manufacturer with no private tag, no AcqTime == SeriesTime,
    // no per-slice frame timing. Spec is silent for this combination;
    // the helper must refuse rather than guess.
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, PropName(0x0054, 0x1102), "START");
    SetDicomProperty(image, PropName(0x0008, 0x0021), "20260430");
    SetDicomProperty(image, PropName(0x0008, 0x0031), "121530");
    SetDicomProperty(image, PropName(0x0008, 0x0070), "Acme Imaging Inc.");
    SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1078),
                     "20260430110000");

    CPPUNIT_ASSERT_THROW(mitk::DeduceDecayCorrection(image),
                         mitk::AmbiguousDecayTimingException);
  }

  // ---- GetManufacturerFamily ----

  void GetManufacturerFamily_Siemens_RecognizesSubstring()
  {
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, PropName(0x0008, 0x0070), "SIEMENS Healthineers");
    CPPUNIT_ASSERT_EQUAL(mitk::ManufacturerFamily::Siemens,
                         mitk::GetManufacturerFamily(image));
  }

  void GetManufacturerFamily_GE_RecognizesGEMedicalSystems()
  {
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, PropName(0x0008, 0x0070), "GE MEDICAL SYSTEMS");
    CPPUNIT_ASSERT_EQUAL(mitk::ManufacturerFamily::GE,
                         mitk::GetManufacturerFamily(image));

    auto image2 = MakeSyntheticImage(1, 1);
    SetDicomProperty(image2, PropName(0x0008, 0x0070), "GE HEALTHCARE");
    CPPUNIT_ASSERT_EQUAL(mitk::ManufacturerFamily::GE,
                         mitk::GetManufacturerFamily(image2));
  }

  void GetManufacturerFamily_Philips_RecognizesSubstring()
  {
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, PropName(0x0008, 0x0070), "Philips Medical Systems");
    CPPUNIT_ASSERT_EQUAL(mitk::ManufacturerFamily::Philips,
                         mitk::GetManufacturerFamily(image));
  }

  void GetManufacturerFamily_CaseInsensitive()
  {
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, PropName(0x0008, 0x0070), "  siemens  ");  // mixed case + whitespace
    CPPUNIT_ASSERT_EQUAL(mitk::ManufacturerFamily::Siemens,
                         mitk::GetManufacturerFamily(image));
  }

  void GetManufacturerFamily_OtherFallsThroughToOther()
  {
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, PropName(0x0008, 0x0070), "Acme Imaging Inc.");
    CPPUNIT_ASSERT_EQUAL(mitk::ManufacturerFamily::Other,
                         mitk::GetManufacturerFamily(image));

    auto empty = MakeSyntheticImage(1, 1);
    // No (0008,0070) tag attached -> Other.
    CPPUNIT_ASSERT_EQUAL(mitk::ManufacturerFamily::Other,
                         mitk::GetManufacturerFamily(empty));
  }

  void GetManufacturerFamily_NullProvider_ReturnsOther()
  {
    CPPUNIT_ASSERT_EQUAL(mitk::ManufacturerFamily::Other,
                         mitk::GetManufacturerFamily(nullptr));
  }

  // ---- DeduceDecayCorrection: NONE ----

  void None_PerSliceDecayTime()
  {
    // The IBSI-SUV-conformant DC=NONE path corrects to (t_acq + T_ave),
    // so the per-slice decay duration is (t_acq + T_ave - t_inj). We set
    // ActualFrameDuration to a tiny value (1 ms) so T_ave is at most
    // 0.5 ms -- well within the 1e-3 s assertion tolerance -- and the
    // expected per-slice durations remain the trivial (t_acq - t_inj).
    auto image = MakeSyntheticImage(/*nSlices=*/2, /*nTimeSteps=*/1);
    SetDicomProperty(image, PropName(0x0054, 0x1102), "NONE");

    // Each slice has its own acquisition timestamp.
    SetDicomProperty(image, PropName(0x0008, 0x0022), "20260430", 0, 0);
    SetDicomProperty(image, PropName(0x0008, 0x0032), "121000", 0, 0);  // 12:10:00
    SetDicomProperty(image, PropName(0x0008, 0x0022), "20260430", 0, 1);
    SetDicomProperty(image, PropName(0x0008, 0x0032), "122000", 0, 1);  // 12:20:00

    // 1 ms frame duration -> T_ave well below the 1e-3 s tolerance.
    SetDicomProperty(image, PropName(0x0018, 0x1242), "1", 0, 0);
    SetDicomProperty(image, PropName(0x0018, 0x1242), "1", 0, 1);

    SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1078),
                     "20260430110000");  // injection at 11:00:00

    const auto info = mitk::DeduceDecayCorrection(image, /*halfLife=*/6586.26);
    CPPUNIT_ASSERT_EQUAL(mitk::DecayCorrectionStrategy::None, info.strategy);
    // Slice 0: 12:10:00 - 11:00:00 = 4200 s. Slice 1: 12:20:00 - 11:00:00 = 4800 s.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4200.0, info.decayTimes.at(0).at(0), 1e-3);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4800.0, info.decayTimes.at(0).at(1), 1e-3);
    CPPUNIT_ASSERT(info.decayTimes.at(0).at(0) != info.decayTimes.at(0).at(1));
  }

  void None_TaveCorrection_Applied()
  {
    // Verify the +T_ave term is actually being added: pick a frame
    // duration where T_ave is large enough to be unambiguously detected
    // above the assertion tolerance.
    // T = 60 s, T_half = 6586.26 s -> T_ave = T/2 - lambda*T^2/24
    //                                       = 30 - 0.01578 = 29.984 s.
    auto image = MakeSyntheticImage(/*nSlices=*/1, /*nTimeSteps=*/1);
    SetDicomProperty(image, PropName(0x0054, 0x1102), "NONE");
    SetDicomProperty(image, PropName(0x0008, 0x0022), "20260430");
    SetDicomProperty(image, PropName(0x0008, 0x0032), "121000");      // 12:10:00
    SetDicomProperty(image, PropName(0x0018, 0x1242), "60000");       // 60 s frame
    SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1078),
                     "20260430110000");                               // injection 11:00:00

    const auto info = mitk::DeduceDecayCorrection(image, /*halfLife=*/6586.26);
    // Expected: (t_acq + T_ave) - t_inj = 4200 + 29.984 = 4229.984 s.
    // A regression that drops T_ave would yield 4200 s, off by 30 s
    // (~300x the 0.1 s tolerance).
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4229.984, info.decayTimes.at(0).at(0), 0.1);
  }

  void None_MissingFrameDuration_Throws_MissingDICOMPropertyException()
  {
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, PropName(0x0054, 0x1102), "NONE");
    SetDicomProperty(image, PropName(0x0008, 0x0022), "20260430");
    SetDicomProperty(image, PropName(0x0008, 0x0032), "121000");
    // (0018,0x1242) ActualFrameDuration deliberately absent.
    SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1078),
                     "20260430110000");

    CPPUNIT_ASSERT_THROW(mitk::DeduceDecayCorrection(image, 6586.26),
                         mitk::MissingDICOMPropertyException);
  }

  // ---- DeduceDecayCorrection: injection-time resolution ----

  void Prefers_1078_Over_1072()
  {
    // Same fixture as the Step 2 baseline; this test focuses on the
    // injection-time precedence (1078 over 1072), not on the DC=START
    // fallback chain.
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, PropName(0x0054, 0x1102), "START");
    SetDicomProperty(image, PropName(0x0008, 0x0021), "20260430");
    SetDicomProperty(image, PropName(0x0008, 0x0031), "121530");
    SetDicomProperty(image, PropName(0x0008, 0x0070), "SIEMENS");
    SetDicomProperty(image, PropName(0x0008, 0x0022), "20260430");
    SetDicomProperty(image, PropName(0x0008, 0x0032), "121530");
    // (0018,1078) -- should be used.
    SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1078),
                     "20260430110000");
    // (0018,1072) -- present but with deliberately *different* value; must be
    // ignored because (0018,1078) takes precedence.
    SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1072), "100000");

    const auto info = mitk::DeduceDecayCorrection(image);
    // If 1078 wins, decay = 4530 s. If 1072 was used (with date 20260430),
    // decay would be (12:15:30 - 10:00:00) = 8130 s.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(kStartExpectedDecaySeconds,
                                  info.decayTimes.at(0).at(0), 1e-3);
  }

  void Rollover_1072_Recovered()
  {
    // Acquisition today at 01:00:00, injection (TM-only) at 23:00:00.
    // Naive subtraction yields -22 h; rollover guard subtracts 24 h from the
    // injection day -> +2 h decay duration.
    // ActualFrameDuration is set to 1 ms so the +T_ave correction added
    // by the IBSI-SUV-conformant DC=NONE path is at most 0.5 ms -- well
    // within the 1e-3 s assertion tolerance.
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, PropName(0x0054, 0x1102), "NONE");
    SetDicomProperty(image, PropName(0x0008, 0x0022), "20260430");
    SetDicomProperty(image, PropName(0x0008, 0x0032), "010000");
    SetDicomProperty(image, PropName(0x0018, 0x1242), "1");                  // 1 ms frame
    SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1072), "230000");

    const auto info = mitk::DeduceDecayCorrection(image, /*halfLife=*/6586.26);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0 * 3600.0, info.decayTimes.at(0).at(0), 1e-3);
  }

  void Rollover_1078_NoSilentCorrection_Throws_AmbiguousDecayTimingException()
  {
    // (0018,1078) is unambiguous -- a negative delta must surface, not be
    // silently corrected by +/-24 h. Fixture exercises DC=START Step 2
    // (Manufacturer Siemens, AcqTime equals SeriesTime); the rollover
    // guard fires inside the per-slice loop when (acq - inj) is negative.
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, PropName(0x0054, 0x1102), "START");
    SetDicomProperty(image, PropName(0x0008, 0x0021), "20260430");
    SetDicomProperty(image, PropName(0x0008, 0x0031), "121530");
    SetDicomProperty(image, PropName(0x0008, 0x0070), "SIEMENS");
    SetDicomProperty(image, PropName(0x0008, 0x0022), "20260430");
    SetDicomProperty(image, PropName(0x0008, 0x0032), "121530");
    // Injection AFTER series time -> negative decay.
    SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1078),
                     "20260430130000");

    CPPUNIT_ASSERT_THROW(mitk::DeduceDecayCorrection(image),
                         mitk::AmbiguousDecayTimingException);
  }

  void Strategy_Start_MissingSeriesTime_Throws_MissingDICOMPropertyException()
  {
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, PropName(0x0054, 0x1102), "START");
    SetDicomProperty(image, PropName(0x0008, 0x0021), "20260430");
    // Series Time deliberately absent.
    SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1078),
                     "20260430110000");

    CPPUNIT_ASSERT_THROW(mitk::DeduceDecayCorrection(image),
                         mitk::MissingDICOMPropertyException);
  }

  void Strategy_None_MissingAcqTime_Throws_MissingDICOMPropertyException()
  {
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, PropName(0x0054, 0x1102), "NONE");
    SetDicomProperty(image, PropName(0x0008, 0x0022), "20260430");
    // Acquisition Time deliberately absent.
    SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1078),
                     "20260430110000");

    CPPUNIT_ASSERT_THROW(mitk::DeduceDecayCorrection(image),
                         mitk::MissingDICOMPropertyException);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkSUVCalculationHelper)
