/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkImage.h>
#include <mitkLabelSetImage.h>
#include <mitkPropertyKeyPath.h>
#include <mitkPropertyList.h>
#include <mitkPropertyNameHelper.h>
#include <mitkPropertyRelationRuleBase.h>
#include <mitkSegSourceImageRelationRule.h>
#include <mitkTemporoSpatialStringProperty.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

namespace
{
  using Rule = mitk::SegSourceImageRelationRule;

  std::string DICOMKey(unsigned int g, unsigned int e)
  {
    return mitk::GeneratePropertyNameForDICOMTag(g, e);
  }

  // A source image carrying the DICOM identifying tags the rule's data
  // layer captures. Geometry initialization is the minimum needed for the
  // seg's own Initialize to succeed.
  mitk::Image::Pointer MakeDicomSource(const std::string& sopInstanceUID = "1.2.3.4.instance",
                                       const std::string& sopClassUID = "1.2.3.4.class",
                                       const std::string& seriesUID = "1.2.3.4.series")
  {
    auto image = mitk::Image::New();
    unsigned int dim[3] = {2u, 2u, 2u};
    image->Initialize(mitk::MakeScalarPixelType<unsigned char>(), 3, dim);
    image->SetProperty(DICOMKey(0x0008, 0x0018).c_str(),
                       mitk::TemporoSpatialStringProperty::New(sopInstanceUID));
    image->SetProperty(DICOMKey(0x0008, 0x0016).c_str(),
                       mitk::TemporoSpatialStringProperty::New(sopClassUID));
    image->SetProperty(DICOMKey(0x0020, 0x000e).c_str(),
                       mitk::TemporoSpatialStringProperty::New(seriesUID));
    return image;
  }

  mitk::Image::Pointer MakeNonDicomSource()
  {
    auto image = mitk::Image::New();
    unsigned int dim[3] = {1u, 1u, 1u};
    image->Initialize(mitk::MakeScalarPixelType<unsigned char>(), 3, dim);
    return image;
  }
}

class mitkSegSourceImageRelationRuleTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkSegSourceImageRelationRuleTestSuite);

  // Identity / type binding
  MITK_TEST(CanonicalPurposeTagReturnsExpectedValue);
  MITK_TEST(DefaultConstructedRuleCarriesCanonicalPurposeTag);

  // Connect, producer path (typed Image* convenience)
  MITK_TEST(ConnectProducesNonEmptyRelationUID);
  MITK_TEST(ConnectCapturesSourceSeriesInstanceUID);
  MITK_TEST(ConnectDoesNotTouchSegOwnIdentity);
  MITK_TEST(ConnectOnNonDicomSourceEstablishesIDLayerRelation);
  MITK_TEST(TwoDifferentSourcesProduceTwoDistinctRelations);

  // Connect, reader path (IPropertyProvider via a manually-built provider)
  MITK_TEST(ConnectViaPropertyProviderReachesEquivalentState);

  // Lifecycle: Disconnect must also remove the rule's extension property
  MITK_TEST(DisconnectRemovesPerRelationSourceSeriesUID);

  CPPUNIT_TEST_SUITE_END();

public:
  void CanonicalPurposeTagReturnsExpectedValue()
  {
    // The DCM code value DCMQI emits for SEG source-image references; pinned
    // so a regression in either side surfaces explicitly.
    CPPUNIT_ASSERT_EQUAL(std::string("121322"), Rule::CanonicalPurposeTag());
  }

  void DefaultConstructedRuleCarriesCanonicalPurposeTag()
  {
    // The class doc claims the canonical purpose tag is type-bound; this
    // pins the invariant against accidental ctor changes.
    auto rule = Rule::New();
    const auto ruleId = rule->GetRuleID();
    CPPUNIT_ASSERT_MESSAGE(
      "Rule ID should contain the canonical purpose tag '121322'.",
      ruleId.find("121322") != std::string::npos);
  }

  void ConnectProducesNonEmptyRelationUID()
  {
    auto seg = mitk::MultiLabelSegmentation::New();
    const auto source = MakeDicomSource();

    const auto uid = Rule::Connect(seg, source);
    CPPUNIT_ASSERT_MESSAGE("Connect must return a non-empty relation UID.", !uid.empty());
  }

  void ConnectCapturesSourceSeriesInstanceUID()
  {
    auto seg = mitk::MultiLabelSegmentation::New();
    const auto source = MakeDicomSource("inst-1", "class-1", "series-X");

    Rule::Connect(seg, source);

    const auto relations = Rule::GetSourceImageRelations(seg);
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1u), relations.size());
    CPPUNIT_ASSERT_EQUAL(std::string("series-X"), relations.front().sourceSeriesInstanceUID);
  }

  void ConnectDoesNotTouchSegOwnIdentity()
  {
    // The rule models a derivation relation only. Patient / study /
    // frame-of-reference inheritance is the helper's
    // InheritXxxFromSource responsibility, not the rule's. Pinning this
    // boundary prevents a future change from quietly mixing concerns.
    auto seg = mitk::MultiLabelSegmentation::New();
    auto source = MakeDicomSource();
    source->SetProperty(DICOMKey(0x0010, 0x0010).c_str(),
                        mitk::TemporoSpatialStringProperty::New("SOURCE_PATIENT"));
    source->SetProperty(DICOMKey(0x0020, 0x000d).c_str(),
                        mitk::TemporoSpatialStringProperty::New("SOURCE_STUDY_UID"));
    source->SetProperty(DICOMKey(0x0020, 0x0052).c_str(),
                        mitk::TemporoSpatialStringProperty::New("SOURCE_FOR_UID"));

    Rule::Connect(seg, source);

    // PatientName, StudyInstanceUID, FrameOfReferenceUID on seg must NOT
    // have been copied from source by Connect.
    const auto segPatient = seg->GetConstProperty(DICOMKey(0x0010, 0x0010));
    CPPUNIT_ASSERT(segPatient.IsNull() || segPatient->GetValueAsString() != "SOURCE_PATIENT");
    CPPUNIT_ASSERT(seg->GetConstProperty(DICOMKey(0x0020, 0x000d)).IsNull());
    CPPUNIT_ASSERT(seg->GetConstProperty(DICOMKey(0x0020, 0x0052)).IsNull());
  }

  void ConnectOnNonDicomSourceEstablishesIDLayerRelation()
  {
    // A NIfTI-style source has no DICOM identifying tags. The data layer
    // cannot capture them, but the ID layer (via Identifiable) still
    // produces a relation. This is what allows downstream tools to
    // reference segs of non-DICOM provenance via relation rules.
    auto seg = mitk::MultiLabelSegmentation::New();
    const auto source = MakeNonDicomSource();

    const auto relationUID = Rule::Connect(seg, source);
    CPPUNIT_ASSERT_MESSAGE(
      "Connect must establish an ID-layer relation even without DICOM tags on source.",
      !relationUID.empty());

    const auto relations = Rule::GetSourceImageRelations(seg);
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1u), relations.size());
    CPPUNIT_ASSERT_MESSAGE("Source-series UID must be empty when source had no DICOM tags.",
                           relations.front().sourceSeriesInstanceUID.empty());
  }

  void TwoDifferentSourcesProduceTwoDistinctRelations()
  {
    // Multi-source segmentations (e.g. multi-modal fusion) need each
    // Connect to create its own relation entry; collapsing them would
    // lose source identity at the writer.
    auto seg = mitk::MultiLabelSegmentation::New();
    const auto sourceA = MakeDicomSource("inst-A", "class-A", "series-A");
    const auto sourceB = MakeDicomSource("inst-B", "class-B", "series-B");

    const auto uidA = Rule::Connect(seg, sourceA);
    const auto uidB = Rule::Connect(seg, sourceB);
    CPPUNIT_ASSERT_MESSAGE("Distinct sources must yield distinct relation UIDs.", uidA != uidB);

    const auto relations = Rule::GetSourceImageRelations(seg);
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(2u), relations.size());

    const bool seriesAFound = relations[0].sourceSeriesInstanceUID == "series-A" ||
                              relations[1].sourceSeriesInstanceUID == "series-A";
    const bool seriesBFound = relations[0].sourceSeriesInstanceUID == "series-B" ||
                              relations[1].sourceSeriesInstanceUID == "series-B";
    CPPUNIT_ASSERT(seriesAFound && seriesBFound);
  }

  void ConnectViaPropertyProviderReachesEquivalentState()
  {
    // The SEG reader cannot connect against an Image (source DICOM instances
    // live only as metadata on the SEG file). It synthesizes a PropertyList
    // carrying the three identifying values and uses the instance Connect
    // overload. This test pins that the resulting state is enumerable in the
    // same shape as the producer-path Connect produces, so the reader's
    // call-site code path is covered without coupling the rule to a
    // "build me a provider" convenience.
    auto seg = mitk::MultiLabelSegmentation::New();

    auto provider = mitk::PropertyList::New();
    provider->SetProperty(DICOMKey(0x0008, 0x0018).c_str(),
                          mitk::TemporoSpatialStringProperty::New("inst-Z"));
    provider->SetProperty(DICOMKey(0x0008, 0x0016).c_str(),
                          mitk::TemporoSpatialStringProperty::New("class-Z"));
    provider->SetProperty(DICOMKey(0x0020, 0x000e).c_str(),
                          mitk::TemporoSpatialStringProperty::New("series-Z"));

    auto rule = Rule::New();
    const auto relationUID = rule->Connect(seg, provider);
    CPPUNIT_ASSERT(!relationUID.empty());

    const auto relations = Rule::GetSourceImageRelations(seg);
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1u), relations.size());
    CPPUNIT_ASSERT_EQUAL(std::string("series-Z"), relations.front().sourceSeriesInstanceUID);
  }

  void DisconnectRemovesPerRelationSourceSeriesUID()
  {
    // The rule extends the base's data layer with a per-relation
    // SourceSeriesInstanceUID slot. The class doc claims Disconnect cleans
    // it up via virtual dispatch; verify that no rule extension property
    // survives the disconnect.
    auto seg = mitk::MultiLabelSegmentation::New();
    const auto source = MakeDicomSource("inst-D", "class-D", "series-D");

    auto rule = Rule::New();
    const auto relationUID = rule->Connect(seg, source);
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1u), Rule::GetSourceImageRelations(seg).size());

    rule->Disconnect(seg, relationUID);

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Disconnect must remove the relation.",
      static_cast<std::size_t>(0u), Rule::GetSourceImageRelations(seg).size());

    // No property under the relation rule's root key path should mention
    // the SourceSeriesInstanceUID extension after Disconnect. Scanning the
    // full key list avoids hard-coding the instance ID generation strategy.
    const auto rootKey = mitk::PropertyKeyPathToPropertyName(
      mitk::PropertyRelationRuleBase::GetRootKeyPath());
    for (const auto& key : seg->GetPropertyKeys())
    {
      const bool isRuleProperty = key.find(rootKey) == 0;
      const bool mentionsSeriesUid = key.find("SourceSeriesInstanceUID") != std::string::npos;
      CPPUNIT_ASSERT_MESSAGE(
        "Disconnect must remove the rule's per-relation "
        "SourceSeriesInstanceUID extension.",
        !(isRuleProperty && mentionsSeriesUid));
    }
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkSegSourceImageRelationRule)
