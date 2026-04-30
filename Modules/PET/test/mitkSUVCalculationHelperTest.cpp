/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

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
  // Build a 1×1×nSlices×nTimeSteps mitk::Image with float pixels. The data
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

  // Single-tag helpers
  MITK_TEST(RadionuclideHalfLife_Found);
  MITK_TEST(RadionuclideHalfLife_Empty);
  MITK_TEST(RadionuclideTotalDose_Found);
  MITK_TEST(RadionuclideTotalDose_Empty);
  MITK_TEST(PatientWeight_Found);
  MITK_TEST(PatientWeight_Missing_Throws_MissingDICOMPropertyException);

  // DeduceDecayCorrection
  MITK_TEST(Admin_AllZeros_SingleTimestep);
  MITK_TEST(Admin_FromImageGeometry_NoAcquisitionTags);
  MITK_TEST(Start_SingleTimestep_UsesSeriesTimeMinusInjection);
  MITK_TEST(Start_MultiTimestep_SameValueAcrossTimesteps);
  MITK_TEST(None_PerSliceDecayTime);
  MITK_TEST(Prefers_1078_Over_1072);
  MITK_TEST(Rollover_1072_Recovered);
  MITK_TEST(Rollover_1078_NoSilentCorrection_Throws_AmbiguousDecayTimingException);
  MITK_TEST(Strategy_Start_MissingSeriesTime_Throws_MissingDICOMPropertyException);
  MITK_TEST(Strategy_None_MissingAcqTime_Throws_MissingDICOMPropertyException);

  CPPUNIT_TEST_SUITE_END();

private:

  // Most decay-correction tests share a common acquisition-day setup. Helper
  // configures the image with values that yield well-known decay durations.
  // Series Time 12:15:30, injection (1078) on the same day at 11:00:00 →
  // SeriesTime − InjectionTime = (12*3600+15*60+30) − (11*3600) = 4530 s.
  static constexpr double kStartExpectedDecaySeconds = 4530.0;

  void SetupCommonStartCase(mitk::Image* image)
  {
    SetDicomProperty(image, PropName(0x0054, 0x1102), "START");
    SetDicomProperty(image, PropName(0x0008, 0x0021), "20260430");          // SeriesDate
    SetDicomProperty(image, PropName(0x0008, 0x0031), "121530");            // SeriesTime
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

  // ---- Single-tag helpers ----

  void RadionuclideHalfLife_Found()
  {
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1075), "6586.26");

    const auto values = mitk::GetRadionuclideHalfLife(image);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), values.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6586.26, values[0], 1e-9);
  }

  void RadionuclideHalfLife_Empty()
  {
    auto image = MakeSyntheticImage(1, 1);
    CPPUNIT_ASSERT(mitk::GetRadionuclideHalfLife(image).empty());
  }

  void RadionuclideTotalDose_Found()
  {
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1074), "1.85e8");

    const auto values = mitk::GetRadionuclideTotalDose(image);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), values.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.85e8, values[0], 1.0);
  }

  void RadionuclideTotalDose_Empty()
  {
    auto image = MakeSyntheticImage(1, 1);
    CPPUNIT_ASSERT(mitk::GetRadionuclideTotalDose(image).empty());
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

  void Start_SingleTimestep_UsesSeriesTimeMinusInjection()
  {
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

  void Start_MultiTimestep_SameValueAcrossTimesteps()
  {
    auto image = MakeSyntheticImage(/*nSlices=*/2, /*nTimeSteps=*/3);
    SetupCommonStartCase(image);

    const auto info = mitk::DeduceDecayCorrection(image);
    CPPUNIT_ASSERT_EQUAL(mitk::DecayCorrectionStrategy::Start, info.strategy);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), info.decayTimes.size());
    // MOD-H-2: every (timestep, slice) entry holds the SAME value.
    for (mitk::TimeStepType t = 0; t < 3; ++t)
    {
      CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), info.decayTimes.at(t).size());
      for (const auto& slice : info.decayTimes.at(t))
      {
        CPPUNIT_ASSERT_DOUBLES_EQUAL(kStartExpectedDecaySeconds, slice.second, 1e-3);
      }
    }
  }

  // ---- DeduceDecayCorrection: NONE ----

  void None_PerSliceDecayTime()
  {
    auto image = MakeSyntheticImage(/*nSlices=*/2, /*nTimeSteps=*/1);
    SetDicomProperty(image, PropName(0x0054, 0x1102), "NONE");

    // Each slice has its own acquisition timestamp.
    SetDicomProperty(image, PropName(0x0008, 0x0022), "20260430", 0, 0);
    SetDicomProperty(image, PropName(0x0008, 0x0032), "121000", 0, 0);  // 12:10:00
    SetDicomProperty(image, PropName(0x0008, 0x0022), "20260430", 0, 1);
    SetDicomProperty(image, PropName(0x0008, 0x0032), "122000", 0, 1);  // 12:20:00

    SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1078),
                     "20260430110000");  // injection at 11:00:00

    const auto info = mitk::DeduceDecayCorrection(image);
    CPPUNIT_ASSERT_EQUAL(mitk::DecayCorrectionStrategy::None, info.strategy);
    // Slice 0: 12:10:00 - 11:00:00 = 4200 s. Slice 1: 12:20:00 - 11:00:00 = 4800 s.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4200.0, info.decayTimes.at(0).at(0), 1e-3);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4800.0, info.decayTimes.at(0).at(1), 1e-3);
    CPPUNIT_ASSERT(info.decayTimes.at(0).at(0) != info.decayTimes.at(0).at(1));
  }

  // ---- DeduceDecayCorrection: injection-time resolution ----

  void Prefers_1078_Over_1072()
  {
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, PropName(0x0054, 0x1102), "START");
    SetDicomProperty(image, PropName(0x0008, 0x0021), "20260430");
    SetDicomProperty(image, PropName(0x0008, 0x0031), "121530");
    // (0018,1078) — should be used.
    SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1078),
                     "20260430110000");
    // (0018,1072) — present but with deliberately *different* value; must be
    // ignored because (0018,1078) takes precedence.
    SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1072), "100000");

    const auto info = mitk::DeduceDecayCorrection(image);
    // If 1078 wins, decay = 4530 s. If 1072 was used (with date 20260430),
    // decay would be (12:15:30 − 10:00:00) = 8130 s.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(kStartExpectedDecaySeconds,
                                  info.decayTimes.at(0).at(0), 1e-3);
  }

  void Rollover_1072_Recovered()
  {
    // Acquisition today at 01:00:00, injection (TM-only) at 23:00:00.
    // Naive subtraction yields -22 h; rollover guard subtracts 24 h from the
    // injection day → +2 h decay duration.
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, PropName(0x0054, 0x1102), "NONE");
    SetDicomProperty(image, PropName(0x0008, 0x0022), "20260430");
    SetDicomProperty(image, PropName(0x0008, 0x0032), "010000");
    SetDicomProperty(image, SeqPropName(0x0054, 0x0016, 0x0018, 0x1072), "230000");

    const auto info = mitk::DeduceDecayCorrection(image);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0 * 3600.0, info.decayTimes.at(0).at(0), 1e-3);
  }

  void Rollover_1078_NoSilentCorrection_Throws_AmbiguousDecayTimingException()
  {
    // (0018,1078) is unambiguous — a negative delta must surface, not be
    // silently corrected by ±24 h.
    auto image = MakeSyntheticImage(1, 1);
    SetDicomProperty(image, PropName(0x0054, 0x1102), "START");
    SetDicomProperty(image, PropName(0x0008, 0x0021), "20260430");
    SetDicomProperty(image, PropName(0x0008, 0x0031), "121530");
    // Injection AFTER series time → negative decay.
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
