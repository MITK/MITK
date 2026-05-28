/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkDICOMSegmentationPropertyHelper.h>
#include <mitkIOUtil.h>
#include <mitkImage.h>
#include <mitkLabel.h>
#include <mitkLabelSetImage.h>
#include <mitkProperties.h>
#include <mitkImageWriteAccessor.h>
#include <mitkPropertyList.h>
#include <mitkPropertyNameHelper.h>
#include <mitkSegSourceImageRelationRule.h>
#include <mitkSegTestSourceImageFactory.h>
#include <mitkTemporoSpatialStringProperty.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcitem.h>
#include <dcmtk/ofstd/ofstring.h>

#include <filesystem>
#include <fstream>
#include <string>

// Tests for the property-driven DICOM SEG writer. There is no legacy
// referenceFiles fallback inside the writer; legacy segs are translated
// into rule connections on the multi-label native reader side (covered
// separately by DICOMSegmentationPropertyHelper::
// MigrateLegacyReferenceFilesToRelation, exercised below). Every test
// here builds the segmentation programmatically so no external DICOM
// SEG fixture is required.

namespace
{
  using Helper = mitk::DICOMSegmentationPropertyHelper;
  using Rule = mitk::SegSourceImageRelationRule;

  std::string DICOMKey(unsigned int g, unsigned int e)
  {
    return mitk::GeneratePropertyNameForDICOMTag(g, e);
  }

  // Stamp the strict-mode Validate contract on seg's property list. The
  // shape mirrors what InheritXxxFromSource + Complete would produce when
  // the seg has been derived from a real DICOM source.
  void StampIdentityTags(mitk::MultiLabelSegmentation* seg)
  {
    seg->SetProperty(DICOMKey(0x0008, 0x0060).c_str(),
                     mitk::TemporoSpatialStringProperty::New("SEG"));
    seg->SetProperty(DICOMKey(0x0008, 0x103E).c_str(),
                     mitk::TemporoSpatialStringProperty::New("MITK Writer Test"));
    seg->SetProperty(DICOMKey(0x0070, 0x0084).c_str(),
                     mitk::TemporoSpatialStringProperty::New("MITK"));
    seg->SetProperty(DICOMKey(0x0012, 0x0071).c_str(),
                     mitk::TemporoSpatialStringProperty::New("Session A"));
    seg->SetProperty(DICOMKey(0x0012, 0x0050).c_str(),
                     mitk::TemporoSpatialStringProperty::New("0"));
    seg->SetProperty(DICOMKey(0x0012, 0x0060).c_str(),
                     mitk::TemporoSpatialStringProperty::New("CenterA"));
    seg->SetProperty(DICOMKey(0x0010, 0x0010).c_str(),
                     mitk::TemporoSpatialStringProperty::New("WriterTest^Patient"));
    seg->SetProperty(DICOMKey(0x0010, 0x0020).c_str(),
                     mitk::TemporoSpatialStringProperty::New("PID-001"));
    seg->SetProperty(DICOMKey(0x0020, 0x0010).c_str(),
                     mitk::TemporoSpatialStringProperty::New("Study-001"));
    // DICOM UIDs must be numeric strings ("0123456789." only). Letters
    // would trip DCMTK's VR check on write.
    seg->SetProperty(DICOMKey(0x0020, 0x000D).c_str(),
                     mitk::TemporoSpatialStringProperty::New("1.2.826.0.1.3680043.10.999.1.1"));
    seg->SetProperty(DICOMKey(0x0020, 0x000E).c_str(),
                     mitk::TemporoSpatialStringProperty::New("1.2.826.0.1.3680043.10.999.1.2"));
    seg->SetProperty(DICOMKey(0x0020, 0x0052).c_str(),
                     mitk::TemporoSpatialStringProperty::New("1.2.826.0.1.3680043.10.999.1.3"));
  }

  // Common scaffolding shared by every writer-test seg: 3-slice geometry,
  // a single label with the metadata Validate requires, foreground voxels
  // so dcmqi sees non-empty frames, and optionally the strict-mode
  // identity-tag set. No source-image relation is attached here; callers
  // layer that on top per the case they exercise.
  //
  // withTracking / withSegProperty toggle the per-label DICOM Type 3 / Type 1
  // metadata so individual tests can exercise the writer's absence paths.
  mitk::MultiLabelSegmentation::Pointer BuildBaseSeg(bool stampIdentity = true,
                                                     unsigned char labelValue = 1,
                                                     bool withTracking = true,
                                                     bool withSegProperty = true)
  {
    auto geometryImage = mitk::Image::New();
    unsigned int dim[3] = {4u, 4u, 3u};
    geometryImage->Initialize(mitk::MakeScalarPixelType<mitk::Label::PixelType>(), 3, dim);

    auto seg = mitk::MultiLabelSegmentation::New();
    seg->Initialize(geometryImage);

    auto label = mitk::Label::New();
    label->SetName("L");
    label->SetValue(labelValue);
    label->SetAlgorithmType(mitk::Label::AlgorithmType::MANUAL);
    label->SetAlgorithmName("WriterTest");
    if (withSegProperty)
    {
      label->SetSegmentedPropertyCategory(mitk::DICOMCodeSequence("T-D0050", "SRT", "Tissue"));
      label->SetSegmentedPropertyType(mitk::DICOMCodeSequenceWithModifiers("T-D0050", "SRT", "Tissue"));
    }
    if (withTracking)
    {
      label->SetTrackingID("track-id");
      label->SetTrackingUID("track-uid");
    }
    seg->AddLabel(label, 0, true, true);

    // dcmqi skips empty slices and produces no DICOM SEG if every slice is
    // empty. Stamp a small foreground voxel block so each slice carries
    // the declared label value and the resulting SEG has frames.
    {
      auto groupImage = seg->GetGroupImage(0);
      mitk::ImageWriteAccessor writeAccessor(groupImage);
      auto *pixels = static_cast<mitk::Label::PixelType *>(writeAccessor.GetData());
      const auto dims = groupImage->GetDimensions();
      const auto sliceSize = static_cast<std::size_t>(dims[0]) * dims[1];
      for (unsigned int z = 0; z < dims[2]; ++z)
      {
        pixels[z * sliceSize + 0] = labelValue;
        pixels[z * sliceSize + 1] = labelValue;
      }
    }

    if (stampIdentity)
      StampIdentityTags(seg);

    return seg;
  }

  // Attach a full DICOM-flavoured rule connection to seg with per-slice
  // source SOPInstance + class UIDs. Matches the source-image-relation
  // shape the SEG reader produces, so the writer's
  // property-driven path is exercised end-to-end.
  void AttachDicomSourceRelation(
    mitk::MultiLabelSegmentation* seg,
    const std::string& seriesUID = "1.2.826.0.1.3680043.10.999.2.1")
  {
    auto perSliceInstance = mitk::TemporoSpatialStringProperty::New();
    auto perSliceClass = mitk::TemporoSpatialStringProperty::New();
    for (int sliceIndex = 0; sliceIndex < 3; ++sliceIndex)
    {
      perSliceInstance->SetValue(0, sliceIndex,
                                 std::string("1.2.826.0.1.3680043.10.999.3.1.") + std::to_string(sliceIndex));
      perSliceClass->SetValue(0, sliceIndex, "1.2.840.10008.5.1.4.1.1.2");
    }
    auto provider = mitk::PropertyList::New();
    provider->SetProperty(DICOMKey(0x0008, 0x0018).c_str(), perSliceInstance);
    provider->SetProperty(DICOMKey(0x0008, 0x0016).c_str(), perSliceClass);
    provider->SetProperty(DICOMKey(0x0020, 0x000e).c_str(),
                          mitk::TemporoSpatialStringProperty::New(seriesUID));
    auto rule = Rule::New();
    rule->Connect(seg, provider.GetPointer());
  }

  // Build a single-label seg with identity tags and a DICOM-flavoured
  // rule connection. dcmqi's geometry check matches source IPP to seg
  // slice origin string-for-string, so the rule's three slices map
  // exactly onto the seg's three slices.
  mitk::MultiLabelSegmentation::Pointer BuildSegWithRule(bool stampIdentity = true,
                                                          const std::string& seriesUID = "1.2.826.0.1.3680043.10.999.2.1",
                                                          unsigned char labelValue = 1,
                                                          bool withTracking = true,
                                                          bool withSegProperty = true)
  {
    auto seg = BuildBaseSeg(stampIdentity, labelValue, withTracking, withSegProperty);
    AttachDicomSourceRelation(seg, seriesUID);
    return seg;
  }

  // Attach an ID-layer-only rule connection: SegSourceImageRelationRule::
  // Connect against an Image that has no DICOM identifying tags. The rule
  // mints a relation UID and per-rule bookkeeping but no per-slice
  // SOPInstance/SOPClass properties. The writer therefore sees an empty
  // sourceItems vector even though a relation exists - the path that
  // hits BuildSyntheticSourceItems / strict-mode throw.
  void AttachIDLayerOnlyRelation(mitk::MultiLabelSegmentation* seg)
  {
    auto nonDicomSource = mitk::Image::New();
    unsigned int dim[3] = {4u, 4u, 3u};
    nonDicomSource->Initialize(mitk::MakeScalarPixelType<mitk::Label::PixelType>(), 3, dim);
    Rule::Connect(seg, nonDicomSource);
  }

  // Open a SEG written by the writer and return its SOP Class UID. Empty
  // string on read failure; the caller asserts on the expected value.
  std::string GetWrittenSEGSOPClassUID(const std::string& path)
  {
    DcmFileFormat ff;
    if (ff.loadFile(path.c_str()).bad())
      return {};
    OFString tmp;
    if (ff.getDataset()->findAndGetOFString(DCM_SOPClassUID, tmp).bad())
      return {};
    return tmp.c_str();
  }

  // Find DICOM Code Value "UNKNOWN" placeholders in the SEG's identity
  // tags after a synthetic write. Used to assert that synthesis stamps
  // exactly what the helper claims it does.
  bool SEGCarriesUnknownPatientID(const std::string& path)
  {
    DcmFileFormat ff;
    if (ff.loadFile(path.c_str()).bad())
      return false;
    OFString tmp;
    if (ff.getDataset()->findAndGetOFString(DCM_PatientID, tmp).bad())
      return false;
    return tmp == Helper::UnknownPatientID().c_str();
  }

  // Write the seg via mitk::IOUtil through the configured writer options.
  std::string WriteSegToTempFile(mitk::MultiLabelSegmentation* seg,
                                 const mitk::IFileWriter::Options& options,
                                 const std::string& nameHint)
  {
    const auto tempDir = std::filesystem::temp_directory_path() / "mitkDICOMSegWriterTest";
    std::filesystem::create_directories(tempDir);
    const std::string path = (tempDir / (nameHint + ".dcm")).string();
    mitk::IOUtil::Save(seg, path, options);
    return path;
  }

  // Read a DICOM-tag-keyed string property from the seg's property list.
  // Returns empty when the property is absent.
  std::string ReadStringProp(const mitk::MultiLabelSegmentation* seg,
                             unsigned int group, unsigned int element)
  {
    const auto prop = seg->GetConstProperty(DICOMKey(group, element));
    return prop.IsNotNull() ? prop->GetValueAsString() : std::string();
  }

  // Read a top-level (file-meta + dataset) string tag from a written SEG.
  // Returns empty string when the file or tag is absent.
  std::string ReadTopLevelString(const std::string& segPath, const DcmTagKey& tag)
  {
    DcmFileFormat ff;
    if (ff.loadFile(segPath.c_str()).bad())
      return {};
    OFString tmp;
    if (ff.getDataset()->findAndGetOFString(tag, tmp).bad())
      return {};
    return tmp.c_str();
  }

  // Build a seg derived from a DICOM-flavoured source image. Mirrors the
  // user-facing workflow: New() -> Initialize(sourceImage) populates the
  // seg's property list from the source's DICOM tags. The caller is
  // responsible for adding labels, foreground voxels, and Rule::Connect.
  mitk::MultiLabelSegmentation::Pointer InitializeSegFromSourceImage(
    const mitk::Image* sourceImage,
    unsigned char labelValue = 1)
  {
    auto seg = mitk::MultiLabelSegmentation::New();
    seg->Initialize(sourceImage);

    auto label = mitk::Label::New();
    label->SetName("L");
    label->SetValue(labelValue);
    label->SetAlgorithmType(mitk::Label::AlgorithmType::MANUAL);
    label->SetAlgorithmName("WorkflowTest");
    seg->AddLabel(label, 0, true, true);

    auto groupImage = seg->GetGroupImage(0);
    mitk::ImageWriteAccessor writeAccessor(groupImage);
    auto *pixels = static_cast<mitk::Label::PixelType *>(writeAccessor.GetData());
    const auto dims = groupImage->GetDimensions();
    const auto sliceSize = static_cast<std::size_t>(dims[0]) * dims[1];
    for (unsigned int z = 0; z < dims[2]; ++z)
    {
      pixels[z * sliceSize + 0] = labelValue;
      pixels[z * sliceSize + 1] = labelValue;
    }

    return seg;
  }

  // Snapshot of one SegmentSequence item as read directly via DCMTK. Code-
  // triple fields are empty strings when the surrounding sequence is absent;
  // hasTrackingID / hasTrackingUID separate "absent" from "present but empty".
  struct SegmentItemView
  {
    bool hasTrackingID = false;
    std::string trackingID;
    bool hasTrackingUID = false;
    std::string trackingUID;
    std::string categoryCodeValue, categoryCodeScheme, categoryCodeMeaning;
    std::string typeCodeValue, typeCodeScheme, typeCodeMeaning;
    std::string typeModifierCodeValue, typeModifierCodeScheme, typeModifierCodeMeaning;
  };

  void ReadCodeTriple(DcmItem* parent, const DcmTagKey& seqTag,
                      std::string& outValue, std::string& outScheme, std::string& outMeaning)
  {
    DcmSequenceOfItems* seq = nullptr;
    if (parent->findAndGetSequence(seqTag, seq).bad() || seq == nullptr)
      return;
    DcmItem* item = seq->getItem(0);
    if (item == nullptr)
      return;
    OFString tmp;
    if (item->findAndGetOFString(DCM_CodeValue, tmp).good())
      outValue = tmp.c_str();
    if (item->findAndGetOFString(DCM_CodingSchemeDesignator, tmp).good())
      outScheme = tmp.c_str();
    if (item->findAndGetOFString(DCM_CodeMeaning, tmp).good())
      outMeaning = tmp.c_str();
  }

  // Navigate (0062,0002) SegmentSequence -> item[segmentNumber-1] and read
  // the per-segment code triples + Tracking ID/UID presence. The plan's
  // assertions about DICOM-level absence (Tracking ID/UID omitted) and the
  // unknown-code fallback cannot be made through dcmqi without descending
  // into the file, so this helper does that walk directly.
  SegmentItemView ReadSegmentItem(const std::string& segPath, unsigned int segmentNumber)
  {
    SegmentItemView view;
    DcmFileFormat ff;
    if (ff.loadFile(segPath.c_str()).bad())
      return view;
    auto* dataset = ff.getDataset();
    DcmSequenceOfItems* segmentSeq = nullptr;
    if (dataset->findAndGetSequence(DCM_SegmentSequence, segmentSeq).bad() || segmentSeq == nullptr)
      return view;
    DcmItem* item = segmentSeq->getItem(segmentNumber - 1);
    if (item == nullptr)
      return view;

    OFString tmp;
    if (item->findAndGetOFString(DCM_TrackingID, tmp).good())
    {
      view.hasTrackingID = true;
      view.trackingID = tmp.c_str();
    }
    if (item->findAndGetOFString(DCM_TrackingUID, tmp).good())
    {
      view.hasTrackingUID = true;
      view.trackingUID = tmp.c_str();
    }

    ReadCodeTriple(item, DCM_SegmentedPropertyCategoryCodeSequence,
                   view.categoryCodeValue, view.categoryCodeScheme, view.categoryCodeMeaning);
    ReadCodeTriple(item, DCM_SegmentedPropertyTypeCodeSequence,
                   view.typeCodeValue, view.typeCodeScheme, view.typeCodeMeaning);

    // SegmentedPropertyTypeModifierCodeSequence nests inside the Type code
    // sequence's item per PS3.3 Segment Description Macro, not at the
    // segment-item level.
    DcmSequenceOfItems* typeSeq = nullptr;
    if (item->findAndGetSequence(DCM_SegmentedPropertyTypeCodeSequence, typeSeq).good()
        && typeSeq != nullptr)
    {
      if (DcmItem* typeItem = typeSeq->getItem(0); typeItem != nullptr)
      {
        ReadCodeTriple(typeItem, DCM_SegmentedPropertyTypeModifierCodeSequence,
                       view.typeModifierCodeValue,
                       view.typeModifierCodeScheme,
                       view.typeModifierCodeMeaning);
      }
    }
    return view;
  }

  // Count the items in a written SEG's (0062,0002) SegmentSequence. Used to
  // pin "labelmap encoding produces N+1 segments (Background + N user labels)"
  // vs "binary encoding produces N segments (no auto-Background)".
  unsigned int CountSegmentSequenceItems(const std::string& segPath)
  {
    DcmFileFormat ff;
    if (ff.loadFile(segPath.c_str()).bad())
      return 0u;
    DcmSequenceOfItems* segmentSeq = nullptr;
    if (ff.getDataset()->findAndGetSequence(DCM_SegmentSequence, segmentSeq).bad()
        || segmentSeq == nullptr)
      return 0u;
    return static_cast<unsigned int>(segmentSeq->card());
  }
}

class mitkDICOMSegmentationIOWriterTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkDICOMSegmentationIOWriterTestSuite);
  MITK_TEST(PropertyDrivenWriteSucceedsAndReloads);
  MITK_TEST(StrictModeRefusesIncompleteSegmentation);
  MITK_TEST(StrictModeRefusesSegWithoutSourceRelation);
  MITK_TEST(StrictModeRefusesIDLayerOnlyRelation);
  MITK_TEST(SyntheticModeFillsMissingIdentityAndWrites);
  MITK_TEST(SyntheticModeMintsStubSourceItemsWhenNoRelation);
  MITK_TEST(SyntheticModeMintsStubSourceItemsForIDLayerOnlyRelation);
  MITK_TEST(SyntheticWriteProducesVRValidOutput);
  MITK_TEST(DefaultEncodingProducesLegacySOPClass);
  MITK_TEST(LabelmapEncodingProducesSup243SOPClass);
  MITK_TEST(LabelmapRoundTripStripsAutoAddedBackgroundSegment);
  MITK_TEST(BinaryEncodingProducesLegacySOPClass);
  MITK_TEST(MultiSourceSegRoundTripPreservesAtLeastOneRelation);
  MITK_TEST(MigrateLegacyReferenceFilesIsNoOpWithoutProperty);
  MITK_TEST(MigrateLegacyReferenceFilesSkipsWhenRuleAlreadyPresent);
  MITK_TEST(WriteSucceedsWithoutTrackingFields);
  MITK_TEST(WriteSucceedsWithoutSegPropertyCategoryAndType);
  MITK_TEST(StrictWriteSucceedsForInitializedFromDICOMSource);
  MITK_TEST(SyntheticWriteSucceedsForInitializedFromDICOMSource);
  MITK_TEST(StrictReSaveAfterDICOMSegRoundTripPreservesIdentity);
  CPPUNIT_TEST_SUITE_END();

public:
  void PropertyDrivenWriteSucceedsAndReloads()
  {
    auto seg = BuildSegWithRule();
    CPPUNIT_ASSERT_MESSAGE("Strict-mode contract is complete for the synthetic input",
                           Helper::Validate(seg).empty());

    mitk::IFileWriter::Options options;
    const auto path = WriteSegToTempFile(seg, options, "property-driven");

    const auto loaded = mitk::IOUtil::Load(path);
    CPPUNIT_ASSERT_MESSAGE("Round-trip load produced exactly one BaseData",
                           loaded.size() == 1);
    const auto loadedSeg = dynamic_cast<mitk::MultiLabelSegmentation*>(loaded[0].GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Round-trip load produced a MultiLabelSegmentation",
                           loadedSeg != nullptr);
  }

  void StrictModeRefusesIncompleteSegmentation()
  {
    auto seg = BuildSegWithRule(/*stampIdentity=*/false);
    CPPUNIT_ASSERT_MESSAGE("Test precondition: incomplete seg fails Validate",
                           !Helper::Validate(seg).empty());

    mitk::IFileWriter::Options options;  // strict by default
    CPPUNIT_ASSERT_THROW_MESSAGE("Strict mode must refuse an incomplete seg",
                                 WriteSegToTempFile(seg, options, "strict-fail"),
                                 mitk::Exception);
  }

  // Validate-passing seg without any rule connection: the writer's
  // empty-sourceItems check (separate from Validate, since the SEG IOD
  // makes the source-image relation type 1C) must still throw in strict
  // mode rather than crash dcmqi on an empty dcmDatasets vector.
  void StrictModeRefusesSegWithoutSourceRelation()
  {
    auto seg = BuildBaseSeg(/*stampIdentity=*/true);
    CPPUNIT_ASSERT_MESSAGE("Precondition: Validate passes on this fixture (rule absence is type 1C)",
                           Helper::Validate(seg).empty());
    CPPUNIT_ASSERT_MESSAGE("Precondition: seg has no rule connection",
                           Rule::GetSourceImageRelations(seg).empty());

    mitk::IFileWriter::Options options;  // strict by default
    CPPUNIT_ASSERT_THROW_MESSAGE("Strict mode must refuse a seg without source-image relation",
                                 WriteSegToTempFile(seg, options, "strict-no-relation"),
                                 mitk::Exception);
  }

  // ID-layer-only relation (Connect against a non-DICOM source): a
  // relation exists on the seg but carries no per-slice SOPInstance /
  // SOPClass properties. The writer's BuildSourceItemsFromProperties
  // skips such relations, leaving sourceItems empty. Strict mode must
  // therefore reject this case the same way it rejects "no relation at
  // all" - the user has to either Connect a real DICOM source or opt
  // into synthetic mode.
  void StrictModeRefusesIDLayerOnlyRelation()
  {
    auto seg = BuildBaseSeg(/*stampIdentity=*/true);
    AttachIDLayerOnlyRelation(seg);
    const auto relations = Rule::GetSourceImageRelations(seg);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Precondition: exactly one rule connection on seg",
                                 std::size_t{1}, relations.size());
    CPPUNIT_ASSERT_MESSAGE("Precondition: relation is ID-layer-only (no per-slice instance UIDs)",
                           relations.front().instanceUIDsPerSlice.IsNull());

    mitk::IFileWriter::Options options;  // strict by default
    CPPUNIT_ASSERT_THROW_MESSAGE(
      "Strict mode must refuse an ID-layer-only relation the same as a relation-less seg",
      WriteSegToTempFile(seg, options, "strict-id-layer"),
      mitk::Exception);
  }

  void SyntheticModeFillsMissingIdentityAndWrites()
  {
    auto seg = BuildSegWithRule(/*stampIdentity=*/false);

    mitk::IFileWriter::Options options;
    options["Strict / synthetic mode"] = std::string("synthetic");
    const auto path = WriteSegToTempFile(seg, options, "synthetic");

    CPPUNIT_ASSERT_MESSAGE("Synthetic-mode write must stamp the UNKNOWN PatientID placeholder",
                           SEGCarriesUnknownPatientID(path));
  }

  // Synthetic-mode safety net for the no-relation case: the writer
  // mints a per-slice stub via BuildSyntheticSourceItems instead of
  // throwing. The stub stamps Secondary Capture Image Storage as its
  // SOPClass; after the SEG reader's PopulateSourceImageRelations runs
  // on reload, the reloaded relation's classUIDsPerSlice carries that
  // value. The series UID is freshly minted by MintSyntheticUID, so
  // we only check it is present and non-empty.
  void SyntheticModeMintsStubSourceItemsWhenNoRelation()
  {
    auto seg = BuildBaseSeg(/*stampIdentity=*/true);
    CPPUNIT_ASSERT_MESSAGE("Precondition: seg starts with no rule connection",
                           Rule::GetSourceImageRelations(seg).empty());

    mitk::IFileWriter::Options options;
    options["Strict / synthetic mode"] = std::string("synthetic");
    const auto path = WriteSegToTempFile(seg, options, "synthetic-stub");

    const auto loaded = mitk::IOUtil::Load(path);
    CPPUNIT_ASSERT_MESSAGE("Synthetic-mode write of relation-less seg reloads as exactly one BaseData",
                           loaded.size() == 1);
    auto* loadedSeg = dynamic_cast<mitk::MultiLabelSegmentation*>(loaded[0].GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Reloaded data is a MultiLabelSegmentation", loadedSeg != nullptr);

    const auto loadedRelations = Rule::GetSourceImageRelations(loadedSeg);
    CPPUNIT_ASSERT_MESSAGE(
      "Reloaded seg carries a relation reconstructed from the synthetic stub source items",
      !loadedRelations.empty());
    const auto& relation = loadedRelations.front();
    CPPUNIT_ASSERT_MESSAGE("Synthetic relation carries a non-empty SeriesInstanceUID",
                           !relation.sourceSeriesInstanceUID.empty());
    CPPUNIT_ASSERT_MESSAGE("Synthetic relation has per-slice class UID property",
                           relation.classUIDsPerSlice.IsNotNull());
    const auto availSlices = relation.classUIDsPerSlice->GetAvailableSlices(0);
    CPPUNIT_ASSERT_MESSAGE("Synthetic relation covers at least one slice",
                           !availSlices.empty());
    const std::string secondaryCaptureSOP = "1.2.840.10008.5.1.4.1.1.7";
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Synthetic source items carry the Secondary Capture Image Storage SOPClass",
      secondaryCaptureSOP,
      relation.classUIDsPerSlice->GetValue(0, availSlices.front()));
  }

  // ID-layer-only counterpart of the case above: the seg has a relation,
  // but the rule's per-slice SOPInstance / SOPClass properties are null
  // because the source carried no DICOM identifying tags. The writer
  // sees an empty sourceItems vector and falls into the synthetic stub
  // path - same outcome as if no relation existed at all.
  void SyntheticModeMintsStubSourceItemsForIDLayerOnlyRelation()
  {
    auto seg = BuildBaseSeg(/*stampIdentity=*/true);
    AttachIDLayerOnlyRelation(seg);

    mitk::IFileWriter::Options options;
    options["Strict / synthetic mode"] = std::string("synthetic");
    const auto path = WriteSegToTempFile(seg, options, "synthetic-id-layer");

    const auto loaded = mitk::IOUtil::Load(path);
    CPPUNIT_ASSERT_MESSAGE("Synthetic-mode write reloads as exactly one BaseData",
                           loaded.size() == 1);
    auto* loadedSeg = dynamic_cast<mitk::MultiLabelSegmentation*>(loaded[0].GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Reloaded data is a MultiLabelSegmentation", loadedSeg != nullptr);

    const auto loadedRelations = Rule::GetSourceImageRelations(loadedSeg);
    CPPUNIT_ASSERT_MESSAGE(
      "Reloaded seg carries a relation reconstructed from the synthetic stub",
      !loadedRelations.empty());
    CPPUNIT_ASSERT_MESSAGE(
      "Synthetic relation populated per-slice class UIDs after the round trip",
      loadedRelations.front().classUIDsPerSlice.IsNotNull());
    const auto availSlices = loadedRelations.front().classUIDsPerSlice->GetAvailableSlices(0);
    CPPUNIT_ASSERT_MESSAGE("Synthetic relation covers at least one slice",
                           !availSlices.empty());
    const std::string secondaryCaptureSOP = "1.2.840.10008.5.1.4.1.1.7";
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Synthetic source items carry the Secondary Capture Image Storage SOPClass",
      secondaryCaptureSOP,
      loadedRelations.front().classUIDsPerSlice->GetValue(0, availSlices.front()));
  }

  void SyntheticWriteProducesVRValidOutput()
  {
    // dcmqi's doDicomValueChecks defaults to true; if any synthesised
    // value violated its VR, the writer would throw instead of producing
    // a file. This guards against future changes to the placeholder
    // strings or UID minting that would silently break the
    // "synthesis always produces VR-valid values" invariant.
    auto seg = BuildSegWithRule(/*stampIdentity=*/false);

    mitk::IFileWriter::Options options;
    options["Strict / synthetic mode"] = std::string("synthetic");
    CPPUNIT_ASSERT_NO_THROW_MESSAGE(
      "Synthetic mode must produce DICOM-VR-valid output under dcmqi's default value checks",
      WriteSegToTempFile(seg, options, "vrcheck"));
  }

  void DefaultEncodingProducesLegacySOPClass()
  {
    auto seg = BuildSegWithRule();

    mitk::IFileWriter::Options options;  // no encoding option set
    const auto path = WriteSegToTempFile(seg, options, "default");

    // 1.2.840.10008.5.1.4.1.1.66.4 = Segmentation Storage (legacy binary).
    // Binary is the writer's default because the Sup 243 labelmap SOP class
    // is too recent for much of the installed base of DICOM tooling.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Default encoding must emit the legacy binary SEG SOP class",
                                 std::string("1.2.840.10008.5.1.4.1.1.66.4"),
                                 GetWrittenSEGSOPClassUID(path));
  }

  void LabelmapEncodingProducesSup243SOPClass()
  {
    auto seg = BuildSegWithRule();

    mitk::IFileWriter::Options options;
    options["Segmentation encoding"] = std::string("labelmap");
    const auto path = WriteSegToTempFile(seg, options, "labelmap");

    // 1.2.840.10008.5.1.4.1.1.66.7 = Label Map Segmentation Storage (Sup 243)
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Explicit labelmap opt-in must emit the Sup 243 SOP class",
                                 std::string("1.2.840.10008.5.1.4.1.1.66.7"),
                                 GetWrittenSEGSOPClassUID(path));
  }

  // Sup 243 makes the pixel value the segment number, so every pixel value
  // present in the labelmap image must map to a declared SegmentSequence
  // entry. dcmqi enforces this by auto-inserting a Background segment
  // (number 0, DCM code 125040) when the image contains any zero pixels;
  // MITK undoes the asymmetry on read because pixel value 0 is MITK's
  // UNLABELED sentinel and a phantom "Background" label would pollute the
  // reloaded seg. This test pins both halves of the contract so a future
  // dcmqi or reader change cannot silently break either side.
  void LabelmapRoundTripStripsAutoAddedBackgroundSegment()
  {
    auto seg = BuildSegWithRule();

    mitk::IFileWriter::Options options;
    options["Segmentation encoding"] = std::string("labelmap");
    const auto path = WriteSegToTempFile(seg, options, "labelmap-bg-roundtrip");

    // Write side: dcmqi must add Background as SegmentSequence item 1.
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Labelmap SEG must carry the auto-added Background segment plus the user's one label",
      2u, CountSegmentSequenceItems(path));
    const auto bgItem = ReadSegmentItem(path, 1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Auto-added Background segment must carry DCM code value 125040",
                                 std::string("125040"), bgItem.categoryCodeValue);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Auto-added Background segment must use the DCM coding scheme",
                                 std::string("DCM"), bgItem.categoryCodeScheme);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Auto-added Background segment must be named 'Background'",
                                 std::string("Background"), bgItem.categoryCodeMeaning);

    // Read side: MITK must strip the Background segment.
    const auto loaded = mitk::IOUtil::Load(path);
    CPPUNIT_ASSERT_MESSAGE("Labelmap SEG must reload as exactly one BaseData",
                           loaded.size() == 1);
    auto* loadedSeg = dynamic_cast<mitk::MultiLabelSegmentation*>(loaded[0].GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Labelmap SEG must reload as a MultiLabelSegmentation",
                           loadedSeg != nullptr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Reloaded seg must carry only the user's labels (Background dropped on read)",
      1u, loadedSeg->GetTotalNumberOfLabels());
    for (const auto labelValue : loadedSeg->GetAllLabelValues())
    {
      CPPUNIT_ASSERT_MESSAGE(
        "No reloaded label may carry the UNLABELED_VALUE (0)",
        labelValue != mitk::MultiLabelSegmentation::UNLABELED_VALUE);
      const auto label = loadedSeg->GetLabel(labelValue);
      CPPUNIT_ASSERT_MESSAGE("Reloaded label must exist for its value", label.IsNotNull());
      CPPUNIT_ASSERT_MESSAGE(
        "No reloaded label may be named 'Background'",
        std::string(label->GetName()) != "Background");
    }
  }

  void BinaryEncodingProducesLegacySOPClass()
  {
    auto seg = BuildSegWithRule();

    mitk::IFileWriter::Options options;
    options["Segmentation encoding"] = std::string("binary");
    const auto path = WriteSegToTempFile(seg, options, "binary");

    // 1.2.840.10008.5.1.4.1.1.66.4 = Segmentation Storage (legacy binary)
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Binary opt-out must emit the legacy SOP class",
                                 std::string("1.2.840.10008.5.1.4.1.1.66.4"),
                                 GetWrittenSEGSOPClassUID(path));
  }

  void MultiSourceSegRoundTripPreservesAtLeastOneRelation()
  {
    auto seg = BuildSegWithRule(true, "1.2.826.0.1.3680043.10.999.4.1");
    // Second source series on the same seg
    auto perSliceInstance = mitk::TemporoSpatialStringProperty::New();
    auto perSliceClass = mitk::TemporoSpatialStringProperty::New();
    for (int s = 0; s < 3; ++s)
    {
      perSliceInstance->SetValue(0, s, std::string("1.2.826.0.1.3680043.10.999.4.2.") + std::to_string(s));
      perSliceClass->SetValue(0, s, "1.2.840.10008.5.1.4.1.1.2");
    }
    auto providerB = mitk::PropertyList::New();
    providerB->SetProperty(DICOMKey(0x0008, 0x0018).c_str(), perSliceInstance);
    providerB->SetProperty(DICOMKey(0x0008, 0x0016).c_str(), perSliceClass);
    providerB->SetProperty(DICOMKey(0x0020, 0x000e).c_str(),
                           mitk::TemporoSpatialStringProperty::New("1.2.826.0.1.3680043.10.999.4.3"));
    auto rule = Rule::New();
    rule->Connect(seg, providerB.GetPointer());

    mitk::IFileWriter::Options options;
    const auto path = WriteSegToTempFile(seg, options, "multisource");
    const auto loaded = mitk::IOUtil::Load(path);
    CPPUNIT_ASSERT_MESSAGE("Multi-source seg reloads as exactly one BaseData",
                           loaded.size() == 1);

    auto* loadedSeg = dynamic_cast<mitk::MultiLabelSegmentation*>(loaded[0].GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Multi-source seg reloads as a MultiLabelSegmentation",
                           loadedSeg != nullptr);
    const auto relations = Rule::GetSourceImageRelations(loadedSeg);
    // dcmqi's current implementation lumps every source DcmItem under
    // dcmDatasets[0]'s SeriesInstanceUID, so the re-read seg carries one
    // relation. The assertion is therefore "at least one" rather than
    // "exactly two": when dcmqi gains true multi-series support this
    // test will exercise the same path and the assertion still holds.
    CPPUNIT_ASSERT_MESSAGE("Multi-source seg carries at least one rule connection after round-trip",
                           !relations.empty());

    // Sharpen the smoke check: the one relation that survives must carry
    // a non-empty source series UID and a non-empty per-slice instance
    // UID property. A serializer that dropped the per-slice property and
    // left only the rule's bookkeeping behind would still pass the
    // "!relations.empty()" assertion above but fail this one — which is
    // the regression we actually care about for the round-trip story.
    const auto& relation = relations.front();
    CPPUNIT_ASSERT_MESSAGE("Round-tripped relation must carry a non-empty source series UID",
                           !relation.sourceSeriesInstanceUID.empty());
    CPPUNIT_ASSERT_MESSAGE("Round-tripped relation must carry a non-null per-slice instance UID property",
                           relation.instanceUIDsPerSlice.IsNotNull());
    const auto slicesAfter = relation.instanceUIDsPerSlice->GetAvailableSlices(0);
    CPPUNIT_ASSERT_MESSAGE("Round-tripped relation must carry per-slice content (not collapsed to zero entries)",
                           !slicesAfter.empty());
  }

  void MigrateLegacyReferenceFilesIsNoOpWithoutProperty()
  {
    auto seg = BuildSegWithRule();
    // Snapshot rule count before
    const auto beforeCount = Rule::GetSourceImageRelations(seg).size();
    const auto added = Helper::MigrateLegacyReferenceFilesToRelation(seg);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Migration on a seg without referenceFiles must add nothing",
                                 std::size_t(0), added);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Existing rule connections survive the no-op migration",
                                 beforeCount, Rule::GetSourceImageRelations(seg).size());
  }

  void MigrateLegacyReferenceFilesSkipsWhenRuleAlreadyPresent()
  {
    auto seg = BuildSegWithRule();
    // Add a referenceFiles property pointing at a definitely-missing file:
    // the migration must early-out because the seg already has a rule
    // connection, not attempt to open the file.
    mitk::StringLookupTable lut;
    lut.SetTableValue(0, "/nonexistent/path/to/source.dcm");
    seg->SetProperty("referenceFiles", mitk::StringLookupTableProperty::New(lut));

    const auto added = Helper::MigrateLegacyReferenceFilesToRelation(seg);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Migration must early-out when a rule connection already exists",
      std::size_t(0), added);
    CPPUNIT_ASSERT_MESSAGE(
      "Existing referenceFiles property survives the early-out (no destructive cleanup on skip)",
      seg->GetProperty("referenceFiles").IsNotNull());
  }

  // Tracking ID/UID are DICOM Type 3. A label with no tracking properties
  // must write successfully in strict mode and the resulting SEG must not
  // carry (0062,0020)/(0062,0021) for that segment; reloading the SEG must
  // leave HasTrackingID/UID false on the label.
  //
  // Default encoding is binary Segmentation Storage; SegmentSequence item 1
  // is the user's label "L" (no auto-inserted Background segment, unlike
  // dcmqi's labelmap path).
  void WriteSucceedsWithoutTrackingFields()
  {
    auto seg = BuildSegWithRule(/*stampIdentity=*/true,
                                "1.2.826.0.1.3680043.10.999.5.1",
                                /*labelValue=*/1,
                                /*withTracking=*/false,
                                /*withSegProperty=*/true);
    CPPUNIT_ASSERT_MESSAGE("Strict-mode Validate accepts a seg without per-label tracking",
                           Helper::Validate(seg).empty());

    mitk::IFileWriter::Options options;
    const auto path = WriteSegToTempFile(seg, options, "no-tracking");

    const auto view = ReadSegmentItem(path, 1);
    CPPUNIT_ASSERT_MESSAGE("Written SEG must omit (0062,0020) when source label had no tracking",
                           !view.hasTrackingID);
    CPPUNIT_ASSERT_MESSAGE("Written SEG must omit (0062,0021) when source label had no tracking",
                           !view.hasTrackingUID);

    const auto loaded = mitk::IOUtil::Load(path);
    CPPUNIT_ASSERT_MESSAGE("Round-trip load produced exactly one BaseData",
                           loaded.size() == 1);
    const auto* loadedSeg = dynamic_cast<mitk::MultiLabelSegmentation*>(loaded[0].GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Round-trip load produced a MultiLabelSegmentation",
                           loadedSeg != nullptr);
    const auto loadedLabels = loadedSeg->GetConstLabelsByValue(loadedSeg->GetLabelValuesByGroup(0));
    CPPUNIT_ASSERT_MESSAGE("Reloaded seg has one label", loadedLabels.size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Reloaded label reports HasTrackingID() == false",
                           !loadedLabels.front()->HasTrackingID());
    CPPUNIT_ASSERT_MESSAGE("Reloaded label reports HasTrackingUID() == false",
                           !loadedLabels.front()->HasTrackingUID());
  }

  // Primary user-reported workflow: load a DICOM source image, build a
  // seg via Initialize(sourceImage) (which routes through
  // DICOMQIPropertyHelper::DeriveDICOMSourceProperties), Connect a source
  // relation, save in strict mode. Asserts the IOD-mandated identity
  // tags survive into the written file.
  //
  // Note on SeriesInstanceUID: the constructor mints a stable seg-own
  // SeriesInstanceUID for in-memory use (lets strict-mode Validate
  // succeed). However, dcmqi's segmentation writer mints its own
  // SeriesInstanceUID for the output file - there is no dcmqi API to
  // override it. So the on-disk SeriesInstanceUID does NOT match the
  // seg's in-memory property; this is an upstream limitation, not a
  // MITK-side workaround target.
  void StrictWriteSucceedsForInitializedFromDICOMSource()
  {
    const std::string sourceSeriesUID = "1.2.826.0.1.3680043.10.999.6.2";
    const std::string sourceStudyUID = "1.2.826.0.1.3680043.10.999.6.1";
    const std::string sourceForUID = "1.2.826.0.1.3680043.10.999.6.3";
    auto source = mitk::test::BuildSourceImageWithDICOMIdentity(
      "Wf^Patient", "Wf-PID", "Wf-Study",
      sourceStudyUID, sourceSeriesUID, sourceForUID);

    auto seg = InitializeSegFromSourceImage(source);
    Rule::Connect(seg, source.GetPointer());

    CPPUNIT_ASSERT_MESSAGE("Validate must report nothing missing for a seg derived from a DICOM source",
                           Helper::Validate(seg).empty());

    const auto segSeriesUID = ReadStringProp(seg, 0x0020, 0x000E);
    CPPUNIT_ASSERT_MESSAGE("Seg's own SeriesInstanceUID must be minted at construction",
                           !segSeriesUID.empty());
    CPPUNIT_ASSERT_MESSAGE("Seg's SeriesInstanceUID must differ from source's series UID",
                           segSeriesUID != sourceSeriesUID);

    mitk::IFileWriter::Options options;
    const auto path = WriteSegToTempFile(seg, options, "wf-strict");

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Source's PatientID survives into the written SEG",
                                 std::string("Wf-PID"), ReadTopLevelString(path, DCM_PatientID));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Source's StudyInstanceUID survives into the written SEG",
                                 sourceStudyUID, ReadTopLevelString(path, DCM_StudyInstanceUID));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Source's StudyID survives into the written SEG",
                                 std::string("Wf-Study"), ReadTopLevelString(path, DCM_StudyID));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Source's FrameOfReferenceUID survives into the written SEG",
                                 sourceForUID, ReadTopLevelString(path, DCM_FrameOfReferenceUID));

    const auto writtenSeriesUID = ReadTopLevelString(path, DCM_SeriesInstanceUID);
    CPPUNIT_ASSERT_MESSAGE("Written SEG SeriesInstanceUID is non-empty", !writtenSeriesUID.empty());
    CPPUNIT_ASSERT_MESSAGE("Written SEG SeriesInstanceUID differs from source's series UID "
                           "(SEG is its own series, not a re-issue of the source's)",
                           writtenSeriesUID != sourceSeriesUID);
  }

  // Synthetic-mode counterpart of the workflow case above. Even with a
  // real DICOM source attached via Initialize, the user can opt into
  // synthetic mode. The write must succeed and the written SEG must
  // carry an own (non-source) SeriesInstanceUID.
  void SyntheticWriteSucceedsForInitializedFromDICOMSource()
  {
    auto source = mitk::test::BuildSourceImageWithDICOMIdentity();
    auto seg = InitializeSegFromSourceImage(source);
    Rule::Connect(seg, source.GetPointer());

    mitk::IFileWriter::Options options;
    options["Strict / synthetic mode"] = std::string("synthetic");
    const auto path = WriteSegToTempFile(seg, options, "wf-synth");

    const auto writtenSeriesUID = ReadTopLevelString(path, DCM_SeriesInstanceUID);
    CPPUNIT_ASSERT_MESSAGE("Synthetic-mode write produces a non-empty SeriesInstanceUID",
                           !writtenSeriesUID.empty());
    const auto sourceSeriesUID = source->GetConstProperty(DICOMKey(0x0020, 0x000e))->GetValueAsString();
    CPPUNIT_ASSERT_MESSAGE("Synthetic-mode write SeriesInstanceUID differs from source's series UID",
                           writtenSeriesUID != sourceSeriesUID);
  }

  // Round-trip via the DICOM SEG reader: write a workflow seg in strict
  // mode, load it back, save again in strict mode. The reloaded seg
  // must carry every IOD-mandated identity tag - in particular
  // FrameOfReferenceUID, which only lands on the loaded seg's property
  // list when (0020,0052) is in the default DICOMTagsOfInterest.
  // SeriesInstanceUID stability across persist -> reload -> re-persist
  // is NOT asserted here: dcmqi mints a fresh UID for the output SEG on
  // every write and has no API for MITK to override it.
  void StrictReSaveAfterDICOMSegRoundTripPreservesIdentity()
  {
    auto source = mitk::test::BuildSourceImageWithDICOMIdentity();
    auto seg = InitializeSegFromSourceImage(source);
    Rule::Connect(seg, source.GetPointer());

    mitk::IFileWriter::Options options;
    const auto firstPath = WriteSegToTempFile(seg, options, "wf-roundtrip-1");

    const auto loaded = mitk::IOUtil::Load(firstPath);
    CPPUNIT_ASSERT_MESSAGE("Round-trip load produced exactly one BaseData", loaded.size() == 1);
    auto* reloadedSeg = dynamic_cast<mitk::MultiLabelSegmentation*>(loaded[0].GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Reloaded data is a MultiLabelSegmentation", reloadedSeg != nullptr);

    const auto reloadedMissing = Helper::Validate(reloadedSeg);
    if (!reloadedMissing.empty())
    {
      std::ostringstream diag;
      diag << "Reloaded seg must pass strict-mode Validate; missing items:";
      for (const auto& item : reloadedMissing)
        diag << "\n  - " << item.description << " (scope=" << static_cast<int>(item.scope)
             << ", id=" << item.identifier << ")";
      CPPUNIT_FAIL(diag.str());
    }

    CPPUNIT_ASSERT_NO_THROW_MESSAGE(
      "Re-saving the reloaded seg in strict mode must succeed",
      WriteSegToTempFile(reloadedSeg, options, "wf-roundtrip-2"));
  }

  // Segmented Property Category/Type are DICOM Type 1 but the writer's
  // unknown-code fallback supplies an honest value when the label has none.
  // Strict mode must accept the absence; the written SEG must carry the
  // unknown SCT triple verbatim. Regression gate against re-introducing the
  // historical M-01000 / M-03000 SRT defaults.
  void WriteSucceedsWithoutSegPropertyCategoryAndType()
  {
    auto seg = BuildSegWithRule(/*stampIdentity=*/true,
                                "1.2.826.0.1.3680043.10.999.5.2",
                                /*labelValue=*/1,
                                /*withTracking=*/true,
                                /*withSegProperty=*/false);
    CPPUNIT_ASSERT_MESSAGE("Strict-mode Validate accepts a seg without per-label Cat/Type",
                           Helper::Validate(seg).empty());

    mitk::IFileWriter::Options options;
    const auto path = WriteSegToTempFile(seg, options, "no-segproperty");

    // Default encoding is binary; the user's label "L" is segment item 1
    // (binary has no auto-inserted Background segment).
    const auto view = ReadSegmentItem(path, 1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fallback Category code value", std::string("49755003"), view.categoryCodeValue);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fallback Category coding scheme", std::string("SCT"), view.categoryCodeScheme);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fallback Category code meaning",
                                 std::string("Morphologically altered structure"), view.categoryCodeMeaning);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fallback Type code value", std::string("49755003"), view.typeCodeValue);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fallback Type coding scheme", std::string("SCT"), view.typeCodeScheme);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fallback Type code meaning",
                                 std::string("Morphologically altered structure"), view.typeCodeMeaning);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fallback Type modifier code value",
                                 std::string("261665006"), view.typeModifierCodeValue);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fallback Type modifier coding scheme",
                                 std::string("SCT"), view.typeModifierCodeScheme);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fallback Type modifier code meaning",
                                 std::string("Unknown (qualifier value)"), view.typeModifierCodeMeaning);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkDICOMSegmentationIOWriter)
