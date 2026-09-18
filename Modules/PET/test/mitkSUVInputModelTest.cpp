/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <string>

#include <mitkDICOMProperty.h>
#include <mitkDICOMTagPath.h>
#include <mitkImage.h>
#include <mitkPixelType.h>
#include <mitkProperties.h>
#include <mitkStringProperty.h>
#include <mitkTemporoSpatialStringProperty.h>

#include <mitkSUVInputModel.h>

#include <string>
#include <vector>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

namespace
{
  // Minimal mitk::Image acting as an IPropertyProvider for the
  // classifier. The pixel data is irrelevant; only the property list
  // matters.
  mitk::Image::Pointer MakeImage()
  {
    mitk::Image::Pointer image = mitk::Image::New();
    const auto pixelType = mitk::MakeScalarPixelType<float>();
    const unsigned int dims[3] = { 1u, 1u, 1u };
    image->Initialize(pixelType, 3, dims);
    return image;
  }

  // Set a DICOM tag value on the image (top-level tag, no sequence).
  void SetDicomTag(mitk::Image* image, unsigned int group, unsigned int element,
                   const std::string& value)
  {
    const std::string key = mitk::DICOMTagPathToPropertyName(
      mitk::DICOMTagPath(group, element));
    auto prop = mitk::DICOMProperty::New();
    prop->SetValue(0, 0, value);
    image->SetProperty(key.c_str(), prop);
  }

  // Property name for a functional-group path with concrete item indices,
  // matching what the DICOM reader produces for an Enhanced PET object:
  // DICOM.5200.9230.[frame].GGGG.EEEE.[item].GGGG.EEEE
  std::string PerFrameName(unsigned int frame,
                           unsigned int innerGroup, unsigned int innerElement, unsigned int item,
                           unsigned int leafGroup, unsigned int leafElement)
  {
    mitk::DICOMTagPath path;
    path.AddSelection(0x5200, 0x9230, static_cast<mitk::DICOMTagPath::ItemSelectionIndex>(frame));
    path.AddSelection(innerGroup, innerElement,
                      static_cast<mitk::DICOMTagPath::ItemSelectionIndex>(item));
    path.AddElement(leafGroup, leafElement);
    return mitk::DICOMTagPathToPropertyName(path);
  }

  void SetProperty(mitk::Image* image, const std::string& name, const std::string& value)
  {
    auto prop = mitk::DICOMProperty::New();
    prop->SetValue(0, 0, value);
    image->SetProperty(name.c_str(), prop);
  }

  // A minimal Enhanced PET object: the SOP Class UID that selects the code
  // path, a frame count, and per-frame unit codes and rescale values.
  mitk::Image::Pointer MakeEnhancedImage(unsigned int frames, const std::string& unitCode,
                                         const std::vector<std::string>& perFrameSlopes)
  {
    auto image = MakeImage();
    SetDicomTag(image, 0x0008, 0x0016, "1.2.840.10008.5.1.4.1.1.130");
    SetDicomTag(image, 0x0028, 0x0008, std::to_string(frames));
    for (unsigned int f = 0; f < frames; ++f)
    {
      if (!unitCode.empty())
      {
        SetProperty(image, PerFrameName(f, 0x0040, 0x9096, 0, 0x0040, 0x08EA), "");
        SetProperty(image,
                    mitk::DICOMTagPathToPropertyName(
                      mitk::DICOMTagPath()
                        .AddSelection(0x5200, 0x9230, static_cast<mitk::DICOMTagPath::ItemSelectionIndex>(f))
                        .AddSelection(0x0040, 0x9096, 0)
                        .AddSelection(0x0040, 0x08EA, 0)
                        .AddElement(0x0008, 0x0100)),
                    unitCode);
      }
      if (f < perFrameSlopes.size())
      {
        SetProperty(image, PerFrameName(f, 0x0028, 0x9145, 0, 0x0028, 0x1053), perFrameSlopes[f]);
      }
    }
    return image;
  }

  // Set a lifted private-tag property (the names BaseDICOMReaderService
  // attaches for the Philips CNTS scale factors).
  void SetNamedString(mitk::Image* image, const std::string& name,
                      const std::string& value)
  {
    auto prop = mitk::TemporoSpatialStringProperty::New(value);
    image->SetProperty(name.c_str(), prop);
  }
}

class mitkSUVInputModelTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkSUVInputModelTestSuite);

  // Standard units
  MITK_TEST(BQML_ClassifiesAsActivityConcentration);
  MITK_TEST(GML_ClassifiesAsPrenormalizedBW);
  MITK_TEST(CM2ML_ClassifiesAsPrenormalizedBSA);

  // (0054,1006) SUV Type refines the source variant for GML
  MITK_TEST(GML_SUVType_BW_ClassifiesAsPrenormalizedBW);
  MITK_TEST(GML_SUVType_LBMJANMA_ClassifiesAsPrenormalizedLBMJanma);
  MITK_TEST(GML_SUVType_LBMJAMES128_ClassifiesAsPrenormalizedLBMJames128);
  MITK_TEST(GML_SUVType_IBW_ClassifiesAsPrenormalizedIBW);
  MITK_TEST(GML_SUVType_BSA_Throws_Inconsistent);
  MITK_TEST(GML_SUVType_Unknown_Throws);
  MITK_TEST(CM2ML_SUVType_BSA_OK);
  MITK_TEST(CM2ML_SUVType_NotBSA_Throws);

  // Whitespace / case insensitivity at the value boundary
  MITK_TEST(LowercaseAndPaddedUnits_StillRecognised);

  // Philips CNTS variants
  MITK_TEST(CNTS_PhilipsSUVScale_ClassifiesAsPrenormalizedBW);
  MITK_TEST(CNTS_PhilipsActivityScale_ClassifiesAsActivityConcentration);
  MITK_TEST(CNTS_PhilipsBothFactorsPresent_PrefersActivityScale);
  MITK_TEST(CNTS_PhilipsActivityScaleZero_FallsBackToSUVScale);
  MITK_TEST(CNTS_SUVScaleWithNonBWSUVType_Throws_UnsupportedPETUnitsException);
  MITK_TEST(CNTS_SUVScaleWithExplicitBWSUVType_Accepted);
  MITK_TEST(CNTS_ActivityScaleWithNonBWSUVType_Accepted);

  // Error matrix
  MITK_TEST(NoUnits_Throws_MissingDICOMPropertyException);
  MITK_TEST(UnknownUnits_Throws_UnsupportedPETUnitsException);
  MITK_TEST(CNTS_NonPhilipsManufacturer_Throws_UnsupportedPETUnitsException);
  MITK_TEST(CNTS_PhilipsNoFactor_Throws_MissingPhilipsPETScaleException);

  // Defensive contract
  MITK_TEST(GML_SUVTypeLBMJANMA_ClassifiesAsJanmahasatian);
  MITK_TEST(GML_SUVTypeLBMJAMES128_ClassifiesAsJames128);
  MITK_TEST(GML_SUVTypeLBM_ClassifiesAsMorgan);
  // Enhanced PET
  MITK_TEST(EnhancedPET_BqMl_ClassifiesAsActivityConcentration);
  MITK_TEST(EnhancedPET_GmlSUVbw_ClassifiesAsPrenormalizedBW);
  MITK_TEST(EnhancedPET_UniformPerFrameRescale_Accepted);
  MITK_TEST(EnhancedPET_RescaleWrittenTwoWays_Accepted);
  MITK_TEST(EnhancedPET_VaryingPerFrameRescale_Refuses);
  MITK_TEST(EnhancedPET_FewerRescaleValuesThanFrames_Refuses);
  MITK_TEST(EnhancedPET_UnitDiffersBetweenFrames_Refuses);
  MITK_TEST(EnhancedPET_NoUsableUnit_Throws);
  MITK_TEST(EnhancedPET_RescaleTypeFallback_Accepted);
  MITK_TEST(IsEnhancedPETInput_OnlyForTheEnhancedSOPClass);

  MITK_TEST(NullProvider_Throws);

  CPPUNIT_TEST_SUITE_END();

public:

  void BQML_ClassifiesAsActivityConcentration()
  {
    auto img = MakeImage();
    SetDicomTag(img, 0x0054, 0x1001, "BQML");

    const auto m = mitk::ClassifyPETInput(img.GetPointer(),
                                          mitk::DICOMReadPolicy::Lenient);
    CPPUNIT_ASSERT(mitk::SUVPixelSemantics::ActivityConcentration == m.semantics);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, m.activityScale, 1e-12);
  }

  void GML_ClassifiesAsPrenormalizedBW()
  {
    auto img = MakeImage();
    SetDicomTag(img, 0x0054, 0x1001, "GML");

    const auto m = mitk::ClassifyPETInput(img.GetPointer(),
                                          mitk::DICOMReadPolicy::Lenient);
    CPPUNIT_ASSERT(mitk::SUVPixelSemantics::PrenormalizedSUV == m.semantics);
    CPPUNIT_ASSERT(mitk::SUVVariant::BW == m.sourceVariant);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, m.prenormScale, 1e-12);
  }

  void CM2ML_ClassifiesAsPrenormalizedBSA()
  {
    auto img = MakeImage();
    SetDicomTag(img, 0x0054, 0x1001, "CM2ML");

    const auto m = mitk::ClassifyPETInput(img.GetPointer(),
                                          mitk::DICOMReadPolicy::Lenient);
    CPPUNIT_ASSERT(mitk::SUVPixelSemantics::PrenormalizedSUV == m.semantics);
    CPPUNIT_ASSERT(mitk::SUVVariant::BSA == m.sourceVariant);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, m.prenormScale, 1e-12);
  }

  void GML_SUVType_BW_ClassifiesAsPrenormalizedBW()
  {
    auto img = MakeImage();
    SetDicomTag(img, 0x0054, 0x1001, "GML");
    SetDicomTag(img, 0x0054, 0x1006, "BW");

    const auto m = mitk::ClassifyPETInput(img.GetPointer(),
                                          mitk::DICOMReadPolicy::Lenient);
    CPPUNIT_ASSERT(mitk::SUVPixelSemantics::PrenormalizedSUV == m.semantics);
    CPPUNIT_ASSERT(mitk::SUVVariant::BW == m.sourceVariant);
  }

  void GML_SUVType_LBMJANMA_ClassifiesAsPrenormalizedLBMJanma()
  {
    auto img = MakeImage();
    SetDicomTag(img, 0x0054, 0x1001, "GML");
    SetDicomTag(img, 0x0054, 0x1006, "LBMJANMA");

    const auto m = mitk::ClassifyPETInput(img.GetPointer(),
                                          mitk::DICOMReadPolicy::Lenient);
    CPPUNIT_ASSERT(mitk::SUVPixelSemantics::PrenormalizedSUV == m.semantics);
    CPPUNIT_ASSERT(mitk::SUVVariant::LBM_Janmahasatian == m.sourceVariant);
  }

  void GML_SUVType_LBMJAMES128_ClassifiesAsPrenormalizedLBMJames128()
  {
    auto img = MakeImage();
    SetDicomTag(img, 0x0054, 0x1001, "GML");
    SetDicomTag(img, 0x0054, 0x1006, "LBMJAMES128");

    const auto m = mitk::ClassifyPETInput(img.GetPointer(),
                                          mitk::DICOMReadPolicy::Lenient);
    CPPUNIT_ASSERT(mitk::SUVPixelSemantics::PrenormalizedSUV == m.semantics);
    CPPUNIT_ASSERT(mitk::SUVVariant::LBM_James128 == m.sourceVariant);
  }

  void GML_SUVType_IBW_ClassifiesAsPrenormalizedIBW()
  {
    auto img = MakeImage();
    SetDicomTag(img, 0x0054, 0x1001, "GML");
    SetDicomTag(img, 0x0054, 0x1006, "IBW");

    const auto m = mitk::ClassifyPETInput(img.GetPointer(),
                                          mitk::DICOMReadPolicy::Lenient);
    CPPUNIT_ASSERT(mitk::SUVPixelSemantics::PrenormalizedSUV == m.semantics);
    CPPUNIT_ASSERT(mitk::SUVVariant::IBW == m.sourceVariant);
  }

  void GML_SUVType_BSA_Throws_Inconsistent()
  {
    auto img = MakeImage();
    SetDicomTag(img, 0x0054, 0x1001, "GML");
    SetDicomTag(img, 0x0054, 0x1006, "BSA");

    CPPUNIT_ASSERT_THROW(mitk::ClassifyPETInput(img.GetPointer(),
                                                mitk::DICOMReadPolicy::Lenient),
                         mitk::InvalidDICOMPropertyValueException);
  }

  void GML_SUVType_Unknown_Throws()
  {
    auto img = MakeImage();
    SetDicomTag(img, 0x0054, 0x1001, "GML");
    SetDicomTag(img, 0x0054, 0x1006, "FOO");

    CPPUNIT_ASSERT_THROW(mitk::ClassifyPETInput(img.GetPointer(),
                                                mitk::DICOMReadPolicy::Lenient),
                         mitk::UnsupportedPETUnitsException);
  }

  void CM2ML_SUVType_BSA_OK()
  {
    auto img = MakeImage();
    SetDicomTag(img, 0x0054, 0x1001, "CM2ML");
    SetDicomTag(img, 0x0054, 0x1006, "BSA");

    const auto m = mitk::ClassifyPETInput(img.GetPointer(),
                                          mitk::DICOMReadPolicy::Lenient);
    CPPUNIT_ASSERT(mitk::SUVPixelSemantics::PrenormalizedSUV == m.semantics);
    CPPUNIT_ASSERT(mitk::SUVVariant::BSA == m.sourceVariant);
  }

  void CM2ML_SUVType_NotBSA_Throws()
  {
    // CM2ML is strictly coupled to SUV Type = BSA per the IBSI-SUV
    // spec. Any other SUVType value is an inconsistency.
    auto img = MakeImage();
    SetDicomTag(img, 0x0054, 0x1001, "CM2ML");
    SetDicomTag(img, 0x0054, 0x1006, "BW");

    CPPUNIT_ASSERT_THROW(mitk::ClassifyPETInput(img.GetPointer(),
                                                mitk::DICOMReadPolicy::Lenient),
                         mitk::InvalidDICOMPropertyValueException);
  }

  void LowercaseAndPaddedUnits_StillRecognised()
  {
    // Defensive trim+upper applied internally so real-world variants
    // do not bypass the classifier.
    auto img = MakeImage();
    SetDicomTag(img, 0x0054, 0x1001, "  bqml ");

    const auto m = mitk::ClassifyPETInput(img.GetPointer(),
                                          mitk::DICOMReadPolicy::Lenient);
    CPPUNIT_ASSERT(mitk::SUVPixelSemantics::ActivityConcentration == m.semantics);
  }

  void CNTS_PhilipsSUVScale_ClassifiesAsPrenormalizedBW()
  {
    auto img = MakeImage();
    SetDicomTag(img, 0x0054, 0x1001, "CNTS");
    SetDicomTag(img, 0x0008, 0x0070, "Philips Medical Systems");
    SetNamedString(img.GetPointer(), "mitk.pet.PhilipsSUVScale", "0.0005");

    const auto m = mitk::ClassifyPETInput(img.GetPointer(),
                                          mitk::DICOMReadPolicy::Lenient);
    CPPUNIT_ASSERT(mitk::SUVPixelSemantics::PrenormalizedSUV == m.semantics);
    CPPUNIT_ASSERT(mitk::SUVVariant::BW == m.sourceVariant);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0005, m.prenormScale, 1e-12);
  }

  void CNTS_PhilipsActivityScale_ClassifiesAsActivityConcentration()
  {
    auto img = MakeImage();
    SetDicomTag(img, 0x0054, 0x1001, "CNTS");
    SetDicomTag(img, 0x0008, 0x0070, "Philips Medical Systems");
    SetNamedString(img.GetPointer(), "mitk.pet.PhilipsActivityScale", "0.5");

    const auto m = mitk::ClassifyPETInput(img.GetPointer(),
                                          mitk::DICOMReadPolicy::Lenient);
    CPPUNIT_ASSERT(mitk::SUVPixelSemantics::ActivityConcentration == m.semantics);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, m.activityScale, 1e-12);
  }

  void CNTS_PhilipsBothFactorsPresent_PrefersActivityScale()
  {
    // No benchmark DRO carries both factors -- DRO_2_4 has only the
    // SUV-scale, DRO_2_5 and DRO_3_4_2 only the activity-scale -- so the
    // precedence is invisible to the benchmark and this case is the only
    // thing pinning it.
    //
    // The activity-scale wins because its result stays correctable: it
    // yields Bq/mL and the normalization then uses this run's patient data,
    // overrides included. The SUV-scale yields SUVbw directly, baking in
    // whatever weight the scanner held at acquisition time.
    auto img = MakeImage();
    SetDicomTag(img, 0x0054, 0x1001, "CNTS");
    SetDicomTag(img, 0x0008, 0x0070, "Philips Medical Systems");
    SetNamedString(img.GetPointer(), "mitk.pet.PhilipsSUVScale",      "0.0005");
    SetNamedString(img.GetPointer(), "mitk.pet.PhilipsActivityScale", "0.5");

    const auto m = mitk::ClassifyPETInput(img.GetPointer(),
                                          mitk::DICOMReadPolicy::Lenient);
    CPPUNIT_ASSERT(mitk::SUVPixelSemantics::ActivityConcentration == m.semantics);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, m.activityScale, 1e-12);
  }

  void CNTS_PhilipsActivityScaleZero_FallsBackToSUVScale()
  {
    // "Absent, empty or zero" are one condition for the precedence: a
    // factor that cannot scale anything is not a factor.
    auto img = MakeImage();
    SetDicomTag(img, 0x0054, 0x1001, "CNTS");
    SetDicomTag(img, 0x0008, 0x0070, "Philips Medical Systems");
    SetNamedString(img.GetPointer(), "mitk.pet.PhilipsSUVScale",      "0.0005");
    SetNamedString(img.GetPointer(), "mitk.pet.PhilipsActivityScale", "0");

    const auto m = mitk::ClassifyPETInput(img.GetPointer(),
                                          mitk::DICOMReadPolicy::Lenient);
    CPPUNIT_ASSERT(mitk::SUVPixelSemantics::PrenormalizedSUV == m.semantics);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0005, m.prenormScale, 1e-12);
  }

  void CNTS_SUVScaleWithNonBWSUVType_Throws_UnsupportedPETUnitsException()
  {
    // The SUV-scale factor produces SUVbw by definition, so an input
    // declaring a different SUV Type is internally inconsistent and must
    // not be silently read as BW.
    auto img = MakeImage();
    SetDicomTag(img, 0x0054, 0x1001, "CNTS");
    SetDicomTag(img, 0x0008, 0x0070, "Philips Medical Systems");
    SetDicomTag(img, 0x0054, 0x1006, "LBMJANMA");
    SetNamedString(img.GetPointer(), "mitk.pet.PhilipsSUVScale", "0.0005");

    CPPUNIT_ASSERT_THROW(mitk::ClassifyPETInput(img.GetPointer(),
                                                mitk::DICOMReadPolicy::Lenient),
                         mitk::UnsupportedPETUnitsException);
  }

  void CNTS_SUVScaleWithExplicitBWSUVType_Accepted()
  {
    // BW is what the factor yields, so stating it explicitly is consistent
    // and must not be refused -- the check has to discriminate, not just
    // reject any present SUV Type.
    auto img = MakeImage();
    SetDicomTag(img, 0x0054, 0x1001, "CNTS");
    SetDicomTag(img, 0x0008, 0x0070, "Philips Medical Systems");
    SetDicomTag(img, 0x0054, 0x1006, "BW");
    SetNamedString(img.GetPointer(), "mitk.pet.PhilipsSUVScale", "0.0005");

    const auto m = mitk::ClassifyPETInput(img.GetPointer(),
                                          mitk::DICOMReadPolicy::Lenient);
    CPPUNIT_ASSERT(mitk::SUVPixelSemantics::PrenormalizedSUV == m.semantics);
    CPPUNIT_ASSERT(mitk::SUVVariant::BW == m.sourceVariant);
  }

  void CNTS_ActivityScaleWithNonBWSUVType_Accepted()
  {
    // The activity-scale path yields Bq/mL and normalizes afterwards, so a
    // declared SUV Type places no constraint on it. Only the SUV-scale path
    // is checked.
    auto img = MakeImage();
    SetDicomTag(img, 0x0054, 0x1001, "CNTS");
    SetDicomTag(img, 0x0008, 0x0070, "Philips Medical Systems");
    SetDicomTag(img, 0x0054, 0x1006, "LBMJANMA");
    SetNamedString(img.GetPointer(), "mitk.pet.PhilipsActivityScale", "0.5");

    const auto m = mitk::ClassifyPETInput(img.GetPointer(),
                                          mitk::DICOMReadPolicy::Lenient);
    CPPUNIT_ASSERT(mitk::SUVPixelSemantics::ActivityConcentration == m.semantics);
  }

  void NoUnits_Throws_MissingDICOMPropertyException()
  {
    auto img = MakeImage();   // no (0054,1001) attached
    CPPUNIT_ASSERT_THROW(mitk::ClassifyPETInput(img.GetPointer(),
                                                mitk::DICOMReadPolicy::Lenient),
                         mitk::MissingDICOMPropertyException);
  }

  void UnknownUnits_Throws_UnsupportedPETUnitsException()
  {
    auto img = MakeImage();
    SetDicomTag(img, 0x0054, 0x1001, "FOO");
    CPPUNIT_ASSERT_THROW(mitk::ClassifyPETInput(img.GetPointer(),
                                                mitk::DICOMReadPolicy::Lenient),
                         mitk::UnsupportedPETUnitsException);
    // Same outcome under Strict (no IBSI adaptation defined).
    CPPUNIT_ASSERT_THROW(mitk::ClassifyPETInput(img.GetPointer(),
                                                mitk::DICOMReadPolicy::Strict),
                         mitk::UnsupportedPETUnitsException);
  }

  void CNTS_NonPhilipsManufacturer_Throws_UnsupportedPETUnitsException()
  {
    auto img = MakeImage();
    SetDicomTag(img, 0x0054, 0x1001, "CNTS");
    SetDicomTag(img, 0x0008, 0x0070, "SIEMENS Healthineers");
    CPPUNIT_ASSERT_THROW(mitk::ClassifyPETInput(img.GetPointer(),
                                                mitk::DICOMReadPolicy::Lenient),
                         mitk::UnsupportedPETUnitsException);
  }

  void CNTS_PhilipsNoFactor_Throws_MissingPhilipsPETScaleException()
  {
    auto img = MakeImage();
    SetDicomTag(img, 0x0054, 0x1001, "CNTS");
    SetDicomTag(img, 0x0008, 0x0070, "Philips Medical Systems");
    // No SUV-scale and no activity-scale property attached.
    CPPUNIT_ASSERT_THROW(mitk::ClassifyPETInput(img.GetPointer(),
                                                mitk::DICOMReadPolicy::Lenient),
                         mitk::MissingPhilipsPETScaleException);
    // Same outcome under Strict.
    CPPUNIT_ASSERT_THROW(mitk::ClassifyPETInput(img.GetPointer(),
                                                mitk::DICOMReadPolicy::Strict),
                         mitk::MissingPhilipsPETScaleException);
  }

  // ---- SUV Type round trip ----
  //
  // ApplyOutputTagPolicy stamps (0054,1006) on every SUV image this module
  // produces, and (0054,1001) = GML plus a SUV Type is exactly the
  // pre-normalized input that DRO_2_1_x and DRO_2_6_x exercise. So an SUV
  // output is a legitimate input, and the classifier has to read back the
  // variant the filter wrote. These cases pin each lean-body-mass code
  // separately: they once all collapsed onto the generic "LBM", which made
  // the output unreadable and -- once "LBM" gained a meaning -- would have
  // resolved a Janmahasatian image to Morgan instead.

  void GML_SUVTypeLBMJANMA_ClassifiesAsJanmahasatian()
  {
    auto img = MakeImage();
    SetDicomTag(img, 0x0054, 0x1001, "GML");
    SetDicomTag(img, 0x0054, 0x1006, "LBMJANMA");

    const auto m = mitk::ClassifyPETInput(img.GetPointer(),
                                          mitk::DICOMReadPolicy::Lenient);
    CPPUNIT_ASSERT(mitk::SUVVariant::LBM_Janmahasatian == m.sourceVariant);
  }

  void GML_SUVTypeLBMJAMES128_ClassifiesAsJames128()
  {
    auto img = MakeImage();
    SetDicomTag(img, 0x0054, 0x1001, "GML");
    SetDicomTag(img, 0x0054, 0x1006, "LBMJAMES128");

    const auto m = mitk::ClassifyPETInput(img.GetPointer(),
                                          mitk::DICOMReadPolicy::Lenient);
    CPPUNIT_ASSERT(mitk::SUVVariant::LBM_James128 == m.sourceVariant);
  }

  void GML_SUVTypeLBM_ClassifiesAsMorgan()
  {
    // Previously refused outright. The IBSI-SUV manual calls the formula
    // obsolete but lists it among the convertible SUV Types, and data
    // carrying it exists.
    auto img = MakeImage();
    SetDicomTag(img, 0x0054, 0x1001, "GML");
    SetDicomTag(img, 0x0054, 0x1006, "LBM");

    const auto m = mitk::ClassifyPETInput(img.GetPointer(),
                                          mitk::DICOMReadPolicy::Lenient);
    CPPUNIT_ASSERT(mitk::SUVPixelSemantics::PrenormalizedSUV == m.semantics);
    CPPUNIT_ASSERT(mitk::SUVVariant::LBM_Morgan == m.sourceVariant);
  }

  // ---- Enhanced PET Image Storage ----
  //
  // These objects carry none of the classic attributes, so the classifier
  // reads the unit out of the functional groups instead. The guards below
  // matter as much as the mapping: MITK models one frame per file, so a
  // per-frame value that differs between frames would silently be applied
  // to the whole volume.

  void EnhancedPET_BqMl_ClassifiesAsActivityConcentration()
  {
    auto img = MakeEnhancedImage(4, "Bq/ml", {"1.0", "1.0", "1.0", "1.0"});

    const auto m = mitk::ClassifyEnhancedPETInput(img.GetPointer(),
                                                  mitk::DICOMReadPolicy::Lenient);
    CPPUNIT_ASSERT(mitk::SUVPixelSemantics::ActivityConcentration == m.semantics);
  }

  void EnhancedPET_GmlSUVbw_ClassifiesAsPrenormalizedBW()
  {
    auto img = MakeEnhancedImage(4, "g/ml{SUVbw}", {"1.0", "1.0", "1.0", "1.0"});

    const auto m = mitk::ClassifyEnhancedPETInput(img.GetPointer(),
                                                  mitk::DICOMReadPolicy::Lenient);
    CPPUNIT_ASSERT(mitk::SUVPixelSemantics::PrenormalizedSUV == m.semantics);
    CPPUNIT_ASSERT(mitk::SUVVariant::BW == m.sourceVariant);
  }

  void EnhancedPET_UniformPerFrameRescale_Accepted()
  {
    // The guard must detect variation, not the mere existence of per-frame
    // values -- otherwise it degenerates into "refuse all Enhanced PET".
    auto img = MakeEnhancedImage(4, "Bq/ml", {"4.0", "4.0", "4.0", "4.0"});

    CPPUNIT_ASSERT_NO_THROW(mitk::ClassifyEnhancedPETInput(
      img.GetPointer(), mitk::DICOMReadPolicy::Lenient));
  }

  void EnhancedPET_RescaleWrittenTwoWays_Accepted()
  {
    // "4" and "4.0" are the same slope written in two VRs -- DS through the
    // Pixel Value Transformation Sequence, FD through the Real World Value
    // Mapping Sequence. Comparing the strings would report variation on a
    // perfectly uniform object.
    auto img = MakeEnhancedImage(3, "Bq/ml", {"4", "4.0", "4.00"});

    CPPUNIT_ASSERT_NO_THROW(mitk::ClassifyEnhancedPETInput(
      img.GetPointer(), mitk::DICOMReadPolicy::Lenient));
  }

  void EnhancedPET_VaryingPerFrameRescale_Refuses()
  {
    auto img = MakeEnhancedImage(4, "Bq/ml", {"4.0", "4.0", "3.0", "4.0"});

    CPPUNIT_ASSERT_THROW(mitk::ClassifyEnhancedPETInput(img.GetPointer(),
                                                        mitk::DICOMReadPolicy::Lenient),
                         mitk::EnhancedPETPerFrameVariationException);
  }

  void EnhancedPET_FewerRescaleValuesThanFrames_Refuses()
  {
    // A path that resolves to fewer values than the object has frames has
    // not observed the per-frame values; concluding "uniform" would be an
    // accident. This is the failure mode a mis-specified or unregistered
    // tag path produces, and it must not read as success.
    auto img = MakeEnhancedImage(4, "Bq/ml", {"4.0", "4.0"});

    CPPUNIT_ASSERT_THROW(mitk::ClassifyEnhancedPETInput(img.GetPointer(),
                                                        mitk::DICOMReadPolicy::Lenient),
                         mitk::EnhancedPETPerFrameVariationException);
  }

  void EnhancedPET_UnitDiffersBetweenFrames_Refuses()
  {
    auto img = MakeEnhancedImage(2, "Bq/ml", {"1.0", "1.0"});
    // Overwrite the second frame's unit so the two disagree.
    SetProperty(img.GetPointer(),
                mitk::DICOMTagPathToPropertyName(
                  mitk::DICOMTagPath()
                    .AddSelection(0x5200, 0x9230, 1)
                    .AddSelection(0x0040, 0x9096, 0)
                    .AddSelection(0x0040, 0x08EA, 0)
                    .AddElement(0x0008, 0x0100)),
                "g/ml{SUVbw}");

    CPPUNIT_ASSERT_THROW(mitk::ClassifyEnhancedPETInput(img.GetPointer(),
                                                        mitk::DICOMReadPolicy::Lenient),
                         mitk::EnhancedPETPerFrameVariationException);
  }

  void EnhancedPET_NoUsableUnit_Throws()
  {
    auto img = MakeEnhancedImage(2, "", {"1.0", "1.0"});

    CPPUNIT_ASSERT_THROW(mitk::ClassifyEnhancedPETInput(img.GetPointer(),
                                                        mitk::DICOMReadPolicy::Lenient),
                         mitk::MissingDICOMPropertyException);
  }

  void EnhancedPET_RescaleTypeFallback_Accepted()
  {
    // No Measurement Units Code Sequence, but Rescale Type names a unit the
    // pipeline converts. The manual marks this a fallback because Rescale
    // Type is supposed to be "US" for PET.
    auto img = MakeEnhancedImage(2, "", {"1.0", "1.0"});
    SetProperty(img.GetPointer(), PerFrameName(0, 0x0028, 0x9145, 0, 0x0028, 0x1054), "BQML");

    const auto m = mitk::ClassifyEnhancedPETInput(img.GetPointer(),
                                                  mitk::DICOMReadPolicy::Lenient);
    CPPUNIT_ASSERT(mitk::SUVPixelSemantics::ActivityConcentration == m.semantics);
  }

  void IsEnhancedPETInput_OnlyForTheEnhancedSOPClass()
  {
    // The Enhanced PET code path must be unreachable for anything else, or
    // classic PET inputs would start taking it.
    auto enhanced = MakeImage();
    SetDicomTag(enhanced, 0x0008, 0x0016, "1.2.840.10008.5.1.4.1.1.130");
    CPPUNIT_ASSERT(mitk::IsEnhancedPETInput(enhanced.GetPointer()));

    auto classic = MakeImage();
    SetDicomTag(classic, 0x0008, 0x0016, "1.2.840.10008.5.1.4.1.1.128");
    CPPUNIT_ASSERT(!mitk::IsEnhancedPETInput(classic.GetPointer()));

    auto none = MakeImage();
    CPPUNIT_ASSERT(!mitk::IsEnhancedPETInput(none.GetPointer()));
  }

  void NullProvider_Throws()
  {
    CPPUNIT_ASSERT_THROW(mitk::ClassifyPETInput(nullptr,
                                                mitk::DICOMReadPolicy::Lenient),
                         mitk::Exception);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkSUVInputModel)
