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
  mitk::MultiLabelSegmentation::Pointer BuildBaseSeg(bool stampIdentity = true,
                                                     unsigned char labelValue = 1)
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
    label->SetSegmentedPropertyCategory(mitk::DICOMCodeSequence("T-D0050", "SRT", "Tissue"));
    label->SetSegmentedPropertyType(mitk::DICOMCodeSequenceWithModifiers("T-D0050", "SRT", "Tissue"));
    label->SetTrackingID("track-id");
    label->SetTrackingUID("track-uid");
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
                                                          unsigned char labelValue = 1)
  {
    auto seg = BuildBaseSeg(stampIdentity, labelValue);
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
  MITK_TEST(LabelmapEncodingProducesSup243SOPClass);
  MITK_TEST(BinaryEncodingProducesLegacySOPClass);
  MITK_TEST(MultiSourceSegRoundTripPreservesAtLeastOneRelation);
  MITK_TEST(MigrateLegacyReferenceFilesIsNoOpWithoutProperty);
  MITK_TEST(MigrateLegacyReferenceFilesSkipsWhenRuleAlreadyPresent);
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

  void LabelmapEncodingProducesSup243SOPClass()
  {
    auto seg = BuildSegWithRule();

    mitk::IFileWriter::Options options;  // labelmap by default
    const auto path = WriteSegToTempFile(seg, options, "labelmap");

    // 1.2.840.10008.5.1.4.1.1.66.7 = Label Map Segmentation Storage (Sup 243)
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Default encoding must emit Sup 243 labelmap SEG",
                                 std::string("1.2.840.10008.5.1.4.1.1.66.7"),
                                 GetWrittenSEGSOPClassUID(path));
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
};

MITK_TEST_SUITE_REGISTRATION(mitkDICOMSegmentationIOWriter)
