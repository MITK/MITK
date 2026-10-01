/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <set>
#include <string>
#include <vector>

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
  bool HasRule(const mitk::DecayCorrectionInfo& info, mitk::SUVAdaptationRule rule)
  {
    return std::any_of(info.adaptations.cbegin(), info.adaptations.cend(),
                       [rule](const mitk::SUVAdaptation& a) { return rule == a.rule; });
  }

  // Most cases here only care about the parsed sequence, not about which
  // IBSI-SUV recommendations fired, so they funnel through this wrapper
  // rather than declaring an adaptation vector each time. The cases that do
  // care pass their own vector to mitk::GetRadiopharmaceuticalInfos.
  // Most weight cases do not care which recommendations fired, so they
  // funnel through this wrapper rather than declaring a record each time.
  double ReadWeight(const mitk::IPropertyProvider* provider,
                    mitk::DICOMReadPolicy policy = mitk::DICOMReadPolicy::Lenient)
  {
    std::vector<mitk::SUVAdaptation> ignoredAdaptations;
    return mitk::GetPatientsWeight(provider, policy, ignoredAdaptations);
  }

  std::vector<mitk::RadiopharmaceuticalInfo> ReadRPI(
    const mitk::IPropertyProvider* provider,
    mitk::DICOMReadPolicy policy = mitk::DICOMReadPolicy::Lenient)
  {
    std::vector<mitk::SUVAdaptation> ignoredAdaptations;
    return mitk::GetRadiopharmaceuticalInfos(provider, policy, ignoredAdaptations);
  }

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
  MITK_TEST(Start_NoReferenceTimeSource_Throws_AmbiguousDecayTimingException);
  MITK_TEST(None_PerSliceDecayTime);
  MITK_TEST(None_TaveCorrection_Applied);
  MITK_TEST(None_MissingFrameDuration_Throws_MissingDICOMPropertyException);
  MITK_TEST(Prefers_1078_Over_1072);
  MITK_TEST(Rollover_1072_Recovered);
  MITK_TEST(AdminWindow_AcquisitionOneHourEarly_Accepted);
  MITK_TEST(AdminWindow_JustBelowFloor_SubstitutesDate);
  MITK_TEST(AdminWindow_JustBelowTwoHalfLives_Accepted);
  MITK_TEST(AdminWindow_AtTwoHalfLives_SubstitutesDate);
  MITK_TEST(AdminWindow_HalfLifeJustBelowLimit_SubstitutesDate);
  MITK_TEST(AdminWindow_HalfLifeAtLimit_Refuses);
  MITK_TEST(AdminWindow_HalfLifeUnavailable_Refuses);
  MITK_TEST(AdminWindow_StrictPolicy_RefusesDateSubstitution);
  MITK_TEST(AdminWindow_StrictPolicy_RefusalIsCatchableAsBaseException);
  MITK_TEST(AdminTime_UtcOffsetHonoured);
  MITK_TEST(AdminTime_UnparseableStartDateTime_Throws);
  MITK_TEST(Start_Step1_NegativeOffset_DoesNotFallThroughToStep2);
  MITK_TEST(Strategy_Start_MissingSeriesTime_Throws_MissingDICOMPropertyException);
  MITK_TEST(Strategy_None_MissingAcqTime_Throws_MissingDICOMPropertyException);

  // Enhanced PET: the reference instant per slice
  MITK_TEST(Enhanced_DecayCorrectedNO_UsesFrameReferencePerSlice);
  MITK_TEST(Enhanced_DecayCorrectedYES_IgnoresFrameTimes);
  MITK_TEST(Enhanced_DecayCorrectedNO_FallsBackToAcquisitionPlusTAvePerSlice);
  MITK_TEST(Enhanced_DecayCorrectedNO_DatetimeMissingOnOneSlice_Throws);
  MITK_TEST(Enhanced_FramesUnresolvedByReader_Refuses);

  // DC=START rule gating and the adaptation record
  MITK_TEST(Start_Step2_UnrecognizedVendor_Computes);
  MITK_TEST(Start_Step3_UnrecognizedVendor_ComputesAndRecordsBothRules);
  MITK_TEST(Start_Step4_GE_WithoutActualFrameDuration_Computes);
  MITK_TEST(Start_Step4_GE_DoesNotApplyTAve);
  MITK_TEST(Start_Step3_NonGE_ActualFrameDurationAbsent_Refuses);
  MITK_TEST(Start_Step3_NonGE_ActualFrameDurationEmpty_Refuses);
  MITK_TEST(Start_MultiBed_GeneralRule_LaterBedsShareSeriesReference);
  MITK_TEST(Start_MultiBed_GE_LaterBedsShareSeriesReference);
  MITK_TEST(Start_MultiBed_LaterBedUnresolvable_Throws_AmbiguousDecayTimingException);
  MITK_TEST(Start_MultiBed_StrictPolicy_Refused);
  MITK_TEST(Start_Step2_Uniform_StrictPolicy_Computes);
  MITK_TEST(Start_MultiBed_SeriesMatchingBedIsNotSliceZero);
  MITK_TEST(Start_MultiBed_SlotMissingAcquisitionTime_DoesNotBorrowNeighbour);
  MITK_TEST(Start_SlotMissingAcquisitionTime_NotClassifiedAsSeriesTime);
  MITK_TEST(Start_Dynamic_LaterFramesShareSeriesReference);
  MITK_TEST(Start_UnparseableAcquisitionTime_Throws_InvalidDICOMPropertyValueException);
  MITK_TEST(Radiopharm_DoseBelowThreshold_RecordsAdaptation);
  MITK_TEST(Radiopharm_DoseAboveThreshold_RecordsNothing);

  // Patient's Weight gram encoding
  MITK_TEST(PatientWeight_BelowThreshold_TakenAsKilograms);
  MITK_TEST(PatientWeight_JustBelowThreshold_TakenAsKilograms);
  MITK_TEST(PatientWeight_AtThreshold_TakenAsGrams);
  MITK_TEST(PatientWeight_GramEncoded_TakenAsGrams);
  MITK_TEST(PatientWeight_GramEncoded_RecordsAdaptation);
  MITK_TEST(PatientWeight_PlausibleWeight_RecordsNothing);
  MITK_TEST(PatientWeight_GramEncoded_StrictPolicy_Throws);
  MITK_TEST(PatientWeight_PlausibleWeight_StrictPolicy_PassesThrough);

  // Adaptation record: naming, rendering, serialization
  MITK_TEST(RuleToString_EveryRuleHasADistinctStableName);
  MITK_TEST(FormatAdaptationSummary_EmptyRecord_IsEmpty);
  MITK_TEST(FormatAdaptationSummary_RendersTagAndTransition);
  MITK_TEST(FormatAdaptationSummary_OmitsAbsentTagAndOriginal);
  MITK_TEST(FormatAdaptationSummary_CountsEveryEntry);
  MITK_TEST(SerializeAdaptations_EmptyRecord_IsEmptyJSONArray);
  MITK_TEST(SerializeAdaptations_CarriesAllFourFields);
  MITK_TEST(FormatDerivationDescription_EmptyRecord_StillNamesTheDerivation);
  MITK_TEST(FormatDerivationDescription_CountLeadsAndAgreesInNumber);
  MITK_TEST(FormatDerivationDescription_NeverExceedsTheLOLimit);
  MITK_TEST(RecordAdaptation_StrictPolicy_RefusesEvenWithoutARecord);
  MITK_TEST(RecordAdaptation_LenientPolicy_AppendsAndToleratesNullRecord);

  // Rescale plausibility diagnostics
  MITK_TEST(Rescale_PlausibleValues_NoFindings);
  MITK_TEST(Rescale_AbsentSlope_IsReported);
  MITK_TEST(Rescale_NonPositiveSlope_IsReported);
  MITK_TEST(Rescale_NonZeroIntercept_IsReported);
  MITK_TEST(Rescale_BothObjectionable_ReportsBoth);
  MITK_TEST(Rescale_NullProvider_NoFindings);

  CPPUNIT_TEST_SUITE_END();

private:

  // Most decay-correction tests share a common acquisition-day setup.
  // The half-life is part of the fixture because every DC=START path
  // resolves the administration time through an acceptance window of
  // 2 * T_half, so there is no rule to apply without one.
  // Series Time 12:15:30, injection (1078) on the same day at 11:00:00 ->
  // SeriesTime - InjectionTime = (12*3600+15*60+30) - (11*3600) = 4530 s.
  // After the IBSI-SUV-conformant DC=START rewrite this fixture exercises
  // Step 2 of the fallback chain: Manufacturer is set to "SIEMENS" and
  // AcquisitionDate / AcquisitionTime equal SeriesDate / SeriesTime, so
  // the helper picks per-slice AcquisitionTime as the reference. The
  // numeric expectation is unchanged because AcqTime equals SeriesTime.
  static constexpr double kStartExpectedDecaySeconds = 4530.0;

  // Fixture for the cases that must reach Steps 3/4: AcquisitionTime is
  // present but differs from SeriesTime, so Step 2 cannot fire. Acquisition
  // 12:10:00 against an 11:00:00 injection gives t_acq - t_inj = 4200 s;
  // with FrameReferenceTime 600 s the pure GE shift lands on 3600 s.
  // ActualFrameDuration is deliberately left out -- the cases that need it
  // set it themselves, and its absence is itself under test.
  void SetupStep34Case(mitk::Image* image, const char* manufacturer)
  {
    SetDicomProperty(image, PropName(0x0054, 0x1102), "START");
    SetDicomProperty(image, PropName(0x0008, 0x0021), "20260430");          // SeriesDate
    SetDicomProperty(image, PropName(0x0008, 0x0031), "121530");            // SeriesTime
    SetDicomProperty(image, PropName(0x0008, 0x0070), manufacturer);
    SetDicomProperty(image, PropName(0x0008, 0x0022), "20260430");          // AcqDate
    SetDicomProperty(image, PropName(0x0008, 0x0032), "121000");            // AcqTime != SeriesTime
    SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1078),
                     "20260430110000");                                     // RP Start DT
    SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1075),
                     "6586.26");                                            // Half-life [s]
    SetDicomProperty(image, PropName(0x0054, 0x1300), "600000");            // FrameReferenceTime [ms]
  }

  static constexpr double kStep4ExpectedDecaySeconds = 3600.0;

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
    SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1075),
                     "6586.26");                                            // Half-life [s]
  }

  // Closed-form average count-rate time of a frame, mirroring the
  // production formula so the multi-bed cases can place a later bed's
  // FrameReferenceTime exactly on the series reference.
  static double TAveSeconds(double frameDurationSeconds, double halfLifeSeconds)
  {
    const double lambda = std::log(2.0) / halfLifeSeconds;
    const double lambdaT = lambda * frameDurationSeconds;
    return std::log(lambdaT / -std::expm1(-lambdaT)) / lambda;
  }

  static constexpr double kBedDurationSeconds = 300.0;

  // One bed of a DC=START whole-body series, as a scanner that corrects
  // every bed to the series start writes it: the bed starts lagSeconds
  // after SeriesTime (12:15:30) and its FrameReferenceTime reaches from
  // SeriesTime to the bed's decay-equivalent midpoint. The general rule then
  // lands every bed on SeriesTime, i.e. on kStartExpectedDecaySeconds. The
  // GE rule carries no T_ave term, so GE beds pass withTAve = false.
  void SetBed(mitk::Image* image, mitk::TimeStepType t, unsigned int s,
              int lagSeconds, bool withTAve)
  {
    const int acqSecondsOfDay = 12 * 3600 + 15 * 60 + 30 + lagSeconds;
    char acqTime[16];
    std::snprintf(acqTime, sizeof(acqTime), "%02d%02d%02d", acqSecondsOfDay / 3600,
                  (acqSecondsOfDay / 60) % 60, acqSecondsOfDay % 60);
    const double frameRefSeconds =
      lagSeconds + (withTAve ? TAveSeconds(kBedDurationSeconds, kF18HalfLifeSeconds) : 0.0);

    SetDicomProperty(image, PropName(0x0008, 0x0022), "20260430", t, s);
    SetDicomProperty(image, PropName(0x0008, 0x0032), acqTime, t, s);
    SetDicomProperty(image, PropName(0x0054, 0x1300),
                     std::to_string(frameRefSeconds * 1000.0), t, s);
  }

  void SetBedDuration(mitk::Image* image, mitk::TimeStepType t, unsigned int s)
  {
    SetDicomProperty(image, PropName(0x0018, 0x1242),
                     std::to_string(kBedDurationSeconds * 1000.0), t, s);
  }

  // Three-bed SIEMENS series whose first bed starts at SeriesTime.
  mitk::Image::Pointer MakeThreeBedGeneralRuleImage()
  {
    auto image = MakeSyntheticImage(/*nSlices=*/3, /*nTimeSteps=*/1);
    SetupCommonStartCase(image);
    for (unsigned int s = 0; s < 3; ++s)
    {
      SetBed(image, 0, s, static_cast<int>(s) * 300, /*withTAve=*/true);
      SetBedDuration(image, 0, s);
    }
    return image;
  }

  static std::size_t CountRule(const mitk::DecayCorrectionInfo& info, mitk::SUVAdaptationRule rule)
  {
    return static_cast<std::size_t>(
      std::count_if(info.adaptations.cbegin(), info.adaptations.cend(),
                    [rule](const mitk::SUVAdaptation& a) { return rule == a.rule; }));
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
    CPPUNIT_ASSERT(ReadRPI(image).empty());
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

    const auto infos = ReadRPI(image);
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

    const auto infos = ReadRPI(image);
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

    const auto infos = ReadRPI(image);
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

    const auto infos = ReadRPI(image);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), infos.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.6808e8, infos[0].totalDoseBq, 1.0);
  }

  void RadionuclideTotalDose_AboveThreshold_PassesThrough()
  {
    // A plausible Bq-magnitude FDG dose must not be touched.
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1074), "3.6808e8");

    const auto infos = ReadRPI(image);
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

    const auto infos = ReadRPI(image);
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
      const auto infos = ReadRPI(image);
      CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), infos.size());
      CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, infos[0].totalDoseBq, 0.0);
    }

    {
      auto image = MakeSyntheticImage(1, 1);
      SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1074), "-5");
      const auto infos = ReadRPI(image);
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
      ReadRPI(image, mitk::DICOMReadPolicy::Strict),
      mitk::ImplausibleRadionuclideDoseException);
  }

  void RadionuclideTotalDose_BelowThreshold_StrictPolicy_CatchableAsBaseException()
  {
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1074), "368.08");

    CPPUNIT_ASSERT_THROW(
      ReadRPI(image, mitk::DICOMReadPolicy::Strict),
      mitk::BenchmarkAdaptationRequiredException);
  }

  void RadionuclideTotalDose_AboveThreshold_StrictPolicy_PassesThrough()
  {
    // Strict policy must not penalise plausible Bq-magnitude inputs.
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1074), "3.6808e8");

    const auto infos = ReadRPI(image, mitk::DICOMReadPolicy::Strict);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), infos.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3.6808e8, infos[0].totalDoseBq, 1.0);
  }

  void PatientWeight_Found()
  {
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, PropName(0x0010, 0x1030), "70.5");

    CPPUNIT_ASSERT_DOUBLES_EQUAL(70.5, ReadWeight(image), 1e-9);
  }

  void PatientWeight_Missing_Throws_MissingDICOMPropertyException()
  {
    auto image = MakeSyntheticImage(1, 1);
    CPPUNIT_ASSERT_THROW(ReadWeight(image),
                         mitk::MissingDICOMPropertyException);
  }

  // ---- Patient's Weight, gram-encoded exports ----
  //
  // DICOM prescribes kilograms for (0010,1030). Exports storing grams exist,
  // and a value at or above 1000 identifies one: nobody weighs a tonne. Read
  // at face value the weight is 1000x too large and every SUV derived from it
  // 1000x too small.
  //
  // The boundary is stated in the unit the helper returns, kilograms, so the
  // reinterpretation is visible as a change of value rather than of unit.
  // No benchmark DRO carries a gram-encoded weight -- the manual says so
  // outright -- which makes these cases the only coverage.

  void PatientWeight_BelowThreshold_TakenAsKilograms()
  {
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, PropName(0x0010, 0x1030), "70");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(70.0, ReadWeight(image), 1e-9);
  }

  void PatientWeight_JustBelowThreshold_TakenAsKilograms()
  {
    // 999 kg is not a plausible patient either, but the recommendation draws
    // the line at 1000 and MITK does not second-guess it: a threshold that
    // moves with the reader is worse than one that is merely generous.
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, PropName(0x0010, 0x1030), "999");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(999.0, ReadWeight(image), 1e-9);
  }

  void PatientWeight_AtThreshold_TakenAsGrams()
  {
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, PropName(0x0010, 0x1030), "1000");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, ReadWeight(image), 1e-9);
  }

  void PatientWeight_GramEncoded_TakenAsGrams()
  {
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, PropName(0x0010, 0x1030), "70000");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(70.0, ReadWeight(image), 1e-9);
  }

  void PatientWeight_GramEncoded_RecordsAdaptation()
  {
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, PropName(0x0010, 0x1030), "70000");

    std::vector<mitk::SUVAdaptation> adaptations;
    const double weightKg =
      mitk::GetPatientsWeight(image, mitk::DICOMReadPolicy::Lenient, adaptations);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(70.0, weightKg, 1e-9);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), adaptations.size());
    CPPUNIT_ASSERT(mitk::SUVAdaptationRule::WeightReinterpretedAsGrams == adaptations[0].rule);
    CPPUNIT_ASSERT_EQUAL(std::string("(0010,1030)"), adaptations[0].dicomTag);
    CPPUNIT_ASSERT_EQUAL(std::string("70000"), adaptations[0].originalValue);
  }

  void PatientWeight_PlausibleWeight_RecordsNothing()
  {
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, PropName(0x0010, 0x1030), "70");

    std::vector<mitk::SUVAdaptation> adaptations;
    (void)mitk::GetPatientsWeight(image, mitk::DICOMReadPolicy::Lenient, adaptations);
    CPPUNIT_ASSERT(adaptations.empty());
  }

  void PatientWeight_GramEncoded_StrictPolicy_Throws()
  {
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, PropName(0x0010, 0x1030), "70000");

    CPPUNIT_ASSERT_THROW(ReadWeight(image, mitk::DICOMReadPolicy::Strict),
                         mitk::ImplausiblePatientWeightException);
    CPPUNIT_ASSERT_THROW(ReadWeight(image, mitk::DICOMReadPolicy::Strict),
                         mitk::BenchmarkAdaptationRequiredException);
  }

  void PatientWeight_PlausibleWeight_StrictPolicy_PassesThrough()
  {
    // Strict must not penalise input that needs no reinterpretation.
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, PropName(0x0010, 0x1030), "70");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(70.0, ReadWeight(image, mitk::DICOMReadPolicy::Strict), 1e-9);
  }

  // ---- The adaptation record: naming, rendering, serialization ----
  //
  // These three renderings are what leaves the process -- the persisted
  // property, the (0008,2111) description and the operator-facing summary --
  // so they are pinned independently of the rules that produce them.

  void RuleToString_EveryRuleHasADistinctStableName()
  {
    // Distinctness matters because the names are the persisted form: two
    // rules sharing one would make a saved record ambiguous. The literals
    // are spelled out rather than derived so that renaming a rule fails
    // here, where the compatibility break is visible, and not silently.
    const std::vector<std::pair<mitk::SUVAdaptationRule, std::string>> expected{
      {mitk::SUVAdaptationRule::DoseReinterpretedAsMBq, "DoseReinterpretedAsMBq"},
      {mitk::SUVAdaptationRule::VendorEmpiricalDecayFallback, "VendorEmpiricalDecayFallback"},
      {mitk::SUVAdaptationRule::UnrecognizedManufacturer, "UnrecognizedManufacturer"},
      {mitk::SUVAdaptationRule::AmbiguousPatientSexMeanOfMaleAndFemale,
       "AmbiguousPatientSexMeanOfMaleAndFemale"},
      {mitk::SUVAdaptationRule::AdministrationDateFromReferenceWithStartDateTime,
       "AdministrationDateFromReferenceWithStartDateTime"},
      {mitk::SUVAdaptationRule::AdministrationDateFromReferenceWithStartTime,
       "AdministrationDateFromReferenceWithStartTime"},
      {mitk::SUVAdaptationRule::AdministrationTimeShiftedBackOneDay,
       "AdministrationTimeShiftedBackOneDay"},
      {mitk::SUVAdaptationRule::WeightReinterpretedAsGrams, "WeightReinterpretedAsGrams"}};

    std::set<std::string> seen;
    for (const auto& [rule, name] : expected)
    {
      CPPUNIT_ASSERT_EQUAL(name, std::string(mitk::SUVAdaptationRuleToString(rule)));
      CPPUNIT_ASSERT_MESSAGE("duplicate rule name: " + name, seen.insert(name).second);
    }

    // Guards the list above against a rule added to the enum but not here.
    // kRuleCount must be the last rule's value plus one.
    constexpr auto kRuleCount =
      static_cast<size_t>(mitk::SUVAdaptationRule::WeightReinterpretedAsGrams) + 1u;
    CPPUNIT_ASSERT_EQUAL(kRuleCount, expected.size());
  }

  void FormatAdaptationSummary_EmptyRecord_IsEmpty()
  {
    // So a caller can print it unconditionally and stay silent.
    CPPUNIT_ASSERT(mitk::FormatAdaptationSummary({}).empty());
  }

  void FormatAdaptationSummary_RendersTagAndTransition()
  {
    const std::vector<mitk::SUVAdaptation> adaptations{
      {mitk::SUVAdaptationRule::WeightReinterpretedAsGrams, "(0010,1030)", "70000", "70"}};

    const std::string summary = mitk::FormatAdaptationSummary(adaptations);
    CPPUNIT_ASSERT_EQUAL(
      std::string("IBSI-SUV input adaptations applied (1):"
                  "\n  - WeightReinterpretedAsGrams (0010,1030): 70000 -> 70"),
      summary);
  }

  void FormatAdaptationSummary_OmitsAbsentTagAndOriginal()
  {
    // The vendor fallback concerns no single tag and reinterprets no stored
    // value; it records only which rule resolved the reference time. The
    // summary must not invent an empty tag or a "-> " with nothing before it.
    const std::vector<mitk::SUVAdaptation> adaptations{
      {mitk::SUVAdaptationRule::VendorEmpiricalDecayFallback, "", "", "Step 4"}};

    CPPUNIT_ASSERT_EQUAL(
      std::string("IBSI-SUV input adaptations applied (1):"
                  "\n  - VendorEmpiricalDecayFallback: Step 4"),
      mitk::FormatAdaptationSummary(adaptations));
  }

  void FormatAdaptationSummary_CountsEveryEntry()
  {
    const std::vector<mitk::SUVAdaptation> adaptations{
      {mitk::SUVAdaptationRule::DoseReinterpretedAsMBq, "(0018,1074)", "400", "400000000"},
      {mitk::SUVAdaptationRule::WeightReinterpretedAsGrams, "(0010,1030)", "70000", "70"}};

    const std::string summary = mitk::FormatAdaptationSummary(adaptations);
    CPPUNIT_ASSERT(summary.find("(2):") != std::string::npos);
    CPPUNIT_ASSERT(summary.find("DoseReinterpretedAsMBq") != std::string::npos);
    CPPUNIT_ASSERT(summary.find("WeightReinterpretedAsGrams") != std::string::npos);
  }

  void SerializeAdaptations_EmptyRecord_IsEmptyJSONArray()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("[]"), mitk::SerializeAdaptations({}));
  }

  void SerializeAdaptations_CarriesAllFourFields()
  {
    const std::vector<mitk::SUVAdaptation> adaptations{
      {mitk::SUVAdaptationRule::DoseReinterpretedAsMBq, "(0018,1074)", "400", "400000000"}};

    CPPUNIT_ASSERT_EQUAL(
      std::string("[{\"dicomTag\":\"(0018,1074)\",\"originalValue\":\"400\","
                  "\"rule\":\"DoseReinterpretedAsMBq\",\"usedValue\":\"400000000\"}]"),
      mitk::SerializeAdaptations(adaptations));
  }

  void FormatDerivationDescription_EmptyRecord_StillNamesTheDerivation()
  {
    // The image is derived whether or not anything was adapted, and
    // (0008,2111) is where a viewer looks for that.
    CPPUNIT_ASSERT_EQUAL(std::string("MITK SUV"), mitk::FormatDerivationDescription({}));
  }

  void FormatDerivationDescription_CountLeadsAndAgreesInNumber()
  {
    const std::vector<mitk::SUVAdaptation> one{
      {mitk::SUVAdaptationRule::WeightReinterpretedAsGrams, "(0010,1030)", "70000", "70"}};
    const std::vector<mitk::SUVAdaptation> two{
      one[0], {mitk::SUVAdaptationRule::DoseReinterpretedAsMBq, "(0018,1074)", "400", "4e8"}};

    CPPUNIT_ASSERT_EQUAL(std::string("MITK SUV; 1 IBSI-SUV input adaptation applied"),
                         mitk::FormatDerivationDescription(one));
    CPPUNIT_ASSERT_EQUAL(std::string("MITK SUV; 2 IBSI-SUV input adaptations applied"),
                         mitk::FormatDerivationDescription(two));
  }

  void FormatDerivationDescription_NeverExceedsTheLOLimit()
  {
    // (0008,2111) is LO. A description that overran it would be rejected or
    // truncated by whatever writes the DICOM object, outside MITK's control.
    std::vector<mitk::SUVAdaptation> many(
      99u, {mitk::SUVAdaptationRule::WeightReinterpretedAsGrams, "(0010,1030)", "70000", "70"});

    CPPUNIT_ASSERT(mitk::FormatDerivationDescription(many).size() <= 64u);
    CPPUNIT_ASSERT(mitk::FormatDerivationDescription({}).size() <= 64u);
  }

  // ---- The Strict backstop ----

  void RecordAdaptation_StrictPolicy_RefusesEvenWithoutARecord()
  {
    // Every rule has a refusal of its own that fires before this point, so
    // reaching it means one was added without a gate. The backstop exists so
    // that mistake surfaces as a loud generic refusal rather than as a
    // silent adaptation under the policy whose entire purpose is to forbid
    // adaptation. The null record is the case that matters: the policy must
    // be checked before the caller's interest in collecting the record.
    std::vector<mitk::SUVAdaptation> adaptations;
    CPPUNIT_ASSERT_THROW(
      mitk::RecordAdaptation(&adaptations, mitk::DICOMReadPolicy::Strict,
                             mitk::SUVAdaptationRule::WeightReinterpretedAsGrams, "", "", ""),
      mitk::BenchmarkAdaptationRequiredException);
    CPPUNIT_ASSERT_THROW(
      mitk::RecordAdaptation(nullptr, mitk::DICOMReadPolicy::Strict,
                             mitk::SUVAdaptationRule::WeightReinterpretedAsGrams, "", "", ""),
      mitk::BenchmarkAdaptationRequiredException);
    CPPUNIT_ASSERT(adaptations.empty());
  }

  void RecordAdaptation_LenientPolicy_AppendsAndToleratesNullRecord()
  {
    std::vector<mitk::SUVAdaptation> adaptations;
    mitk::RecordAdaptation(&adaptations, mitk::DICOMReadPolicy::Lenient,
                           mitk::SUVAdaptationRule::DoseReinterpretedAsMBq,
                           "(0018,1074)", "400", "400000000");

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), adaptations.size());
    CPPUNIT_ASSERT(mitk::SUVAdaptationRule::DoseReinterpretedAsMBq == adaptations[0].rule);
    CPPUNIT_ASSERT_EQUAL(std::string("400000000"), adaptations[0].usedValue);

    // A caller that does not collect the record is a supported case.
    CPPUNIT_ASSERT_NO_THROW(
      mitk::RecordAdaptation(nullptr, mitk::DICOMReadPolicy::Lenient,
                             mitk::SUVAdaptationRule::DoseReinterpretedAsMBq, "", "", ""));
  }

  // ---- Rescale plausibility ----
  //
  // Diagnostics, not adaptations: the reader has already applied these to the
  // pixel buffer, so nothing here changes a value. They are returned rather
  // than only logged so a front end can put them in front of the operator,
  // which is the only reason they are testable at all.

  void Rescale_PlausibleValues_NoFindings()
  {
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, PropName(0x0028, 0x1053), "1.0");
    SetDicomProperty(image, PropName(0x0028, 0x1052), "0.0");
    CPPUNIT_ASSERT(mitk::CheckRescalePlausibility(image).empty());
  }

  void Rescale_AbsentSlope_IsReported()
  {
    // An absent slope is silently treated as 1.0, which is the case worth
    // knowing about: the values look scaled and may not be.
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, PropName(0x0028, 0x1052), "0.0");

    const auto findings = mitk::CheckRescalePlausibility(image);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), findings.size());
    CPPUNIT_ASSERT(findings[0].find("(0028,1053)") != std::string::npos);
  }

  void Rescale_NonPositiveSlope_IsReported()
  {
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, PropName(0x0028, 0x1053), "-2.0");
    SetDicomProperty(image, PropName(0x0028, 0x1052), "0.0");

    const auto findings = mitk::CheckRescalePlausibility(image);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), findings.size());
    CPPUNIT_ASSERT(findings[0].find("-2.0") != std::string::npos);
  }

  void Rescale_NonZeroIntercept_IsReported()
  {
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, PropName(0x0028, 0x1053), "1.0");
    SetDicomProperty(image, PropName(0x0028, 0x1052), "-1024");

    const auto findings = mitk::CheckRescalePlausibility(image);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), findings.size());
    CPPUNIT_ASSERT(findings[0].find("(0028,1052)") != std::string::npos);
    CPPUNIT_ASSERT(findings[0].find("-1024") != std::string::npos);
  }

  void Rescale_BothObjectionable_ReportsBoth()
  {
    // Each tag is judged on its own; one bad value must not mask the other.
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, PropName(0x0028, 0x1053), "0");
    SetDicomProperty(image, PropName(0x0028, 0x1052), "5");
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), mitk::CheckRescalePlausibility(image).size());
  }

  void Rescale_NullProvider_NoFindings()
  {
    CPPUNIT_ASSERT(mitk::CheckRescalePlausibility(nullptr).empty());
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
    // SetupCommonStartCase populates (0,0) only. The per-slot reads are
    // exact, as the reader publishes a value at every slot, so every slot
    // carries its own AcqDate/AcqTime.
    for (unsigned int s = 0; s < 3; ++s)
    {
      SetDicomProperty(image, PropName(0x0008, 0x0022), "20260430", 0, s);
      SetDicomProperty(image, PropName(0x0008, 0x0032), "121530", 0, s);
    }

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
    // reads AcqDate/AcqTime per (t, s) without close-match fallback, so
    // every slot carries its own value, as the reader publishes it.
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
    SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1075),
                     "6586.26");                                            // Half-life [s]
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
    SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1075),
                     "6586.26");                                            // Half-life [s]
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
    SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1075),
                     "6586.26");                                            // Half-life [s]
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

  void Start_NoReferenceTimeSource_Throws_AmbiguousDecayTimingException()
  {
    // No private datetime, no AcquisitionTime at all, so neither the
    // AcqTime == SeriesTime rule nor the frame-timing formulas have
    // anything to work with and the chain is exhausted. The manufacturer
    // is incidental here: since v3.0.1 only the private-datetime rules and
    // the GE formula are manufacturer-specific, so an unrecognized vendor
    // is no longer a reason to refuse on its own.
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

  // ---- DC=START rule gating, IBSI-SUV v3.0.1 ----
  //
  // Only three rules are manufacturer-specific: the two private-datetime
  // rules and the GE reference-time formula. The AcqTime == SeriesTime rule
  // and the general T_ave formula apply to any scanner. These cases pin
  // both halves of that split, because a vendor gate that creeps back would
  // be invisible in the benchmark until a non-Siemens/GE/Philips export
  // appears.

  void Start_Step2_UnrecognizedVendor_Computes()
  {
    auto image = MakeSyntheticImage(1, 1);
    SetupCommonStartCase(image);
    SetDicomProperty(image, PropName(0x0008, 0x0070), "SYNTHETIC");

    const auto info = mitk::DeduceDecayCorrection(image);
    CPPUNIT_ASSERT_EQUAL(mitk::DecayCorrectionStrategy::Start, info.strategy);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(kStartExpectedDecaySeconds,
                                 info.decayTimes.at(0).at(0), 1e-6);
    // An AcquisitionTime that already equals the SeriesTime names the
    // reference instant outright, so nothing was reinterpreted.
    CPPUNIT_ASSERT(info.adaptations.empty());
  }

  void Start_Step3_UnrecognizedVendor_ComputesAndRecordsBothRules()
  {
    auto image = MakeSyntheticImage(1, 1);
    SetupStep34Case(image, "SYNTHETIC");
    SetDicomProperty(image, PropName(0x0018, 0x1242), "300000");  // ActualFrameDuration [ms]

    const auto info = mitk::DeduceDecayCorrection(image);
    // T_ave is strictly positive, so the general formula lands above the
    // pure -FrameReferenceTime shift rather than on it.
    CPPUNIT_ASSERT(info.decayTimes.at(0).at(0) > kStep4ExpectedDecaySeconds);
    CPPUNIT_ASSERT(HasRule(info, mitk::SUVAdaptationRule::VendorEmpiricalDecayFallback));
    CPPUNIT_ASSERT(HasRule(info, mitk::SUVAdaptationRule::UnrecognizedManufacturer));
  }

  void Start_Step4_GE_WithoutActualFrameDuration_Computes()
  {
    // The GE rule is a pure -FrameReferenceTime shift with no T_ave term,
    // so it needs no (0018,1242). Requiring the tag for both formulas
    // refused GE studies the rule resolves perfectly well. No benchmark DRO
    // covers this combination, so this case is the only thing guarding it.
    auto image = MakeSyntheticImage(1, 1);
    SetupStep34Case(image, "GE MEDICAL SYSTEMS");

    const auto info = mitk::DeduceDecayCorrection(image);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(kStep4ExpectedDecaySeconds,
                                 info.decayTimes.at(0).at(0), 1e-6);
    CPPUNIT_ASSERT(HasRule(info, mitk::SUVAdaptationRule::VendorEmpiricalDecayFallback));
    // GE is recognized, so the unverified-vendor note must stay silent.
    CPPUNIT_ASSERT(!HasRule(info, mitk::SUVAdaptationRule::UnrecognizedManufacturer));
  }

  void Start_Step4_GE_DoesNotApplyTAve()
  {
    // Identical tags, two manufacturers. Dropping the vendor gate from the
    // general formula must not also widen it over GE: the two formulas
    // differ by exactly T_ave, and confusing them would skew every GE study.
    auto geImage = MakeSyntheticImage(1, 1);
    SetupStep34Case(geImage, "GE MEDICAL SYSTEMS");
    SetDicomProperty(geImage, PropName(0x0018, 0x1242), "300000");

    auto otherImage = MakeSyntheticImage(1, 1);
    SetupStep34Case(otherImage, "SIEMENS");
    SetDicomProperty(otherImage, PropName(0x0018, 0x1242), "300000");

    const double ge    = mitk::DeduceDecayCorrection(geImage).decayTimes.at(0).at(0);
    const double other = mitk::DeduceDecayCorrection(otherImage).decayTimes.at(0).at(0);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(kStep4ExpectedDecaySeconds, ge, 1e-6);
    CPPUNIT_ASSERT(other > ge);
  }

  void Start_Step3_NonGE_ActualFrameDurationAbsent_Refuses()
  {
    // DRO_error_3_2's shape. Once the vendor gate is gone, the
    // ActualFrameDuration precondition is the only thing left that makes
    // this input refuse, so it is worth pinning directly rather than
    // through the benchmark.
    auto image = MakeSyntheticImage(1, 1);
    SetupStep34Case(image, "SIEMENS");

    CPPUNIT_ASSERT_THROW(mitk::DeduceDecayCorrection(image),
                         mitk::AmbiguousDecayTimingException);
  }

  void Start_Step3_NonGE_ActualFrameDurationEmpty_Refuses()
  {
    // The same refusal reached by a different guard: an absent tag fails
    // the presence check, a present-but-empty one fails the per-slice
    // numeric check. The upstream DRO omits the tag; real exports ship it
    // empty, so both shapes need coverage.
    auto image = MakeSyntheticImage(1, 1);
    SetupStep34Case(image, "SIEMENS");
    SetDicomProperty(image, PropName(0x0018, 0x1242), "");

    CPPUNIT_ASSERT_THROW(mitk::DeduceDecayCorrection(image),
                         mitk::AmbiguousDecayTimingException);
  }

  // ---- DC=START on multi-bed and dynamic series ----
  //
  // The scanner corrects the whole series to one instant, so every bed or
  // frame must be decay-corrected to that same instant. The AcquisitionTime
  // == SeriesTime rule identifies it only on the bed or frame that starts at
  // SeriesTime; the others have to reach it through the frame-timing
  // formulas. Correcting a later bed to its own start counts the inter-bed
  // decay twice. No benchmark DRO has this shape, so these cases are its
  // only guard.

  void Start_MultiBed_GeneralRule_LaterBedsShareSeriesReference()
  {
    auto image = MakeThreeBedGeneralRuleImage();

    const auto info = mitk::DeduceDecayCorrection(image, kF18HalfLifeSeconds);
    for (unsigned int s = 0; s < 3; ++s)
    {
      CPPUNIT_ASSERT_DOUBLES_EQUAL(kStartExpectedDecaySeconds, info.decayTimes.at(0).at(s), 1e-3);
    }
    // Two beds used the empirical formula; the record states once that it
    // fired.
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1),
                         CountRule(info, mitk::SUVAdaptationRule::VendorEmpiricalDecayFallback));
  }

  void Start_MultiBed_GE_LaterBedsShareSeriesReference()
  {
    auto image = MakeSyntheticImage(/*nSlices=*/2, /*nTimeSteps=*/1);
    SetupCommonStartCase(image);
    SetDicomProperty(image, PropName(0x0008, 0x0070), "GE MEDICAL SYSTEMS");
    SetBed(image, 0, 0, 0, /*withTAve=*/false);
    SetBed(image, 0, 1, 182, /*withTAve=*/false);

    const auto info = mitk::DeduceDecayCorrection(image, kF18HalfLifeSeconds);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(kStartExpectedDecaySeconds, info.decayTimes.at(0).at(0), 1e-3);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(kStartExpectedDecaySeconds, info.decayTimes.at(0).at(1), 1e-3);
  }

  void Start_MultiBed_LaterBedUnresolvable_Throws_AmbiguousDecayTimingException()
  {
    // Not GE and no ActualFrameDuration: the first bed matches SeriesTime,
    // the second has no rule that places it. Resolving the first must not
    // license a guess for the second.
    auto image = MakeSyntheticImage(/*nSlices=*/2, /*nTimeSteps=*/1);
    SetupCommonStartCase(image);
    SetBed(image, 0, 0, 0, /*withTAve=*/true);
    SetBed(image, 0, 1, 300, /*withTAve=*/true);

    CPPUNIT_ASSERT_THROW(mitk::DeduceDecayCorrection(image, kF18HalfLifeSeconds),
                         mitk::AmbiguousDecayTimingException);
  }

  void Start_MultiBed_StrictPolicy_Refused()
  {
    auto image = MakeThreeBedGeneralRuleImage();

    CPPUNIT_ASSERT_THROW(
      mitk::DeduceDecayCorrection(image, kF18HalfLifeSeconds, mitk::DICOMReadPolicy::Strict),
      mitk::VendorEmpiricalDecayFallbackRefusedException);
  }

  void Start_Step2_Uniform_StrictPolicy_Computes()
  {
    // Frame-timing tags are present, but no slot needs them: Strict refuses
    // a formula that is actually applied, not one that merely could be.
    auto image = MakeSyntheticImage(/*nSlices=*/2, /*nTimeSteps=*/1);
    SetupCommonStartCase(image);
    for (unsigned int s = 0; s < 2; ++s)
    {
      SetBed(image, 0, s, 0, /*withTAve=*/true);
      SetBedDuration(image, 0, s);
    }

    const auto info =
      mitk::DeduceDecayCorrection(image, kF18HalfLifeSeconds, mitk::DICOMReadPolicy::Strict);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(kStartExpectedDecaySeconds, info.decayTimes.at(0).at(0), 1e-6);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(kStartExpectedDecaySeconds, info.decayTimes.at(0).at(1), 1e-6);
    CPPUNIT_ASSERT(info.adaptations.empty());
  }

  void Start_MultiBed_SeriesMatchingBedIsNotSliceZero()
  {
    // Slice order need not follow bed order. The bed starting at SeriesTime
    // is slice 1 here, and it carries no frame timing at all: a slot the
    // AcquisitionTime == SeriesTime rule resolves must not be held to the
    // formulas' preconditions.
    auto image = MakeSyntheticImage(/*nSlices=*/2, /*nTimeSteps=*/1);
    SetupCommonStartCase(image);
    SetBed(image, 0, 0, 300, /*withTAve=*/true);
    SetBedDuration(image, 0, 0);
    SetDicomProperty(image, PropName(0x0008, 0x0022), "20260430", 0, 1);
    SetDicomProperty(image, PropName(0x0008, 0x0032), "121530", 0, 1);
    SetDicomProperty(image, PropName(0x0054, 0x1300), "", 0, 1);
    SetDicomProperty(image, PropName(0x0018, 0x1242), "", 0, 1);

    const auto info = mitk::DeduceDecayCorrection(image, kF18HalfLifeSeconds);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(kStartExpectedDecaySeconds, info.decayTimes.at(0).at(0), 1e-3);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(kStartExpectedDecaySeconds, info.decayTimes.at(0).at(1), 1e-6);
  }

  void Start_MultiBed_SlotMissingAcquisitionTime_DoesNotBorrowNeighbour()
  {
    // Slice 1 is the bed that starts at SeriesTime, but it has no
    // AcquisitionDate/Time of its own. Reading slice 0's instead would place
    // it 300 s off and still produce a plausible number, so the missing slot
    // has to be refused.
    auto image = MakeSyntheticImage(/*nSlices=*/2, /*nTimeSteps=*/1);
    SetupCommonStartCase(image);
    SetBed(image, 0, 0, 300, /*withTAve=*/true);
    SetBedDuration(image, 0, 0);
    SetDicomProperty(image, PropName(0x0054, 0x1300),
                     std::to_string(TAveSeconds(kBedDurationSeconds, kF18HalfLifeSeconds) * 1000.0), 0, 1);
    SetBedDuration(image, 0, 1);

    CPPUNIT_ASSERT_THROW(mitk::DeduceDecayCorrection(image, kF18HalfLifeSeconds),
                         mitk::AmbiguousDecayTimingException);
  }

  void Start_SlotMissingAcquisitionTime_NotClassifiedAsSeriesTime()
  {
    // Slice 0 starts at SeriesTime. Slice 1 starts 300 s later by its frame
    // timing but has no AcquisitionDate/Time, so it must not inherit slice
    // 0's value and be taken for a bed that needs no frame-timing formula.
    auto image = MakeSyntheticImage(/*nSlices=*/2, /*nTimeSteps=*/1);
    SetupCommonStartCase(image);
    SetBed(image, 0, 0, 0, /*withTAve=*/true);
    SetBedDuration(image, 0, 0);
    SetDicomProperty(image, PropName(0x0054, 0x1300),
                     std::to_string((300.0 + TAveSeconds(kBedDurationSeconds, kF18HalfLifeSeconds)) * 1000.0),
                     0, 1);
    SetBedDuration(image, 0, 1);

    CPPUNIT_ASSERT_THROW(mitk::DeduceDecayCorrection(image, kF18HalfLifeSeconds),
                         mitk::AmbiguousDecayTimingException);
  }

  void Start_Dynamic_LaterFramesShareSeriesReference()
  {
    // The same holds across time: a dynamic DC=START series is corrected to
    // one instant too, so a later frame must not be corrected to its own
    // start just because frame 0 starts at SeriesTime.
    auto image = MakeSyntheticImage(/*nSlices=*/1, /*nTimeSteps=*/2);
    SetupCommonStartCase(image);
    for (mitk::TimeStepType t = 0; t < 2; ++t)
    {
      SetBed(image, t, 0, static_cast<int>(t) * 300, /*withTAve=*/true);
      SetBedDuration(image, t, 0);
    }

    const auto info = mitk::DeduceDecayCorrection(image, kF18HalfLifeSeconds);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(kStartExpectedDecaySeconds, info.decayTimes.at(0).at(0), 1e-3);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(kStartExpectedDecaySeconds, info.decayTimes.at(1).at(0), 1e-3);
  }

  void Start_UnparseableAcquisitionTime_Throws_InvalidDICOMPropertyValueException()
  {
    // A malformed value is reported as such rather than as a timing
    // ambiguity, whichever rule the slot would otherwise have taken.
    auto image = MakeThreeBedGeneralRuleImage();
    SetDicomProperty(image, PropName(0x0008, 0x0032), "12xx30", 0, 1);

    CPPUNIT_ASSERT_THROW(mitk::DeduceDecayCorrection(image, kF18HalfLifeSeconds),
                         mitk::InvalidDICOMPropertyValueException);
  }

  void Radiopharm_DoseBelowThreshold_RecordsAdaptation()
  {
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1074), "368.08");

    std::vector<mitk::SUVAdaptation> adaptations;
    const auto infos = mitk::GetRadiopharmaceuticalInfos(
      image, mitk::DICOMReadPolicy::Lenient, adaptations);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), infos.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), adaptations.size());
    CPPUNIT_ASSERT(mitk::SUVAdaptationRule::DoseReinterpretedAsMBq == adaptations[0].rule);
    CPPUNIT_ASSERT_EQUAL(std::string("(0018,1074)"), adaptations[0].dicomTag);
    // The stored value is kept verbatim so an auditor can see what was read,
    // not only what was used.
    CPPUNIT_ASSERT_EQUAL(std::string("368.08"), adaptations[0].originalValue);
  }

  void Radiopharm_DoseAboveThreshold_RecordsNothing()
  {
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1074), "3.6808e8");

    std::vector<mitk::SUVAdaptation> adaptations;
    (void)mitk::GetRadiopharmaceuticalInfos(image, mitk::DICOMReadPolicy::Lenient,
                                            adaptations);
    CPPUNIT_ASSERT(adaptations.empty());
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
    SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1075),
                     "6586.26");                                            // Half-life [s]
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

  // ---- Administration-time resolution, IBSI-SUV v3.0.1 ----
  //
  // (0018,1078) is used verbatim only while the resulting duration lies in
  // [-3600 s, 2 * T_half). Outside it the stored date is treated as
  // untrustworthy and rebuilt from the decay-correction reference datetime,
  // which is permitted only below a half-life of 41400 s. The fixture below
  // puts acquisition and series at 12:00:00 so the reference instant is
  // exactly that, and the administration stamp alone decides the offset.
  //
  // These replace an earlier case asserting that any negative duration from
  // (0018,1078) must throw. v3.0.1 tolerates up to an hour of it, because a
  // dynamic scan legitimately starts before administration.

  void SetupAdminWindowCase(mitk::Image* image, const char* rpStartDateTime)
  {
    SetDicomProperty(image, PropName(0x0054, 0x1102), "START");
    SetDicomProperty(image, PropName(0x0008, 0x0021), "20260430");   // SeriesDate
    SetDicomProperty(image, PropName(0x0008, 0x0031), "120000");     // SeriesTime
    SetDicomProperty(image, PropName(0x0008, 0x0070), "SIEMENS");
    SetDicomProperty(image, PropName(0x0008, 0x0022), "20260430");   // AcqDate
    SetDicomProperty(image, PropName(0x0008, 0x0032), "120000");     // AcqTime == SeriesTime
    SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1078), rpStartDateTime);
  }

  static constexpr double kF18HalfLifeSeconds = 6586.26;

  void AdminWindow_AcquisitionOneHourEarly_Accepted()
  {
    // Exactly at the floor: acquisition 3600 s before administration is the
    // dynamic-scan case the window exists to admit.
    auto image = MakeSyntheticImage(1, 1);
    SetupAdminWindowCase(image, "20260430130000");

    const auto info = mitk::DeduceDecayCorrection(image, kF18HalfLifeSeconds);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-3600.0, info.decayTimes.at(0).at(0), 1e-6);
    CPPUNIT_ASSERT(info.adaptations.empty());
  }

  void AdminWindow_JustBelowFloor_SubstitutesDate()
  {
    // One second past the floor. The stored date is discarded and rebuilt
    // from the reference, which then needs the -24 h correction.
    auto image = MakeSyntheticImage(1, 1);
    SetupAdminWindowCase(image, "20260430130001");

    const auto info = mitk::DeduceDecayCorrection(image, kF18HalfLifeSeconds);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(86400.0 - 3601.0, info.decayTimes.at(0).at(0), 1e-6);
    CPPUNIT_ASSERT(HasRule(info, mitk::SUVAdaptationRule::AdministrationDateFromReferenceWithStartDateTime));
    CPPUNIT_ASSERT(HasRule(info, mitk::SUVAdaptationRule::AdministrationTimeShiftedBackOneDay));
  }

  void AdminWindow_JustBelowTwoHalfLives_Accepted()
  {
    // 2 * T_half is 13172.52 s, so 13172 s is inside the window and the
    // stamp is used as stored.
    auto image = MakeSyntheticImage(1, 1);
    SetupAdminWindowCase(image, "20260430082028");   // 12:00:00 - 13172 s

    const auto info = mitk::DeduceDecayCorrection(image, kF18HalfLifeSeconds);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(13172.0, info.decayTimes.at(0).at(0), 1e-6);
    CPPUNIT_ASSERT(info.adaptations.empty());
  }

  void AdminWindow_AtTwoHalfLives_SubstitutesDate()
  {
    // One second further out. Same calendar day, so the reconstruction
    // happens to yield the same number -- which is exactly why the value
    // cannot be the assertion here. The adaptation record is what
    // distinguishes "used as stored" from "rebuilt".
    auto image = MakeSyntheticImage(1, 1);
    SetupAdminWindowCase(image, "20260430082027");   // 12:00:00 - 13173 s

    const auto info = mitk::DeduceDecayCorrection(image, kF18HalfLifeSeconds);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(13173.0, info.decayTimes.at(0).at(0), 1e-6);
    CPPUNIT_ASSERT(HasRule(info, mitk::SUVAdaptationRule::AdministrationDateFromReferenceWithStartDateTime));
    CPPUNIT_ASSERT(!HasRule(info, mitk::SUVAdaptationRule::AdministrationTimeShiftedBackOneDay));
  }

  void AdminWindow_HalfLifeJustBelowLimit_SubstitutesDate()
  {
    // Offset 200000 s, far outside 2 * T_half for both this case and the
    // next, so the half-life gate is the only thing that differs.
    auto image = MakeSyntheticImage(1, 1);
    SetupAdminWindowCase(image, "20260428042640");   // 12:00:00 - 200000 s

    const auto info = mitk::DeduceDecayCorrection(image, 41399.0);
    CPPUNIT_ASSERT(HasRule(info, mitk::SUVAdaptationRule::AdministrationDateFromReferenceWithStartDateTime));
  }

  void AdminWindow_HalfLifeAtLimit_Refuses()
  {
    // At and above 41400 s an administration date that is wrong by whole
    // days still yields a plausible SUV, so the substitution is forbidden
    // and the input must be refused. This gate is the only thing keeping
    // long-lived nuclides with untrustworthy dates from computing silently.
    auto image = MakeSyntheticImage(1, 1);
    SetupAdminWindowCase(image, "20260428042640");

    CPPUNIT_ASSERT_THROW(mitk::DeduceDecayCorrection(image, 41400.0),
                         mitk::UnrecoverableAdministrationDateException);
  }

  void AdminWindow_HalfLifeUnavailable_Refuses()
  {
    // No (0018,1075) and no override: the acceptance window is undefined,
    // so there is no rule to apply.
    auto image = MakeSyntheticImage(1, 1);
    SetupAdminWindowCase(image, "20260430110000");

    CPPUNIT_ASSERT_THROW(mitk::DeduceDecayCorrection(image),
                         mitk::MissingDICOMPropertyException);
  }

  void AdminWindow_StrictPolicy_RefusesDateSubstitution()
  {
    auto image = MakeSyntheticImage(1, 1);
    SetupAdminWindowCase(image, "20260430130001");

    CPPUNIT_ASSERT_THROW(
      mitk::DeduceDecayCorrection(image, kF18HalfLifeSeconds,
                                  mitk::DICOMReadPolicy::Strict),
      mitk::AdministrationDateSubstitutionRefusedException);
  }

  void AdminWindow_StrictPolicy_RefusalIsCatchableAsBaseException()
  {
    auto image = MakeSyntheticImage(1, 1);
    SetupAdminWindowCase(image, "20260430130001");

    CPPUNIT_ASSERT_THROW(
      mitk::DeduceDecayCorrection(image, kF18HalfLifeSeconds,
                                  mitk::DICOMReadPolicy::Strict),
      mitk::BenchmarkAdaptationRequiredException);
  }

  void AdminTime_UtcOffsetHonoured()
  {
    // Acquisition and series carry no UTC offset, so they are read as
    // local-to-themselves; the administration stamp carries +0100 and must
    // be shifted to match. Both stamps then name 12:00 UTC and the decay
    // duration is zero. Ignoring the offset would give -3600 s, which is
    // inside the acceptance window and would therefore pass silently.
    //
    // No benchmark DRO mixes offset-bearing and offset-free stamps -- every
    // DRO is internally consistent -- so this case is the only guard on the
    // convention.
    auto image = MakeSyntheticImage(1, 1);
    SetupAdminWindowCase(image, "20260430130000+0100");

    const auto info = mitk::DeduceDecayCorrection(image, kF18HalfLifeSeconds);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, info.decayTimes.at(0).at(0), 1e-6);
  }

  void AdminTime_UnparseableStartDateTime_Throws()
  {
    // A present but malformed (0018,1078) is an error, not an absence: a
    // silent fall-through to (0018,1072) would compute from a different
    // instant than the one the input names.
    auto image = MakeSyntheticImage(1, 1);
    SetupAdminWindowCase(image, "not-a-datetime");
    SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1072), "110000");

    CPPUNIT_ASSERT_THROW(mitk::DeduceDecayCorrection(image, kF18HalfLifeSeconds),
                         mitk::InvalidDICOMPropertyValueException);
  }

  void Start_Step1_NegativeOffset_DoesNotFallThroughToStep2()
  {
    // The vendor private datetime carries no plausibility precondition, so
    // a small negative offset must stay on Step 1 rather than dropping to
    // Step 2. Step 2 would answer +930 s here, Step 1 answers -1800 s, so
    // the value says which rule ran. No benchmark DRO exercises this.
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, PropName(0x0054, 0x1102), "START");
    SetDicomProperty(image, PropName(0x0008, 0x0021), "20260430");
    SetDicomProperty(image, PropName(0x0008, 0x0031), "121530");   // SeriesTime
    SetDicomProperty(image, PropName(0x0008, 0x0070), "SIEMENS");
    SetDicomProperty(image, PropName(0x0008, 0x0022), "20260430");
    SetDicomProperty(image, PropName(0x0008, 0x0032), "121530");   // AcqTime == SeriesTime
    SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1078),
                     "20260430120000");                            // administration 12:00:00
    SetDicomProperty(image, "mitk.pet.SiemensDecayDateTime",
                     "20260430113000");                            // reference 11:30:00

    const auto info = mitk::DeduceDecayCorrection(image, kF18HalfLifeSeconds);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-1800.0, info.decayTimes.at(0).at(0), 1e-6);
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

  // ---- Enhanced PET: the reference instant per slice ----
  //
  // (0018,9758) Decay Corrected replaces (0054,1102). YES means the pixels
  // are corrected to the single (0018,9701); NO means each frame's own
  // measurement instant is the reference, and the reader publishes those
  // with one value per slice.

  // An Enhanced object with the given number of frames and the given
  // (0018,9758), administered at 10:00:00 on the frames' date.
  mitk::Image::Pointer MakeEnhancedDecayImage(unsigned int nSlices, const std::string& decayCorrected)
  {
    auto image = MakeSyntheticImage(nSlices, 1);
    SetDicomProperty(image, PropName(0x0008, 0x0016), "1.2.840.10008.5.1.4.1.1.130");
    SetDicomProperty(image, PropName(0x0028, 0x0008), std::to_string(nSlices));
    SetDicomProperty(image, PropName(0x0018, 0x9758), decayCorrected);
    SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1078), "20260430100000");
    return image;
  }

  void Enhanced_DecayCorrectedNO_UsesFrameReferencePerSlice()
  {
    auto image = MakeEnhancedDecayImage(4, "NO");
    const std::string frameReference = SeqPropName(0x0020, 0x9111, 0x0018, 0x9151);
    SetDicomProperty(image, frameReference, "20260430111500", 0, 0);
    SetDicomProperty(image, frameReference, "20260430111500", 0, 1);
    SetDicomProperty(image, frameReference, "20260430111000", 0, 2);
    SetDicomProperty(image, frameReference, "20260430111000", 0, 3);

    const auto info = mitk::DeduceDecayCorrection(image, /*halfLife=*/6586.2);
    CPPUNIT_ASSERT_EQUAL(mitk::DecayCorrectionStrategy::Start, info.strategy);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4500.0, info.decayTimes.at(0).at(0), 1e-3);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4500.0, info.decayTimes.at(0).at(1), 1e-3);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4200.0, info.decayTimes.at(0).at(2), 1e-3);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4200.0, info.decayTimes.at(0).at(3), 1e-3);
  }

  void Enhanced_DecayCorrectedYES_IgnoresFrameTimes()
  {
    // DRO_7_3_0 against DRO_7_3_1: identical per-frame frame times, and only
    // (0018,9758) decides whether they enter the result.
    auto image = MakeEnhancedDecayImage(4, "YES");
    SetDicomProperty(image, PropName(0x0018, 0x9701), "20260430110000");
    const std::string frameReference = SeqPropName(0x0020, 0x9111, 0x0018, 0x9151);
    SetDicomProperty(image, frameReference, "20260430111500", 0, 0);
    SetDicomProperty(image, frameReference, "20260430111500", 0, 1);
    SetDicomProperty(image, frameReference, "20260430111000", 0, 2);
    SetDicomProperty(image, frameReference, "20260430111000", 0, 3);

    const auto info = mitk::DeduceDecayCorrection(image, /*halfLife=*/6586.2);
    CPPUNIT_ASSERT_EQUAL(mitk::DecayCorrectionStrategy::Start, info.strategy);
    for (unsigned int s = 0; s < 4; ++s)
    {
      CPPUNIT_ASSERT_DOUBLES_EQUAL(3600.0, info.decayTimes.at(0).at(s), 1e-3);
    }
  }

  void Enhanced_DecayCorrectedNO_FallsBackToAcquisitionPlusTAvePerSlice()
  {
    // Frames without a Frame Reference DateTime use their acquisition instant
    // plus T_ave, beside frames that have one: the precedence is per frame.
    // A 1 ms frame duration keeps T_ave far below the assertion tolerance.
    auto image = MakeEnhancedDecayImage(3, "NO");
    SetDicomProperty(image, SeqPropName(0x0020, 0x9111, 0x0018, 0x9151), "20260430111500", 0, 0);
    const std::string acquisition = SeqPropName(0x0020, 0x9111, 0x0018, 0x9074);
    const std::string duration = SeqPropName(0x0020, 0x9111, 0x0018, 0x9220);
    SetDicomProperty(image, acquisition, "20260430111000", 0, 1);
    SetDicomProperty(image, duration, "1", 0, 1);
    SetDicomProperty(image, acquisition, "20260430110500", 0, 2);
    SetDicomProperty(image, duration, "1", 0, 2);

    const auto info = mitk::DeduceDecayCorrection(image, /*halfLife=*/6586.2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4500.0, info.decayTimes.at(0).at(0), 1e-3);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4200.0, info.decayTimes.at(0).at(1), 1e-3);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3900.0, info.decayTimes.at(0).at(2), 1e-3);
  }

  void Enhanced_DecayCorrectedNO_DatetimeMissingOnOneSlice_Throws()
  {
    // The reader publishes functional-group values all or nothing per file,
    // so a frame without any measurement instant is the input's doing.
    auto image = MakeEnhancedDecayImage(4, "NO");
    const std::string frameReference = SeqPropName(0x0020, 0x9111, 0x0018, 0x9151);
    for (unsigned int s = 0; s < 3; ++s)
    {
      SetDicomProperty(image, frameReference, "20260430111500", 0, s);
    }

    CPPUNIT_ASSERT_THROW(mitk::DeduceDecayCorrection(image, /*halfLife=*/6586.2),
                         mitk::MissingDICOMPropertyException);
  }

  void Enhanced_FramesUnresolvedByReader_Refuses()
  {
    // A multi-frame object none of whose functional-group values reached
    // MITK: the reader could not map the functional groups to frames.
    auto image = MakeEnhancedDecayImage(4, "NO");

    CPPUNIT_ASSERT_THROW(mitk::DeduceDecayCorrection(image, /*halfLife=*/6586.2),
                         mitk::EnhancedPETFramesUnresolvedException);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkSUVCalculationHelper)
