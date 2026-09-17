/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkIOUtil.h>
#include <mitkImage.h>
#include <mitkPreferenceListReaderOptionsFunctor.h>
#include <mitkPropertyKeyPath.h>
#include <mitkPropertyNameHelper.h>
#include <mitkSourceImageRelationRule.h>
#include <mitkTemporoSpatialStringProperty.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <regex>

/**
 * \brief Covers SourceImageRelationRule against a destination the DICOM
 *        reader actually produced.
 *
 * The rule's own suite in MitkCore builds its destinations by hand, setting
 * the DICOM identity tags it needs. That leaves the question this suite
 * answers untested: whether a real reader supplies those tags at all. The
 * tags reach an image only through the tags-of-interest registry, so a gap
 * there disables the rule's data layer for every DICOM-read image while
 * every hand-built test keeps passing.
 */
class mitkDICOMSourceImageRelationTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkDICOMSourceImageRelationTestSuite);
  MITK_TEST(ReaderProducesSOPClassUIDAsDICOMProperty);
  MITK_TEST(ConnectAgainstReaderProducedImageReachesCompleteRelation);
  CPPUNIT_TEST_SUITE_END();

private:
  mitk::Image::Pointer m_Source;

  bool HasPropertyMatching(const mitk::IPropertyProvider* provider,
                           const mitk::PropertyKeyPath& path) const
  {
    const std::regex regEx(mitk::PropertyKeyPathToPropertyRegEx(path));

    for (const auto& key : provider->GetPropertyKeys())
    {
      if (std::regex_match(key, regEx))
        return true;
    }

    return false;
  }

public:
  void setUp() override
  {
    // Load through the reader service, not DICOMFileReaderSelector: the
    // service is what hands the reader the tags-of-interest map and the
    // DICOM property functor. A selector-driven load does neither, so it
    // would pass this suite regardless of what the registry contains.
    mitk::PreferenceListReaderOptionsFunctor readerFunctor(
      {"MITK DICOM Reader v2 (autoselect)"}, {""});

    // Name one slice rather than the directory. TinyCTAbdomen is built to
    // split into several blocks - files 100-119 vary orientation, pixel
    // spacing and even SOP class - so a directory load would leave the suite
    // grading whichever block the reader happened to emit first. Naming a
    // file restricts the read to the block that file belongs to.
    m_Source = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("TinyCTAbdomen/130"),
                                               &readerFunctor);

    CPPUNIT_ASSERT_MESSAGE("Test precondition: the DICOM series loads as an image",
                           m_Source.IsNotNull());
  }

  void tearDown() override { m_Source = nullptr; }

  void ReaderProducesSOPClassUIDAsDICOMProperty()
  {
    const auto instanceUIDProp =
      m_Source->GetConstProperty(mitk::GeneratePropertyNameForDICOMTag(0x0008, 0x0018));
    CPPUNIT_ASSERT_MESSAGE("Reader produces the SOP Instance UID property",
                           instanceUIDProp.IsNotNull());

    const auto classUIDProp =
      m_Source->GetConstProperty(mitk::GeneratePropertyNameForDICOMTag(0x0008, 0x0016));
    CPPUNIT_ASSERT_MESSAGE("Reader produces the SOP Class UID property",
                           classUIDProp.IsNotNull());

    // The rule clones this property into the referenced-SOP-Class slot of the
    // Source Image Sequence, and consumers downcast the clone to read it per
    // slice. A block-level StringProperty would survive the clone and fail
    // that downcast silently, so the dynamic type is part of the contract.
    CPPUNIT_ASSERT_MESSAGE(
      "SOP Class UID property carries per-slice values",
      nullptr != dynamic_cast<const mitk::TemporoSpatialStringProperty*>(classUIDProp.GetPointer()));
  }

  void ConnectAgainstReaderProducedImageReachesCompleteRelation()
  {
    auto derived = mitk::Image::New();
    auto rule = mitk::SourceImageRelationRule::New("Test");

    rule->Connect(derived, m_Source);

    CPPUNIT_ASSERT_MESSAGE(
      "Relation against a reader-produced source spans the ID and data layers",
      rule->HasRelation(derived, m_Source,
                        mitk::PropertyRelationRuleBase::RelationType::Complete));

    mitk::PropertyKeyPath refInstanceUIDPath;
    refInstanceUIDPath.AddElement("DICOM").AddElement("0008").AddAnySelection("2112")
      .AddElement("0008").AddElement("1155");
    CPPUNIT_ASSERT_MESSAGE("Source Image Sequence references a source SOP Instance UID",
                           this->HasPropertyMatching(derived, refInstanceUIDPath));

    mitk::PropertyKeyPath refClassUIDPath;
    refClassUIDPath.AddElement("DICOM").AddElement("0008").AddAnySelection("2112")
      .AddElement("0008").AddElement("1150");
    CPPUNIT_ASSERT_MESSAGE("Source Image Sequence references a source SOP Class UID",
                           this->HasPropertyMatching(derived, refClassUIDPath));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkDICOMSourceImageRelation)
