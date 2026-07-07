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
  MITK_TEST(GML_SUVType_LBM_Morgan_Throws);
  MITK_TEST(GML_SUVType_BSA_Throws_Inconsistent);
  MITK_TEST(GML_SUVType_Unknown_Throws);
  MITK_TEST(CM2ML_SUVType_BSA_OK);
  MITK_TEST(CM2ML_SUVType_NotBSA_Throws);

  // Whitespace / case insensitivity at the value boundary
  MITK_TEST(LowercaseAndPaddedUnits_StillRecognised);

  // Philips CNTS variants
  MITK_TEST(CNTS_PhilipsSUVScale_ClassifiesAsPrenormalizedBW);
  MITK_TEST(CNTS_PhilipsActivityScale_ClassifiesAsActivityConcentration);
  MITK_TEST(CNTS_PhilipsBothFactorsPresent_PrefersSUVScale);

  // Error matrix
  MITK_TEST(NoUnits_Throws_MissingDICOMPropertyException);
  MITK_TEST(UnknownUnits_Throws_UnsupportedPETUnitsException);
  MITK_TEST(CNTS_NonPhilipsManufacturer_Throws_UnsupportedPETUnitsException);
  MITK_TEST(CNTS_PhilipsNoFactor_Throws_MissingPhilipsPETScaleException);

  // Defensive contract
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

  void GML_SUVType_LBM_Morgan_Throws()
  {
    // SUV Type 'LBM' (Morgan) is explicitly called out by the IBSI-SUV
    // spec as obsolete and not covered by the DROs; we refuse it
    // rather than silently picking a different LBM formula.
    auto img = MakeImage();
    SetDicomTag(img, 0x0054, 0x1001, "GML");
    SetDicomTag(img, 0x0054, 0x1006, "LBM");

    CPPUNIT_ASSERT_THROW(mitk::ClassifyPETInput(img.GetPointer(),
                                                mitk::DICOMReadPolicy::Lenient),
                         mitk::UnsupportedPETUnitsException);
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

  void CNTS_PhilipsBothFactorsPresent_PrefersSUVScale()
  {
    // The two factors are mutually exclusive in real Philips data; the
    // synthetic input here exercises the ambiguity case explicitly.
    // When both happen to be present, the classifier prefers the
    // SUV-scale (pre-normalized) interpretation; pinning that contract.
    auto img = MakeImage();
    SetDicomTag(img, 0x0054, 0x1001, "CNTS");
    SetDicomTag(img, 0x0008, 0x0070, "Philips Medical Systems");
    SetNamedString(img.GetPointer(), "mitk.pet.PhilipsSUVScale",      "0.0005");
    SetNamedString(img.GetPointer(), "mitk.pet.PhilipsActivityScale", "0.5");

    const auto m = mitk::ClassifyPETInput(img.GetPointer(),
                                          mitk::DICOMReadPolicy::Lenient);
    CPPUNIT_ASSERT(mitk::SUVPixelSemantics::PrenormalizedSUV == m.semantics);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0005, m.prenormScale, 1e-12);
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

  void NullProvider_Throws()
  {
    CPPUNIT_ASSERT_THROW(mitk::ClassifyPETInput(nullptr,
                                                mitk::DICOMReadPolicy::Lenient),
                         mitk::Exception);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkSUVInputModel)
