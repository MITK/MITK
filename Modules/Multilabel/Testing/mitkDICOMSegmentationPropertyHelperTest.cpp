/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkDICOMSegmentationPropertyHelper.h>
#include <mitkImage.h>
#include <mitkLabelSetImage.h>
#include <mitkPropertyNameHelper.h>
#include <mitkTemporoSpatialStringProperty.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

namespace
{
  using Helper = mitk::DICOMSegmentationPropertyHelper;

  std::string DICOMKey(unsigned int g, unsigned int e)
  {
    return mitk::GeneratePropertyNameForDICOMTag(g, e);
  }

  // Source carrying the Patient / Study / FrameOfReference identifying tags
  // the helper's InheritXxxFromSource functions transfer. Per-instance and
  // series UIDs are out of scope here (those belong to the rule's tests).
  mitk::Image::Pointer MakeFakeSourceImage(const std::string& studyUID = "1.2.3.4.study",
                                           const std::string& forUID = "1.2.3.4.for",
                                           const std::string& patientName = "1.2.3.4.patient",
                                           const std::string& patientID = "1.2.3.4.pid",
                                           const std::string& studyID = "1.2.3.4.sid")
  {
    auto image = mitk::Image::New();
    unsigned int dim[3] = {2u, 2u, 2u};
    image->Initialize(mitk::MakeScalarPixelType<unsigned char>(), 3, dim);

    image->SetProperty(DICOMKey(0x0020, 0x000d).c_str(),
                       mitk::TemporoSpatialStringProperty::New(studyUID));
    image->SetProperty(DICOMKey(0x0020, 0x0052).c_str(),
                       mitk::TemporoSpatialStringProperty::New(forUID));
    image->SetProperty(DICOMKey(0x0010, 0x0010).c_str(),
                       mitk::TemporoSpatialStringProperty::New(patientName));
    image->SetProperty(DICOMKey(0x0010, 0x0020).c_str(),
                       mitk::TemporoSpatialStringProperty::New(patientID));
    image->SetProperty(DICOMKey(0x0020, 0x0010).c_str(),
                       mitk::TemporoSpatialStringProperty::New(studyID));
    return image;
  }

  mitk::MultiLabelSegmentation::Pointer MakeFreshSeg()
  {
    return mitk::MultiLabelSegmentation::New();
  }

  std::string ReadStringProp(const mitk::MultiLabelSegmentation* seg,
                             unsigned int g, unsigned int e)
  {
    auto prop = seg->GetConstProperty(DICOMKey(g, e));
    return prop.IsNotNull() ? prop->GetValueAsString() : std::string();
  }

  bool HasProp(const mitk::MultiLabelSegmentation* seg,
               unsigned int g, unsigned int e)
  {
    return seg->GetConstProperty(DICOMKey(g, e)).IsNotNull();
  }
}

class mitkDICOMSegmentationPropertyHelperTestSuite : public mitk::TestFixture
{
  // Rule-side behaviour (CanonicalPurposeTag, Connect, Disconnect) is
  // covered in mitkSegSourceImageRelationRuleTest. This suite focuses on the
  // helper's own responsibilities: construction-time defaults, identity
  // inheritance from source, and the Validate/Complete contract.
  CPPUNIT_TEST_SUITE(mitkDICOMSegmentationPropertyHelperTestSuite);
  MITK_TEST(ConstructorStampsMitkBrandingOnly);

  MITK_TEST(InheritPatientFromSourceOverwritesByDefault);
  MITK_TEST(InheritPatientFromSourceFillsMissingWhenNoOverwrite);
  MITK_TEST(InheritStudyFromSourceOverwritesByDefault);
  MITK_TEST(InheritStudyFromSourceFillsMissingWhenNoOverwrite);
  MITK_TEST(InheritFrameOfReferenceFromSourceOverwritesByDefault);
  MITK_TEST(InheritFromSourceLeavesSeriesInstanceUIDUntouched);

  MITK_TEST(ValidateThrowsOnNullSeg);
  MITK_TEST(CompleteThrowsOnNullSeg);
  MITK_TEST(ValidateClearsAfterCompleteIdentity);
  MITK_TEST(CompleteFillIfMissingPreservesMitkBranding);
  CPPUNIT_TEST_SUITE_END();

public:
  void ConstructorStampsMitkBrandingOnly()
  {
    // The constructor routes through Complete(seg, {}) which stamps only
    // class invariants and MITK branding. Identifying tags
    // (PatientName, PatientID, StudyID, CT time point, CT center) are NOT
    // stamped so a later Initialize(template) or InheritXxxFromSource can
    // adopt the source's real values.
    const auto seg = MakeFreshSeg();
    CPPUNIT_ASSERT_EQUAL(std::string("SEG"), ReadStringProp(seg, 0x0008, 0x0060));
    CPPUNIT_ASSERT_EQUAL(std::string("MITK Segmentation"), ReadStringProp(seg, 0x0008, 0x103E));
    CPPUNIT_ASSERT_EQUAL(std::string("MITK"), ReadStringProp(seg, 0x0070, 0x0084));
    CPPUNIT_ASSERT_EQUAL(Helper::UnknownClinicalTrialSeriesID(),
                         ReadStringProp(seg, 0x0012, 0x0071));

    CPPUNIT_ASSERT_MESSAGE("PatientName must not be stamped at construction.",
                           !HasProp(seg, 0x0010, 0x0010));
    CPPUNIT_ASSERT_MESSAGE("PatientID must not be stamped at construction.",
                           !HasProp(seg, 0x0010, 0x0020));
    CPPUNIT_ASSERT_MESSAGE("StudyID must not be stamped at construction.",
                           !HasProp(seg, 0x0020, 0x0010));
    CPPUNIT_ASSERT_MESSAGE("ClinicalTrialTimePointID must not be stamped at construction.",
                           !HasProp(seg, 0x0012, 0x0050));
    CPPUNIT_ASSERT_MESSAGE("ClinicalTrialCoordinatingCenterName must not be stamped at construction.",
                           !HasProp(seg, 0x0012, 0x0060));
  }

  void InheritPatientFromSourceOverwritesByDefault()
  {
    const auto seg = MakeFreshSeg();
    const auto source = MakeFakeSourceImage();

    Helper::InheritPatientFromSource(seg, source);

    CPPUNIT_ASSERT_EQUAL(std::string("1.2.3.4.patient"), ReadStringProp(seg, 0x0010, 0x0010));
    CPPUNIT_ASSERT_EQUAL(std::string("1.2.3.4.pid"),     ReadStringProp(seg, 0x0010, 0x0020));
  }

  void InheritPatientFromSourceFillsMissingWhenNoOverwrite()
  {
    // overwrite=false fills the missing slot. The constructor no longer
    // stamps identifying tags, so the slot is genuinely empty and the
    // source's value lands.
    const auto seg = MakeFreshSeg();
    const auto source = MakeFakeSourceImage();

    Helper::InheritPatientFromSource(seg, source, /*overwrite=*/false);

    CPPUNIT_ASSERT_EQUAL(std::string("1.2.3.4.patient"), ReadStringProp(seg, 0x0010, 0x0010));
    CPPUNIT_ASSERT_EQUAL(std::string("1.2.3.4.pid"),     ReadStringProp(seg, 0x0010, 0x0020));
  }

  void InheritStudyFromSourceOverwritesByDefault()
  {
    const auto seg = MakeFreshSeg();
    const auto source = MakeFakeSourceImage();

    Helper::InheritStudyFromSource(seg, source);

    CPPUNIT_ASSERT_EQUAL(std::string("1.2.3.4.sid"),   ReadStringProp(seg, 0x0020, 0x0010));
    CPPUNIT_ASSERT_EQUAL(std::string("1.2.3.4.study"), ReadStringProp(seg, 0x0020, 0x000d));
  }

  void InheritStudyFromSourceFillsMissingWhenNoOverwrite()
  {
    // Pre-set StudyID explicitly; overwrite=false must keep it.
    // StudyInstanceUID is missing and must be filled.
    const auto seg = MakeFreshSeg();
    seg->SetProperty(DICOMKey(0x0020, 0x0010).c_str(),
                     mitk::TemporoSpatialStringProperty::New("seg-own-studyID"));
    const auto source = MakeFakeSourceImage();

    Helper::InheritStudyFromSource(seg, source, /*overwrite=*/false);

    CPPUNIT_ASSERT_EQUAL(std::string("seg-own-studyID"), ReadStringProp(seg, 0x0020, 0x0010));
    CPPUNIT_ASSERT_EQUAL(std::string("1.2.3.4.study"),   ReadStringProp(seg, 0x0020, 0x000d));
  }

  void InheritFrameOfReferenceFromSourceOverwritesByDefault()
  {
    const auto seg = MakeFreshSeg();
    const auto source = MakeFakeSourceImage();

    Helper::InheritFrameOfReferenceFromSource(seg, source);

    CPPUNIT_ASSERT_EQUAL(std::string("1.2.3.4.for"), ReadStringProp(seg, 0x0020, 0x0052));
  }

  void InheritFromSourceLeavesSeriesInstanceUIDUntouched()
  {
    // The seg's SeriesInstanceUID is the SEG's own series identity, not the
    // source's. None of the InheritXxxFromSource functions should copy it.
    const auto seg = MakeFreshSeg();
    const auto source = MakeFakeSourceImage();

    Helper::InheritPatientFromSource(seg, source);
    Helper::InheritStudyFromSource(seg, source);
    Helper::InheritFrameOfReferenceFromSource(seg, source);

    CPPUNIT_ASSERT_MESSAGE("Inherit must not transfer SeriesInstanceUID.",
      seg->GetConstProperty(DICOMKey(0x0020, 0x000e)).IsNull());
  }

  void ValidateThrowsOnNullSeg()
  {
    CPPUNIT_ASSERT_THROW(Helper::Validate(nullptr), mitk::Exception);
  }

  void CompleteThrowsOnNullSeg()
  {
    Helper::CompletionOptions options;
    CPPUNIT_ASSERT_THROW(Helper::Complete(nullptr, options), mitk::Exception);
  }

  void ValidateClearsAfterCompleteIdentity()
  {
    const auto seg = MakeFreshSeg();

    Helper::CompletionOptions options;
    options.synthesizeMissingIdentity = true;
    Helper::Complete(seg, options);

    const auto missing = Helper::Validate(seg);
    for (const auto& item : missing)
    {
      CPPUNIT_ASSERT_MESSAGE(
        "Validate must not report Segmentation-level missing items after "
        "Complete(synthesizeMissingIdentity).",
        item.scope != Helper::MissingItem::Scope::Segmentation);
    }
  }

  void CompleteFillIfMissingPreservesMitkBranding()
  {
    const auto seg = MakeFreshSeg();
    Helper::CompletionOptions options;
    options.synthesizeMissingIdentity = true;
    Helper::Complete(seg, options);

    // ContentCreator was stamped "MITK" by the constructor; Complete's
    // synthesis path would otherwise stamp the UnknownContentCreatorName
    // placeholder. Fill-only-if-missing preserves the branding value.
    CPPUNIT_ASSERT_EQUAL(std::string("MITK"), ReadStringProp(seg, 0x0070, 0x0084));
    CPPUNIT_ASSERT_EQUAL(std::string("MITK Segmentation"), ReadStringProp(seg, 0x0008, 0x103E));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkDICOMSegmentationPropertyHelper)
