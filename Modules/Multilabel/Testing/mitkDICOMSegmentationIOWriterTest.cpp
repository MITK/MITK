/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkDICOMSegFileContentTestHelpers.h>
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
#include <mitkSegIOTestFixtures.h>
#include <mitkSegSourceReferenceTestHelpers.h>
#include <mitkSegTestSourceImageFactory.h>
#include <mitkTemporoSpatialStringProperty.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <dcmqi/ColorUtilities.h>

#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcitem.h>
#include <dcmtk/ofstd/ofstring.h>

#include <itksys/SystemTools.hxx>

#include <cmath>
#include <functional>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

// Tests for the property-driven DICOM SEG writer. There is no legacy
// referenceFiles fallback inside the writer; legacy segs are translated
// into rule connections on the multi-label native reader side (covered
// separately by DICOMSegmentationPropertyHelper::
// MigrateLegacyReferenceFilesToRelation, exercised below). Every test
// here builds the segmentation programmatically so no external DICOM
// SEG fixture is required. Every test that writes through
// WriteSegToTempFile checks each item of the Segment Sequence of every
// file written; the round trips by value are in
// mitkDICOMSegmentationIORoundTripTest.

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
      label->SetTrackingUID("1.2.826.0.1.3680043.10.999.11.1");
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

  // A source-image relation the writer cannot write: the UIDs of slice 1 and the source series are
  // given, those of the other slices are valid. The fragments are what the refusal must name besides
  // the relation.
  struct UnwritableRelation
  {
    std::string name;
    std::string seriesUID;
    std::string instanceUIDOfSlice1;
    std::string classUIDOfSlice1;
    std::vector<std::string> fragments;
  };

  std::vector<UnwritableRelation> UnwritableRelations()
  {
    const std::string series = "1.2.826.0.1.3680043.10.999.9.2";
    const std::string instance = "1.2.826.0.1.3680043.10.999.9.1.1";
    const std::string ctImageStorage = "1.2.840.10008.5.1.4.1.1.2";
    // DICOM UIDs admit no component with a leading zero.
    return {
      { "no-series", "", instance, ctImageStorage, {"(0020,000E)"} },
      { "invalid-series", "1.2.826.0.1.3680043.10.999.09.2", instance, ctImageStorage,
        {"(0020,000E)", "1.2.826.0.1.3680043.10.999.09.2"} },
      { "invalid-instance", series, "1.2.826.0.1.3680043.10.999.9.1.01", ctImageStorage,
        {"(0008,0018)", "1.2.826.0.1.3680043.10.999.9.1.01", "slice 1"} },
      { "invalid-class", series, instance, "CT Image Storage", {"(0008,0016)", "CT Image Storage", "slice 1"} },
      { "no-class", series, instance, "", {"(0008,0016)", "slice 1"} },
    };
  }

  // Attach a relation with per-slice source UIDs and return its UID.
  std::string ConnectRelation(mitk::MultiLabelSegmentation* seg, const UnwritableRelation& relation)
  {
    auto perSliceInstance = mitk::TemporoSpatialStringProperty::New();
    auto perSliceClass = mitk::TemporoSpatialStringProperty::New();
    for (int sliceIndex = 0; sliceIndex < 3; ++sliceIndex)
    {
      perSliceInstance->SetValue(0, sliceIndex, "1.2.826.0.1.3680043.10.999.9.1." + std::to_string(sliceIndex));
      perSliceClass->SetValue(0, sliceIndex, "1.2.840.10008.5.1.4.1.1.2");
    }
    perSliceInstance->SetValue(0, 1, relation.instanceUIDOfSlice1);
    perSliceClass->SetValue(0, 1, relation.classUIDOfSlice1);
    return Rule::New()->Connect(seg, perSliceInstance, perSliceClass, relation.seriesUID);
  }

  // A value of the segmentation's patient, study or frame of reference that DCMTK refuses for its VR
  // without Specific Character Set: characters outside the default repertoire or outside the syntax
  // of the VR, or a UI value over 64 characters or a CS or TM value over 16. DCMTK checks no other
  // lengths. An empty Frame of Reference UID DCMTK refuses as an empty Type 1 value. The writer
  // hands these values to dcmqi on the first source item. The fragments are what the refusal must
  // name.
  struct UnwritableIdentityValue
  {
    std::string name;
    unsigned int group;
    unsigned int element;
    std::string value;
    std::vector<std::string> fragments;
  };

  std::vector<UnwritableIdentityValue> UnwritableIdentityValues()
  {
    return {
      { "patient-name", 0x0010, 0x0010, "M\xC3\xBCller^Hans", {"(0010,0010)"} },
      { "patient-sex", 0x0010, 0x0040, "male", {"(0010,0040)", "male"} },
      { "patient-birth-date", 0x0010, 0x0030, "1980-01-01", {"(0010,0030)", "1980-01-01"} },
      { "study-description", 0x0008, 0x1030, "Kn\xC3\xB6" "chel", {"(0008,1030)"} },
      { "study-uid", 0x0020, 0x000D, "1.2.826.0.1.3680043.10.999.01.1", {"(0020,000D)", "1.2.826.0.1.3680043.10.999.01.1"} },
      { "frame-of-reference-uid", 0x0020, 0x0052, "1.2.826.0.1.3680043.10.999.01.3",
        {"(0020,0052)", "1.2.826.0.1.3680043.10.999.01.3"} },
      { "frame-of-reference-uid-empty", 0x0020, 0x0052, "", {"(0020,0052)", "empty"} },
      // 66 characters; a UID has at most 64.
      { "study-uid-too-long", 0x0020, 0x000D, "1.2.826.0.1.3680043.10.999.1.1234567890.1234567890.1234567890.1234",
        {"(0020,000D)", "1.2.826.0.1.3680043.10.999.1.1234567890.1234567890.1234567890.1234"} },
      // 17 characters; CS and TM have at most 16.
      { "patient-sex-too-long", 0x0010, 0x0040, "ABCDEFGHIJKLMNOPQ", {"(0010,0040)", "ABCDEFGHIJKLMNOPQ"} },
      { "study-time-too-long", 0x0008, 0x0030, "120000.1234567890", {"(0008,0030)", "120000.1234567890"} },
    };
  }

  void SetIdentityValue(mitk::MultiLabelSegmentation* seg, const UnwritableIdentityValue& value)
  {
    seg->SetProperty(DICOMKey(value.group, value.element).c_str(), mitk::TemporoSpatialStringProperty::New(value.value));
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

  // Open a SEG written by the writer and return its NumberOfFrames
  // (0028,0008), or -1 on read failure / missing tag so the caller can
  // assert on the exact expected count.
  long GetWrittenSEGNumberOfFrames(const std::string& path)
  {
    DcmFileFormat ff;
    if (ff.loadFile(path.c_str()).bad())
      return -1;
    long frames = 0;
    if (ff.getDataset()->findAndGetLongInt(DCM_NumberOfFrames, frames).bad())
      return -1;
    return frames;
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
    return mitk::test::ReadTag(segPath, tag);
  }

  // The per-slice source SOP Instance UIDs an image declares. Reading them
  // off the image ties the expectation to the fixture object instead of to a
  // second copy of the UIDs the factory happens to mint.
  std::set<std::string> GetPerSliceSOPInstanceUIDs(const mitk::IPropertyProvider* provider)
  {
    std::set<std::string> result;

    const auto prop = provider->GetConstProperty(DICOMKey(0x0008, 0x0018));
    if (const auto* tsProp = dynamic_cast<const mitk::TemporoSpatialStringProperty*>(prop.GetPointer()))
    {
      for (const auto timeStep : tsProp->GetAvailableTimeSteps())
      {
        for (const auto slice : tsProp->GetAvailableSlices(timeStep))
          result.insert(tsProp->GetValue(timeStep, slice));
      }
    }

    return result;
  }

  // Assert that the SEG at segPath references exactly the source's own
  // per-slice instances. Presence alone would pass on a SEG that referenced
  // the real instances and a minted series besides, and a real relation must
  // suppress synthesis outright - so the reference set may not exceed the
  // source's either.
  void AssertSourceReferencesAreExactly(const std::string& segPath,
                                        const mitk::IPropertyProvider* source,
                                        const std::string& mode)
  {
    const auto expected = GetPerSliceSOPInstanceUIDs(source);
    CPPUNIT_ASSERT_MESSAGE("Test precondition: the source declares per-slice SOP Instance UIDs",
                           !expected.empty());

    const auto diff = mitk::test::DiffSourceReferences(
      expected, mitk::test::ReadReferencedSOPInstanceUIDs(segPath));

    if (!diff.empty())
      CPPUNIT_FAIL(mode + "-mode SEG must reference exactly the source's own instances:" + diff);
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

  // A seg that strict mode writes, with one MANUAL label per entry, inserted in the given order.
  // Label i covers voxel i of every slice, so every label becomes a segment with frames, and the
  // last voxels of a slice stay unlabeled.
  mitk::MultiLabelSegmentation::Pointer BuildSegWithLabels(
    const std::vector<std::pair<mitk::Label::PixelType, std::string>>& labels)
  {
    auto geometryImage = mitk::Image::New();
    unsigned int dim[3] = {4u, 4u, 3u};
    geometryImage->Initialize(mitk::MakeScalarPixelType<mitk::Label::PixelType>(), 3, dim);

    auto seg = mitk::MultiLabelSegmentation::New();
    seg->Initialize(geometryImage);
    for (const auto& [value, name] : labels)
    {
      auto label = mitk::Label::New(value, name);
      label->SetAlgorithmType(mitk::Label::AlgorithmType::MANUAL);
      label->SetSegmentedPropertyCategory(mitk::DICOMCodeSequence("T-D0050", "SRT", "Tissue"));
      label->SetSegmentedPropertyType(mitk::DICOMCodeSequenceWithModifiers("T-D0050", "SRT", "Tissue"));
      seg->AddLabel(label, 0, false, false);
    }

    {
      mitk::ImageWriteAccessor writeAccessor(seg->GetGroupImage(0));
      auto* pixels = static_cast<mitk::Label::PixelType*>(writeAccessor.GetData());
      for (unsigned int z = 0; z < dim[2]; ++z)
      {
        for (std::size_t i = 0; i < labels.size(); ++i)
          pixels[z * dim[0] * dim[1] + i] = labels[i].first;
      }
    }

    StampIdentityTags(seg);
    AttachDicomSourceRelation(seg);
    return seg;
  }

  // Two groups with one label each, values 1 and 2, derived from a DICOM source and connected to it.
  mitk::MultiLabelSegmentation::Pointer BuildTwoGroupSeg()
  {
    auto source = mitk::test::BuildSourceImageWithDICOMIdentity();
    auto seg = InitializeSegFromSourceImage(source);
    const auto g1 = seg->AddGroup();
    auto label2 = mitk::Label::New();
    label2->SetName("L2");
    label2->SetValue(2);
    label2->SetAlgorithmType(mitk::Label::AlgorithmType::MANUAL);
    label2->SetAlgorithmName("WorkflowTest");
    seg->AddLabel(label2, g1, true, true);

    // The writer encodes every slice, and a group without voxels makes dcmqi produce no SEG.
    {
      auto *g1Image = seg->GetGroupImage(g1);
      mitk::ImageWriteAccessor writeAccessor(g1Image);
      auto *pixels = static_cast<mitk::Label::PixelType *>(writeAccessor.GetData());
      const auto dims = g1Image->GetDimensions();
      const auto sliceSize = static_cast<std::size_t>(dims[0]) * dims[1];
      for (unsigned int z = 0; z < dims[2]; ++z)
      {
        pixels[z * sliceSize + 0] = 2;
        pixels[z * sliceSize + 1] = 2;
      }
    }

    Rule::Connect(seg, source.GetPointer());
    return seg;
  }

  bool IsLabelmap(const mitk::IFileWriter::Options& options)
  {
    const auto finding = options.find("Segmentation encoding");
    return finding != options.end() && finding->second.ToString() == "labelmap";
  }

  // The writer writes group 0 to the requested path and inserts "_<group>" before the last dot of
  // the file name for the others, or appends it if the file name has none or consists of digits and
  // dots only.
  std::string GroupFilePath(const std::string& path, mitk::MultiLabelSegmentation::GroupIndexType group)
  {
    if (0 == group)
      return path;
    const auto fileName = itksys::SystemTools::GetFilenameName(path);
    const auto extension = fileName.find_first_not_of("0123456789.") == std::string::npos
                             ? std::string()
                             : itksys::SystemTools::GetFilenameLastExtension(path);
    return path.substr(0, path.size() - extension.size()) + "_" + std::to_string(group) + extension;
  }

  void AssertSegmentSequence(const std::string& path, const mitk::test::DICOMSegSegmentMap& expected)
  {
    const auto differences = mitk::test::CompareSegmentSequences(expected, mitk::test::ReadSegmentSequence(path));
    CPPUNIT_ASSERT_MESSAGE(path + " differs on the wire:" + mitk::test::JoinDifferences(differences), differences.empty());
  }

  void AssertWrittenSegments(const std::string& path,
                             const mitk::MultiLabelSegmentation& seg,
                             mitk::MultiLabelSegmentation::GroupIndexType group,
                             bool labelmap)
  {
    const auto differences = mitk::test::CompareWrittenSegments(path, seg, group, labelmap);
    CPPUNIT_ASSERT_MESSAGE(path + " differs on the wire:" + mitk::test::JoinDifferences(differences), differences.empty());
  }

  bool FileExists(const std::string& path)
  {
    return itksys::SystemTools::FileExists(path, true);
  }

  // IOUtil names a missing writer only in its message; a writer that refuses reads "Exception
  // occurred when writing" instead.
  static void AssertNoWriterFound(const mitk::BaseData* data, const std::string& path)
  {
    try
    {
      mitk::IOUtil::Save(data, mitk::test::DICOMSegMimeTypeName(), path);
    }
    catch (const mitk::Exception& e)
    {
      const std::string message = e.GetDescription();
      CPPUNIT_ASSERT_MESSAGE("No writer is selected: " + message, message.find("No suitable writer") != std::string::npos);
      return;
    }
    CPPUNIT_FAIL("The data was written as DICOM SEG");
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
  MITK_TEST(WriterPreservesEmptySlicesInOutput);
  MITK_TEST(MultiSourceSegRoundTripPreservesAtLeastOneRelation);
  MITK_TEST(MigrateLegacyReferenceFilesIsNoOpWithoutProperty);
  MITK_TEST(MigrateLegacyReferenceFilesSkipsWhenRuleAlreadyPresent);
  MITK_TEST(WriteSucceedsWithoutTrackingFields);
  MITK_TEST(WriteSucceedsWithoutSegPropertyCategoryAndType);
  MITK_TEST(WriteReplacesIncompleteCodesByStandIns);
  MITK_TEST(StrictWriteSucceedsForInitializedFromDICOMSource);
  MITK_TEST(SyntheticWriteSucceedsForInitializedFromDICOMSource);
  MITK_TEST(StrictReSaveAfterDICOMSegRoundTripPreservesIdentity);
  MITK_TEST(MultiGroupWriteFormsOneCoherentSeries);
  MITK_TEST(WriteEmitsContentCreatorAndClinicalTrialTagsWhenSet);
  MITK_TEST(WriteNumbersSegmentsByLabelValueInBinary);
  MITK_TEST(WriteNumbersSegmentsByLabelValueInLabelmap);
  MITK_TEST(WriteAcceptsLabelValue32767);
  MITK_TEST(WriteRefusesLabelValuesAbove32767);
  MITK_TEST(WriteTakesBodyPartExaminedFromProperty);
  MITK_TEST(WriteTakesBodyPartExaminedFromTheSourceImage);
  MITK_TEST(WriteLeavesBodyPartExaminedEmptyWithoutProperty);
  MITK_TEST(WriteTakesSeriesDescriptionFromProperty);
  MITK_TEST(WriteKeepsLabelNamesOf64Characters);
  MITK_TEST(WriteTruncatesLongLabelNames);
  MITK_TEST(WriteTruncatesLongAlgorithmNames);
  MITK_TEST(WriteTruncatesLongDescriptionsOfUnnamedLabels);
  MITK_TEST(WriteRefusesLabelNamesWithBackslash);
  MITK_TEST(WriteRefusesNonASCIILabelNames);
  MITK_TEST(WriteRefusesLabelNamesWithControlCharacters);
  MITK_TEST(WriteRefusesInvalidAlgorithmNames);
  MITK_TEST(WriteIgnoresAlgorithmNamesItDoesNotWrite);
  MITK_TEST(WriteRefusesUnwritableRelations);
  MITK_TEST(WriteRefusesUnwritableRelationBeforeContractCheck);
  MITK_TEST(WriteRefusesUnwritableIdentityValues);
  MITK_TEST(WriteRefusesUnwritableIdentityValueBeforeContractCheck);
  MITK_TEST(WriteTurnsDcmqiFailuresIntoExceptions);
  MITK_TEST(WriteRoundsColours);
  MITK_TEST(StrictModeRefusalsAreStrictModeExceptions);
  MITK_TEST(SyntheticModeRefusalIsPlainException);
  MITK_TEST(WriteKeepsDottedDirectoryOfPathWithoutExtension);
  MITK_TEST(WriteKeepsTheExtensionOfTheRequestedPath);
  MITK_TEST(WriteKeepsDottedStemOfPathWithoutExtension);
  MITK_TEST(WriteUsesLowestExistingAnatomicRegion);
  MITK_TEST(WriteLeavesOutIncompleteAnatomicRegions);
  MITK_TEST(WriteRefuses4D);
  MITK_TEST(WriteRefuses2D);
  MITK_TEST(WriteRefusesDataThatIsNoSegmentation);
  MITK_TEST(WriteRefusesUnwritableLabelOfLaterGroupBeforeAnyFile);
  MITK_TEST(WriteReferencesSourcesWhenPlanesDisagreeWithTransform);
  MITK_TEST(WriteRefusesSourceReferencesThatMapToNoFrame);
  MITK_TEST(LabelmapReloadOrdersLabelsBySegmentNumber);
  MITK_TEST(WriteRefusesInvalidTrackingUID);
  MITK_TEST(WriteRefusesInvalidSegmentDescriptions);
  MITK_TEST(WriteRefusesInvalidTrackingIDs);
  MITK_TEST(WriteKeepsTextWithBackslashAndLineBreaks);
  MITK_TEST(WriteNamesUnnamedLabelsByTheirDescription);
  MITK_TEST(WriteTreatsBlankNamesAndDescriptionsAsNone);
  MITK_TEST(WriteRefusesUnnamedLabelsWhoseSegmentLabelIsNoLOValue);
  MITK_TEST(WriteRefusesUnnamedLabelWithoutVoxelsWhoseDescriptionIsNoLOValue);
  MITK_TEST(WriteRefusesUnknownOptionValues);
  MITK_TEST(WriteRefusesUnwritableLabelBeforeTheContractCheck);
  MITK_TEST(WriteRefusesSegmentationWithoutGroups);
  MITK_TEST(WriteRefusesGroupsWithoutLabelsBeforeAnything);
  CPPUNIT_TEST_SUITE_END();

private:
  std::string m_TempPath;

  std::string GetTempFilePath(const std::string& fileName) const
  {
    return m_TempPath + mitk::IOUtil::GetDirectorySeparator() + fileName;
  }

  // Writes seg through IOUtil, as a user does, and leaves checking the files to the caller.
  std::string WriteSegToTempFileUnchecked(mitk::MultiLabelSegmentation* seg,
                                          const mitk::IFileWriter::Options& options,
                                          const std::string& nameHint) const
  {
    const auto path = this->GetTempFilePath(nameHint + ".dcm");
    mitk::IOUtil::Save(seg, path, options);
    return path;
  }

  // Writes seg through IOUtil, as a user does, and checks every item of the Segment Sequence of
  // every file written, keyed by Segment Number. Synthetic mode stamps seg before it is written,
  // so the check runs against the stamped seg.
  std::string WriteSegToTempFile(mitk::MultiLabelSegmentation* seg,
                                 const mitk::IFileWriter::Options& options,
                                 const std::string& nameHint) const
  {
    const auto path = this->WriteSegToTempFileUnchecked(seg, options, nameHint);
    for (mitk::MultiLabelSegmentation::GroupIndexType group = 0; group < seg->GetNumberOfGroups(); ++group)
      AssertWrittenSegments(GroupFilePath(path, group), *seg, group, IsLabelmap(options));
    return path;
  }

  // The exception type by which a write refusal tells callers what can make the write succeed.
  enum class Refusal
  {
    Plain,
    StrictMode,
    UnrepresentableValue
  };

  static std::string ToString(Refusal refusal)
  {
    switch (refusal)
    {
      case Refusal::StrictMode:
        return "a DICOMSegStrictModeException";
      case Refusal::UnrepresentableValue:
        return "a DICOMSegUnrepresentableValueException";
      default:
        return "a plain mitk::Exception";
    }
  }

  // Asserts that the writer refuses a clone of seg, called directly and through IOUtil. Called
  // directly, it must throw an exception of the expected type whose message contains every
  // fragment. Returns the message of the direct call.
  std::string AssertWriteRefused(const mitk::MultiLabelSegmentation* seg,
                                 const mitk::IFileWriter::Options& options,
                                 const std::string& nameHint,
                                 Refusal expected,
                                 const std::vector<std::string>& fragments) const
  {
    const auto path = this->GetTempFilePath(nameHint + ".dcm");
    std::string message;
    bool refused = false;
    auto refusal = Refusal::Plain;
    try
    {
      mitk::test::WriteWithWriter(mitk::test::CloneKeepingUID(*seg), path, "DICOM Segmentation", options);
    }
    catch (const mitk::DICOMSegStrictModeException& e)
    {
      refused = true;
      message = e.GetDescription();
      refusal = Refusal::StrictMode;
    }
    catch (const mitk::DICOMSegUnrepresentableValueException& e)
    {
      refused = true;
      message = e.GetDescription();
      refusal = Refusal::UnrepresentableValue;
    }
    catch (const mitk::Exception& e)
    {
      refused = true;
      message = e.GetDescription();
    }
    if (refused)
    {
      CPPUNIT_ASSERT_MESSAGE(nameHint + ": the refusal is " + ToString(refusal) + " instead of " + ToString(expected) +
                               ": " + message,
                             refusal == expected);
    }
    else
    {
      std::string written;
      for (const auto& [number, label] : mitk::test::ReadSegmentSequence(path))
        written += " segment " + std::to_string(number) + " '" + label->GetName() + "';";
      CPPUNIT_FAIL(nameHint + ": the writer wrote instead of refusing:" + written);
    }
    for (const auto& fragment : fragments)
    {
      CPPUNIT_ASSERT_MESSAGE(nameHint + ": the message does not contain '" + fragment + "': " + message,
                             message.find(fragment) != std::string::npos);
    }

    CPPUNIT_ASSERT_THROW_MESSAGE(nameHint + ": IOUtil::Save refuses as well",
                                 mitk::IOUtil::Save(mitk::test::CloneKeepingUID(*seg), path, options),
                                 mitk::Exception);
    return message;
  }

public:
  void setUp() override
  {
    m_TempPath = mitk::IOUtil::CreateTemporaryDirectory("mitkDICOMSegWriterTest_XXXXXX");
  }

  void tearDown() override
  {
    itksys::SystemTools::RemoveADirectory(m_TempPath);
  }

  // The writer hands the labels of a group to dcmqi in ascending value order, which numbers the
  // segments in that order, so the segment number is the label's rank. B=2 is inserted before A=1
  // so that insertion order and value order differ.
  void WriteNumbersSegmentsByLabelValueInBinary()
  {
    auto seg = BuildSegWithLabels({{2, "B"}, {1, "A"}});
    mitk::IFileWriter::Options options;
    options["Segmentation encoding"] = std::string("binary");
    const auto path = this->WriteSegToTempFile(seg, options, "order-binary");

    const auto segments = mitk::test::ReadSegmentSequence(path);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Segment 1 is the label with the lowest value", std::string("A"), segments.at(1)->GetName());

    const auto loaded = mitk::IOUtil::Load(path);
    const auto* loadedSeg = dynamic_cast<const mitk::MultiLabelSegmentation*>(loaded.front().GetPointer());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Label value 1 is reloaded with its own name", std::string("A"), loadedSeg->GetLabel(1)->GetName());
  }

  // Without values 1..N, the labelmap encoding numbers the segments in the order the writer hands
  // them over, too.
  void WriteNumbersSegmentsByLabelValueInLabelmap()
  {
    auto seg = BuildSegWithLabels({{7, "Seven"}, {3, "Three"}});
    mitk::IFileWriter::Options options;
    options["Segmentation encoding"] = std::string("labelmap");
    const auto path = this->WriteSegToTempFile(seg, options, "order-labelmap");

    const auto segments = mitk::test::ReadSegmentSequence(path);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Segment 1 is the label with the lowest value", std::string("Three"), segments.at(1)->GetName());
  }

  // dcmqi takes label values as short, so 32767 is the largest value it can write.
  void WriteAcceptsLabelValue32767()
  {
    auto seg = BuildSegWithLabels({{1, "Low"}, {32767, "Highest"}});
    this->WriteSegToTempFile(seg, {}, "value-32767");
  }

  void WriteRefusesLabelValuesAbove32767()
  {
    for (const mitk::Label::PixelType value : {mitk::Label::PixelType(32768), mitk::Label::PixelType(40000)})
    {
      auto seg = BuildSegWithLabels({{1, "Low"}, {value, "Too high"}});
      const auto message =
        this->AssertWriteRefused(seg, {}, "value-" + std::to_string(value), Refusal::UnrepresentableValue, {"Too high", std::to_string(value)});
      CPPUNIT_ASSERT_MESSAGE("The refusal is not reported as an ITK error: " + message,
                             message.find("ITK error") == std::string::npos);
    }
  }

  // BodyPartExamined is a property of the series. The writer takes it from the segmentation's
  // DICOM.0018.0015, never from the codes of a label.
  void WriteTakesBodyPartExaminedFromProperty()
  {
    auto seg = BuildSegWithRule();
    seg->GetLabel(1)->SetAnatomicRegion(mitk::DICOMCodeSequenceWithModifiers("T-62000", "SRT", "Liver"));
    seg->SetProperty(DICOMKey(0x0018, 0x0015), mitk::TemporoSpatialStringProperty::New("ABDOMEN"));
    const auto path = this->WriteSegToTempFile(seg, {}, "body-part");

    CPPUNIT_ASSERT_EQUAL_MESSAGE("BodyPartExamined of the written SEG",
                                 std::string("ABDOMEN"), ReadTopLevelString(path, DCM_BodyPartExamined));
  }

  // A segmentation created from an image inherits the body part examined of the image's series.
  void WriteTakesBodyPartExaminedFromTheSourceImage()
  {
    auto source = mitk::test::BuildSourceImageWithDICOMIdentity();
    source->SetProperty(DICOMKey(0x0018, 0x0015), mitk::TemporoSpatialStringProperty::New("ABDOMEN"));
    auto seg = InitializeSegFromSourceImage(source);
    Rule::Connect(seg, source.GetPointer());
    const auto path = this->WriteSegToTempFile(seg, {}, "body-part-from-source");

    CPPUNIT_ASSERT_EQUAL_MESSAGE("BodyPartExamined of the written SEG",
                                 std::string("ABDOMEN"), ReadTopLevelString(path, DCM_BodyPartExamined));
  }

  void WriteLeavesBodyPartExaminedEmptyWithoutProperty()
  {
    auto seg = BuildSegWithRule();
    seg->GetLabel(1)->SetAnatomicRegion(mitk::DICOMCodeSequenceWithModifiers("T-62000", "SRT", "Liver"));
    const auto path = this->WriteSegToTempFile(seg, {}, "no-body-part");

    CPPUNIT_ASSERT_EQUAL_MESSAGE("BodyPartExamined of the written SEG",
                                 std::string(), ReadTopLevelString(path, DCM_BodyPartExamined));
  }

  // StampIdentityTags sets a SeriesDescription other than the default "MITK Segmentation".
  void WriteTakesSeriesDescriptionFromProperty()
  {
    auto seg = BuildSegWithRule();
    const auto path = this->WriteSegToTempFile(seg, {}, "series-description");

    CPPUNIT_ASSERT_EQUAL_MESSAGE("SeriesDescription of the written SEG",
                                 ReadStringProp(seg, 0x0008, 0x103E), ReadTopLevelString(path, DCM_SeriesDescription));
  }

  void WriteKeepsLabelNamesOf64Characters()
  {
    auto seg = BuildSegWithRule();
    seg->GetLabel(1)->SetName(std::string(64, 'n'));
    this->WriteSegToTempFile(seg, {}, "name-64");
  }

  // Segment Label is LO, at most 64 characters; DCMTK does not check the length. The name is cut
  // and marked, as the algorithm name is.
  void WriteTruncatesLongLabelNames()
  {
    const std::string name = "A label name that is deliberately longer than the sixty-four characters DICOM allows";
    auto seg = BuildSegWithRule();
    seg->GetLabel(1)->SetName(name);

    mitk::test::WarningRecorder warnings("(0062,0005)");
    const auto path = this->WriteSegToTempFileUnchecked(seg, {}, "long-name");

    auto expected = mitk::test::ExpectedSegmentSequence(*seg, 0, false);
    expected.at(1)->SetName(name.substr(0, 59) + "[...]");
    AssertSegmentSequence(path, expected);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Warnings about the truncated Segment Label", std::size_t{1}, warnings.GetWarnings().size());
  }

  void WriteTruncatesLongAlgorithmNames()
  {
    const std::string algorithmName = "An algorithm name that is deliberately longer than the sixty-four characters DICOM allows";
    auto seg = BuildSegWithRule();
    seg->GetLabel(1)->SetAlgorithmType(mitk::Label::AlgorithmType::AUTOMATIC);
    seg->GetLabel(1)->SetAlgorithmName(algorithmName);

    mitk::test::WarningRecorder warnings("(0062,0009)");
    const auto path = this->WriteSegToTempFileUnchecked(seg, {}, "long-algorithm-name");

    auto expected = mitk::test::ExpectedSegmentSequence(*seg, 0, false);
    expected.at(1)->SetAlgorithmName(algorithmName.substr(0, 59) + "[...]");
    AssertSegmentSequence(path, expected);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Warnings about the truncated Segment Algorithm Name",
                                 std::size_t{1}, warnings.GetWarnings().size());
  }

  // The description of a label without a name is its Segment Label, which is cut like a name. The
  // Segment Description is ST and keeps the description whole.
  void WriteTruncatesLongDescriptionsOfUnnamedLabels()
  {
    const std::string description = "A label description that is deliberately longer than the sixty-four characters of LO";
    auto seg = BuildSegWithRule();
    seg->GetLabel(1)->SetName("");
    seg->GetLabel(1)->SetDescription(description);

    mitk::test::WarningRecorder warnings("(0062,0005)");
    const auto path = this->WriteSegToTempFileUnchecked(seg, {}, "long-description-of-unnamed");

    auto expected = mitk::test::ExpectedSegmentSequence(*seg, 0, false);
    expected.at(1)->SetName(description.substr(0, 59) + "[...]");
    AssertSegmentSequence(path, expected);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Warnings about the truncated Segment Label", std::size_t{1}, warnings.GetWarnings().size());
  }

  // A backslash separates values in DICOM, so a name containing one cannot be written.
  void WriteRefusesLabelNamesWithBackslash()
  {
    auto seg = BuildSegWithRule();
    seg->GetLabel(1)->SetName("Left\\Right");
    this->AssertWriteRefused(seg, {}, "backslash", Refusal::UnrepresentableValue, {"Left\\Right"});
  }

  // The writer writes no Specific Character Set, so names are limited to ASCII.
  void WriteRefusesNonASCIILabelNames()
  {
    auto seg = BuildSegWithRule();
    seg->GetLabel(1)->SetName("L\xC3\xA4sion");
    this->AssertWriteRefused(seg, {}, "non-ascii", Refusal::UnrepresentableValue, {"L\xC3\xA4sion"});
  }

  // DCMTK checks the Segment Label as LO, which admits neither control characters nor DEL.
  void WriteRefusesLabelNamesWithControlCharacters()
  {
    const std::vector<std::string> names = { "Left\tRight", "Left\nRight", "Left\x7F" "Right" };
    for (std::size_t i = 0; i < names.size(); ++i)
    {
      auto seg = BuildSegWithRule();
      seg->GetLabel(1)->SetName(names[i]);
      this->AssertWriteRefused(seg, {}, "control-character-" + std::to_string(i), Refusal::UnrepresentableValue,
                               {"Left", "Segment Label"});
    }
  }

  // dcmqi hands the algorithm name of an AUTOMATIC or SEMIAUTOMATIC label to DCMTK, which checks
  // it as LO without a Specific Character Set.
  void WriteRefusesInvalidAlgorithmNames()
  {
    const std::vector<std::string> names = { "Tool \xC3\xA4", "Tool\tA", "Tool\\A" };
    const std::vector<mitk::Label::AlgorithmType> types = { mitk::Label::AlgorithmType::AUTOMATIC,
                                                            mitk::Label::AlgorithmType::SEMIAUTOMATIC };
    for (std::size_t i = 0; i < names.size(); ++i)
    {
      for (std::size_t t = 0; t < types.size(); ++t)
      {
        auto seg = BuildSegWithRule();
        seg->GetLabel(1)->SetAlgorithmType(types[t]);
        seg->GetLabel(1)->SetAlgorithmName(names[i]);
        this->AssertWriteRefused(seg, {}, "algorithm-name-" + std::to_string(i) + "-" + std::to_string(t),
                                 Refusal::UnrepresentableValue, {"Segment Algorithm Name"});
      }
    }
  }

  // dcmqi writes no algorithm name for a MANUAL label, and the writer writes an undefined algorithm
  // type as MANUAL, so a name that DCMTK would refuse does not stop these writes.
  void WriteIgnoresAlgorithmNamesItDoesNotWrite()
  {
    auto manual = BuildSegWithRule();
    manual->GetLabel(1)->SetAlgorithmName("Tool \xC3\xA4");
    this->WriteSegToTempFile(manual, {}, "manual-algorithm-name");

    auto undefined = BuildSegWithRule();
    undefined->GetLabel(1)->SetAlgorithmType(mitk::Label::AlgorithmType::Undefined);
    undefined->GetLabel(1)->SetAlgorithmName("Tool \xC3\xA4");
    mitk::IFileWriter::Options options;
    options["Strict / synthetic mode"] = std::string("synthetic");
    this->WriteSegToTempFile(undefined, options, "undefined-algorithm-name");
  }

  // dcmqi needs the series of the source images for the Referenced Series Sequence, and the SOP
  // Instance and SOP Class UIDs of every source image it references; DCMTK checks each as UI.
  void WriteRefusesUnwritableRelations()
  {
    for (const auto& relation : UnwritableRelations())
    {
      auto seg = BuildBaseSeg(/*stampIdentity=*/true);
      auto fragments = relation.fragments;
      fragments.push_back(ConnectRelation(seg, relation));
      this->AssertWriteRefused(seg, {}, relation.name, Refusal::Plain, fragments);
    }
  }

  // No mode can write such a relation. So the refusal comes before the strict-mode refusal of an
  // incomplete contract, which promises that synthetic mode can succeed, and before synthetic mode
  // stamps the segmentation.
  void WriteRefusesUnwritableRelationBeforeContractCheck()
  {
    for (const auto& relation : UnwritableRelations())
    {
      auto strict = BuildBaseSeg(/*stampIdentity=*/false);
      const auto strictRelationUID = ConnectRelation(strict, relation);
      this->AssertWriteRefused(strict, {}, relation.name + "-strict", Refusal::Plain, {strictRelationUID});

      auto synthetic = BuildBaseSeg(/*stampIdentity=*/false);
      ConnectRelation(synthetic, relation);
      CPPUNIT_ASSERT_MESSAGE(relation.name + ": precondition: the segmentation has no Patient ID",
                             ReadStringProp(synthetic, 0x0010, 0x0020).empty());
      CPPUNIT_ASSERT_MESSAGE(relation.name + ": precondition: the segmentation has no Study Instance UID",
                             ReadStringProp(synthetic, 0x0020, 0x000D).empty());
      mitk::IFileWriter::Options options;
      options["Strict / synthetic mode"] = std::string("synthetic");
      CPPUNIT_ASSERT_THROW_MESSAGE(relation.name + ": synthetic mode refuses the relation",
                                   mitk::test::WriteWithWriter(synthetic,
                                                               this->GetTempFilePath(relation.name + "-synthetic.dcm"),
                                                               "DICOM Segmentation", options),
                                   mitk::Exception);
      CPPUNIT_ASSERT_MESSAGE(relation.name + ": the refused write leaves the segmentation unstamped",
                             ReadStringProp(synthetic, 0x0010, 0x0020).empty() &&
                               ReadStringProp(synthetic, 0x0020, 0x000D).empty());
    }
  }

  // The writer writes no Specific Character Set, and DCMTK checks the identity values dcmqi takes
  // over from the first source item against their VRs when it writes the SEG.
  void WriteRefusesUnwritableIdentityValues()
  {
    for (const auto& value : UnwritableIdentityValues())
    {
      auto seg = BuildSegWithRule(/*stampIdentity=*/true);
      SetIdentityValue(seg, value);
      this->AssertWriteRefused(seg, {}, value.name, Refusal::Plain, value.fragments);
    }
  }

  // Synthetic mode keeps a value that is present, so no mode can write such a value. The refusal
  // comes before the strict-mode refusal of an incomplete contract and before synthetic mode stamps
  // the segmentation.
  void WriteRefusesUnwritableIdentityValueBeforeContractCheck()
  {
    for (const auto& value : UnwritableIdentityValues())
    {
      auto strict = BuildSegWithRule(/*stampIdentity=*/false);
      SetIdentityValue(strict, value);
      this->AssertWriteRefused(strict, {}, value.name + "-strict", Refusal::Plain, value.fragments);

      auto synthetic = BuildSegWithRule(/*stampIdentity=*/false);
      SetIdentityValue(synthetic, value);
      CPPUNIT_ASSERT_MESSAGE(value.name + ": precondition: the segmentation has no Patient ID",
                             ReadStringProp(synthetic, 0x0010, 0x0020).empty());
      mitk::IFileWriter::Options options;
      options["Strict / synthetic mode"] = std::string("synthetic");
      CPPUNIT_ASSERT_THROW_MESSAGE(value.name + ": synthetic mode refuses the value",
                                   mitk::test::WriteWithWriter(synthetic,
                                                               this->GetTempFilePath(value.name + "-synthetic.dcm"),
                                                               "DICOM Segmentation", options),
                                   mitk::Exception);
      CPPUNIT_ASSERT_MESSAGE(value.name + ": the refused write leaves the segmentation unstamped",
                             ReadStringProp(synthetic, 0x0010, 0x0020).empty());
    }
  }

  // dcmqi reports some failures by returning no SEG, here the documented one of the labelmap
  // encoding with values 1..N: a label without voxels below one with voxels.
  void WriteTurnsDcmqiFailuresIntoExceptions()
  {
    auto seg = BuildSegWithLabels({{1, "Without voxels"}, {2, "With voxels"}});
    {
      mitk::ImageWriteAccessor writeAccessor(seg->GetGroupImage(0));
      auto* pixels = static_cast<mitk::Label::PixelType*>(writeAccessor.GetData());
      const auto dims = seg->GetGroupImage(0)->GetDimensions();
      const auto sliceSize = static_cast<std::size_t>(dims[0]) * dims[1];
      for (unsigned int z = 0; z < dims[2]; ++z)
        pixels[z * sliceSize] = mitk::MultiLabelSegmentation::UNLABELED_VALUE;
    }
    mitk::IFileWriter::Options options;
    options["Segmentation encoding"] = std::string("labelmap");
    this->AssertWriteRefused(seg, options, "dcmqi-failure", Refusal::Plain, {"dcmqi", "group 0"});
  }

  // Recommended Display CIELab Value is computed from 8 bit sRGB, rounded, not truncated.
  void WriteRoundsColours()
  {
    mitk::Color color;
    color.Set(0.999f, 0.5f, 0.2f);
    CPPUNIT_ASSERT_MESSAGE("Precondition: truncating and rounding differ for the red channel",
                           static_cast<long>(color[0] * 255.f) != std::lround(color[0] * 255.f));

    auto seg = BuildSegWithRule();
    seg->GetLabel(1)->SetColor(color);
    const auto path = this->WriteSegToTempFile(seg, {}, "colour");

    int lab[3];
    dcmqi::ColorUtilities::getIntegerScaledCIELabPCSFromSRGB(lab[0], lab[1], lab[2], 255, 128, 51);
    const auto written = mitk::test::ReadRecommendedDisplayCIELab(path).at(1);
    for (unsigned int i = 0; i < 3; ++i)
      CPPUNIT_ASSERT_EQUAL_MESSAGE("CIELab component " + std::to_string(i), lab[i], static_cast<int>(written[i]));
  }

  // DICOMSegStrictModeException tells callers that a retry in synthetic mode can succeed.
  void StrictModeRefusalsAreStrictModeExceptions()
  {
    this->AssertWriteRefused(BuildSegWithRule(/*stampIdentity=*/false), {}, "strict-incomplete", Refusal::StrictMode, {"PatientID"});
    this->AssertWriteRefused(BuildBaseSeg(/*stampIdentity=*/true), {}, "strict-no-relation", Refusal::StrictMode, {"group 0"});

    auto idLayerOnly = BuildBaseSeg(/*stampIdentity=*/true);
    AttachIDLayerOnlyRelation(idLayerOnly);
    this->AssertWriteRefused(idLayerOnly, {}, "strict-id-layer", Refusal::StrictMode, {"group 0"});
  }

  // A refusal in synthetic mode names what synthesis cannot fill and suggests no mode switch.
  void SyntheticModeRefusalIsPlainException()
  {
    auto seg = BuildSegWithRule();
    seg->GetLabel(1)->SetAlgorithmType(mitk::Label::AlgorithmType::AUTOMATIC);
    seg->GetLabel(1)->SetAlgorithmName("");
    mitk::IFileWriter::Options options;
    options["Strict / synthetic mode"] = std::string("synthetic");
    const auto message =
      this->AssertWriteRefused(seg, options, "synthetic-refusal", Refusal::Plain, {"[label 1]", "Algorithm Name"});
    CPPUNIT_ASSERT_MESSAGE("The synthetic-mode refusal suggests no mode switch: " + message,
                           message.find("Strict / synthetic mode") == std::string::npos);
  }

  // The file name, not the directory, decides the extension of the per-group files. IOUtil hands a
  // path without extension to the writer when the mime type is explicit and no extension is added;
  // every group file then has none either.
  void WriteKeepsDottedDirectoryOfPathWithoutExtension()
  {
    auto seg = BuildTwoGroupSeg();
    const auto directory = this->GetTempFilePath("data.v2");
    itksys::SystemTools::MakeDirectory(directory);
    const auto path = directory + mitk::IOUtil::GetDirectorySeparator() + "seg";

    mitk::IOUtil::Save(seg, mitk::test::DICOMSegMimeTypeName(), path, /*addExtension=*/false);

    CPPUNIT_ASSERT_MESSAGE("Nothing is written next to the dotted directory",
                           !FileExists(this->GetTempFilePath("data.dcm")) && !FileExists(this->GetTempFilePath("data_1.dcm")));
    CPPUNIT_ASSERT_MESSAGE("No extension is added",
                           !FileExists(path + ".dcm") && !FileExists(path + "_1.dcm"));
    for (mitk::MultiLabelSegmentation::GroupIndexType group = 0; group < 2; ++group)
    {
      const auto file = GroupFilePath(path, group);
      CPPUNIT_ASSERT_MESSAGE(file + " exists", FileExists(file));
      AssertWrittenSegments(file, *seg, group, false);
    }
  }

  // The writer keeps the extension it is given rather than replacing it by ".dcm"; IOUtil decides
  // whether an extension is added.
  void WriteKeepsTheExtensionOfTheRequestedPath()
  {
    auto seg = BuildTwoGroupSeg();
    const auto path = this->GetTempFilePath("seg.ima");

    mitk::IOUtil::Save(seg, mitk::test::DICOMSegMimeTypeName(), path, /*addExtension=*/false);

    CPPUNIT_ASSERT_MESSAGE("The extension is not replaced",
                           !FileExists(this->GetTempFilePath("seg.dcm")) && !FileExists(this->GetTempFilePath("seg_1.dcm")));
    for (mitk::MultiLabelSegmentation::GroupIndexType group = 0; group < 2; ++group)
    {
      const auto file = GroupFilePath(path, group);
      CPPUNIT_ASSERT_MESSAGE(file + " exists", FileExists(file));
      AssertWrittenSegments(file, *seg, group, false);
    }
  }

  // A file name of digits and dots only, such as a UID, has no extension, so the group number is
  // appended to it. IOUtil adds no extension to such a name, since it takes everything from the
  // first dot for one.
  void WriteKeepsDottedStemOfPathWithoutExtension()
  {
    auto seg = BuildTwoGroupSeg();
    const auto path = this->GetTempFilePath("1.2.840.113");

    mitk::IOUtil::Save(seg, mitk::test::DICOMSegMimeTypeName(), path, /*addExtension=*/true);

    CPPUNIT_ASSERT_MESSAGE("The first group is written to the requested path", FileExists(path));
    CPPUNIT_ASSERT_MESSAGE("The stem is not split", !FileExists(this->GetTempFilePath("1.2.840_1.113")));
    CPPUNIT_ASSERT_MESSAGE("The group number is appended", FileExists(path + "_1"));
    AssertWrittenSegments(path, *seg, 0, false);
    AssertWrittenSegments(path + "_1", *seg, 1, false);
  }

  // RemoveAnatomicRegion leaves the indices sparse; the writer takes the region with the lowest
  // existing index.
  void WriteUsesLowestExistingAnatomicRegion()
  {
    auto seg = BuildSegWithRule();
    auto label = seg->GetLabel(1);
    label->SetAnatomicRegion(mitk::DICOMCodeSequenceWithModifiers("R0", "99MITK", "Region zero"), 0);
    mitk::DICOMCodeSequenceWithModifiers second("R1", "99MITK", "Region one");
    second.AddModifier(mitk::DICOMCodeSequence("R1M", "99MITK", "Region one modifier"));
    label->SetAnatomicRegion(second, 1);
    label->RemoveAnatomicRegion(0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Precondition: one region left, at index 1", std::size_t{1}, label->GetAnatomicRegionCount());

    const auto path = this->WriteSegToTempFile(seg, {}, "sparse-region");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The written region", std::string("Region one"),
                                 mitk::test::ReadSegmentSequence(path).at(1)->GetAnatomicRegion(0).GetMeaning());
  }

  // The writer passes the anatomic region with the lowest index only if its value, scheme and meaning
  // are all set, and its first modifier likewise; it does not fall back to a later region. dcmqi
  // writes a region modifier only together with its region. WriteSegToTempFile checks the expectation
  // it derives; the literal values pin the writer independently of it.
  void WriteLeavesOutIncompleteAnatomicRegions()
  {
    struct Case
    {
      std::string nameHint;
      std::function<void(mitk::Label&)> edit;
      std::string regionValue; // empty if no region is written
    };
    const std::vector<Case> cases = {
      { "incomplete-region",
        [](mitk::Label& label) {
          mitk::DICOMCodeSequenceWithModifiers region("", "SCT", "Liver");
          region.AddModifier(mitk::DICOMCodeSequence("7771000", "SCT", "Left"));
          label.SetAnatomicRegion(region, 0);
        },
        "" },
      { "incomplete-region-modifier",
        [](mitk::Label& label) {
          mitk::DICOMCodeSequenceWithModifiers region("10200004", "SCT", "Liver");
          region.AddModifier(mitk::DICOMCodeSequence("", "SCT", "Left"));
          label.SetAnatomicRegion(region, 0);
        },
        "10200004" },
      { "incomplete-lowest-region",
        [](mitk::Label& label) {
          label.SetAnatomicRegion(mitk::DICOMCodeSequenceWithModifiers("", "SCT", "Liver"), 0);
          label.SetAnatomicRegion(mitk::DICOMCodeSequenceWithModifiers("10200004", "SCT", "Liver"), 1);
        },
        "" },
    };

    for (const auto& c : cases)
    {
      auto seg = BuildSegWithRule();
      c.edit(*seg->GetLabel(1));
      const auto path = this->WriteSegToTempFile(seg, {}, c.nameHint);

      const auto segment = mitk::test::ReadSegmentSequence(path).at(1);
      if (c.regionValue.empty())
      {
        CPPUNIT_ASSERT_EQUAL_MESSAGE(c.nameHint + ": no anatomic region", std::size_t{0},
                                     segment->GetAnatomicRegionCount());
      }
      else
      {
        CPPUNIT_ASSERT_EQUAL_MESSAGE(c.nameHint + ": one anatomic region", std::size_t{1},
                                     segment->GetAnatomicRegionCount());
        const auto region = segment->GetAnatomicRegion(0);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(c.nameHint + ": region code value", c.regionValue, region.GetValue());
        CPPUNIT_ASSERT_EQUAL_MESSAGE(c.nameHint + ": no region modifier", std::size_t{0}, region.GetModifierCount());
      }
    }
  }

  void WriteRefuses4D()
  {
    mitk::test::RichSegmentationOptions fixtureOptions;
    fixtureOptions.dynamic = true;
    fixtureOptions.dicomLegalNames = true;
    fixtureOptions.withEmptyLabel = false;
    fixtureOptions.withEmptyGroup = false;
    const auto seg = mitk::test::BuildRichSegmentation(fixtureOptions);
    AssertNoWriterFound(seg, this->GetTempFilePath("4d.dcm"));
  }

  void WriteRefuses2D()
  {
    auto image = mitk::Image::New();
    unsigned int dim[2] = {4u, 4u};
    image->Initialize(mitk::MakeScalarPixelType<mitk::Label::PixelType>(), 2, dim);
    auto seg = mitk::MultiLabelSegmentation::New();
    seg->Initialize(image);
    seg->AddLabel(mitk::Label::New(1, "L"), 0, true, false);
    AssertNoWriterFound(seg, this->GetTempFilePath("2d.dcm"));
  }

  void WriteRefusesDataThatIsNoSegmentation()
  {
    auto image = mitk::Image::New();
    unsigned int dim[3] = {4u, 4u, 3u};
    image->Initialize(mitk::MakeScalarPixelType<mitk::Label::PixelType>(), 3, dim);
    AssertNoWriterFound(image, this->GetTempFilePath("image.dcm"));
  }

  // The labels of all groups are checked before the first file is written.
  void WriteRefusesUnwritableLabelOfLaterGroupBeforeAnyFile()
  {
    const std::vector<std::pair<std::string, std::function<void(mitk::MultiLabelSegmentation*)>>> edits = {
      { "backslash", [](mitk::MultiLabelSegmentation* seg) { seg->GetLabel(2)->SetName("Left\\Right"); } },
      { "non-ascii", [](mitk::MultiLabelSegmentation* seg) { seg->GetLabel(2)->SetName("L\xC3\xA4sion"); } },
      { "control-character", [](mitk::MultiLabelSegmentation* seg) { seg->GetLabel(2)->SetName("Left\tRight"); } },
      { "algorithm-name",
        [](mitk::MultiLabelSegmentation* seg) {
          seg->GetLabel(2)->SetAlgorithmType(mitk::Label::AlgorithmType::AUTOMATIC);
          seg->GetLabel(2)->SetAlgorithmName("Tool \xC3\xA4");
        } },
      { "tracking-uid", [](mitk::MultiLabelSegmentation* seg) { seg->GetLabel(2)->SetTrackingUID("track-uid"); } },
      { "description", [](mitk::MultiLabelSegmentation* seg) { seg->GetLabel(2)->SetDescription("L\xC3\xA4sion links"); } },
      { "tracking-id", [](mitk::MultiLabelSegmentation* seg) { seg->GetLabel(2)->SetTrackingID("Tr\xC3\xA4" "ck"); } },
      { "description-of-unnamed",
        [](mitk::MultiLabelSegmentation* seg) {
          seg->GetLabel(2)->SetName("");
          seg->GetLabel(2)->SetDescription("Left\\Right");
        } },
      { "value",
        [](mitk::MultiLabelSegmentation* seg) {
          auto label = mitk::Label::New();
          label->SetName("Too high");
          label->SetValue(32768);
          label->SetAlgorithmType(mitk::Label::AlgorithmType::MANUAL);
          label->SetAlgorithmName("WorkflowTest");
          seg->AddLabel(label, 1, true, false);
        } },
    };
    for (const auto& [name, edit] : edits)
    {
      auto seg = BuildTwoGroupSeg();
      edit(seg);
      const auto path = this->GetTempFilePath("later-group-" + name + ".dcm");

      CPPUNIT_ASSERT_THROW_MESSAGE(name + ": the label of the second group is refused", mitk::IOUtil::Save(seg, path),
                                   mitk::Exception);
      CPPUNIT_ASSERT_MESSAGE(name + ": the first group has no file", !FileExists(path));
      CPPUNIT_ASSERT_MESSAGE(name + ": the second group has no file", !FileExists(GroupFilePath(path, 1)));
    }
  }

  // Setting the index-to-world transform of a sliced geometry leaves its slice planes where they
  // were. dcmqi places the frames by the transform and maps the source images to them by position,
  // so positions taken from the planes map to no frame.
  void WriteReferencesSourcesWhenPlanesDisagreeWithTransform()
  {
    auto source = mitk::test::BuildSourceImageWithDICOMIdentity();
    auto shifted = mitk::AffineTransform3D::New();
    shifted->SetMatrix(source->GetGeometry()->GetIndexToWorldTransform()->GetMatrix());
    mitk::Vector3D offset;
    mitk::FillVector3D(offset, 5.0, -3.0, 10.0);
    shifted->SetOffset(offset);
    source->GetGeometry()->SetIndexToWorldTransform(shifted);

    auto seg = InitializeSegFromSourceImage(source);
    Rule::Connect(seg, source.GetPointer());
    const auto* groupGeometry = seg->GetGroupImage(0)->GetSlicedGeometry();
    CPPUNIT_ASSERT_MESSAGE("Precondition: the slice planes disagree with the index-to-world transform",
                           groupGeometry->GetPlaneGeometry(0)->GetOrigin() != groupGeometry->GetOrigin());

    const auto path = this->WriteSegToTempFile(seg, {}, "stale-planes");
    AssertSourceReferencesAreExactly(path, source, "Strict");
  }

  // dcmqi skips source images it cannot place on a frame without failing. A relation whose slices
  // all lie outside the segmentation would leave a SEG that names no source.
  void WriteRefusesSourceReferencesThatMapToNoFrame()
  {
    auto seg = BuildBaseSeg(/*stampIdentity=*/true);
    auto perSliceInstance = mitk::TemporoSpatialStringProperty::New();
    auto perSliceClass = mitk::TemporoSpatialStringProperty::New();
    for (int sliceIndex = 10; sliceIndex < 13; ++sliceIndex)
    {
      perSliceInstance->SetValue(0, sliceIndex, "1.2.826.0.1.3680043.10.999.12.1." + std::to_string(sliceIndex));
      perSliceClass->SetValue(0, sliceIndex, "1.2.840.10008.5.1.4.1.1.2");
    }
    Rule::New()->Connect(seg, perSliceInstance, perSliceClass, "1.2.826.0.1.3680043.10.999.12.2");

    this->AssertWriteRefused(seg, {}, "unmapped-sources", Refusal::Plain, {"group 0"});
  }

  // The reader adds the labels of a labelmap in segment order, as it does for a binary SEG. Label 2
  // covers the first voxel here, so the order in which values first occur differs.
  void LabelmapReloadOrdersLabelsBySegmentNumber()
  {
    auto seg = BuildSegWithLabels({{2, "B"}, {1, "A"}});
    mitk::IFileWriter::Options options;
    options["Segmentation encoding"] = std::string("labelmap");
    const auto path = this->WriteSegToTempFile(seg, options, "labelmap-order");

    const auto loaded = mitk::IOUtil::Load(path);
    const auto* loadedSeg = dynamic_cast<const mitk::MultiLabelSegmentation*>(loaded.front().GetPointer());
    CPPUNIT_ASSERT_MESSAGE("The labelmap reloads as a segmentation", nullptr != loadedSeg);
    const auto differences = mitk::test::CompareWithDICOMSegFile(*loadedSeg, path);
    CPPUNIT_ASSERT_MESSAGE(path + " was read with differences to its content:" + mitk::test::JoinDifferences(differences),
                           differences.empty());
  }

  // Tracking UID is UI. DCMTK rejects other values inside dcmqi, which then leaves the tag out.
  void WriteRefusesInvalidTrackingUID()
  {
    auto seg = BuildSegWithRule();
    seg->GetLabel(1)->SetTrackingUID("track-uid");
    this->AssertWriteRefused(seg, {}, "invalid-tracking-uid", Refusal::UnrepresentableValue, {"track-uid"});
  }

  // Segment Description is ST. Without Specific Character Set it admits ASCII only and, of the
  // control characters, TAB, LF, FF, CR and ESC. DCMTK checks it only when dcmqi writes the file.
  void WriteRefusesInvalidSegmentDescriptions()
  {
    const std::vector<std::string> descriptions = { "L\xC3\xA4sion links", "Left\x01Right", "Left\x7F" "Right" };
    for (std::size_t i = 0; i < descriptions.size(); ++i)
    {
      auto seg = BuildSegWithRule();
      seg->GetLabel(1)->SetDescription(descriptions[i]);
      this->AssertWriteRefused(seg, {}, "description-" + std::to_string(i), Refusal::UnrepresentableValue,
                               {"label 1", "Segment Description"});
    }
  }

  // Tracking ID is UT, which DCMTK checks like ST.
  void WriteRefusesInvalidTrackingIDs()
  {
    const std::vector<std::string> trackingIDs = { "Tr\xC3\xA4" "ck", "Track\x01", "Track\x7F" };
    for (std::size_t i = 0; i < trackingIDs.size(); ++i)
    {
      auto seg = BuildSegWithRule();
      seg->GetLabel(1)->SetTrackingID(trackingIDs[i]);
      this->AssertWriteRefused(seg, {}, "tracking-id-" + std::to_string(i), Refusal::UnrepresentableValue,
                               {"label 1", "Tracking ID"});
    }
  }

  // Unlike LO, ST and UT hold a single value, so a backslash separates nothing, and they admit
  // line breaks and tabs.
  void WriteKeepsTextWithBackslashAndLineBreaks()
  {
    auto seg = BuildSegWithRule();
    seg->GetLabel(1)->SetDescription("Left\\Right\tA\r\nB");
    seg->GetLabel(1)->SetTrackingID("TID\\1");
    this->WriteSegToTempFile(seg, {}, "text-with-backslash-and-line-breaks");
  }

  // Segment Label is Type 1, so a label without a name is written with its description as Segment
  // Label and reloads under that name.
  void WriteNamesUnnamedLabelsByTheirDescription()
  {
    auto seg = BuildSegWithRule();
    seg->GetLabel(1)->SetName("");
    seg->GetLabel(1)->SetDescription("Liver lesion");
    const auto path = this->WriteSegToTempFile(seg, {}, "unnamed");

    const auto loaded = mitk::IOUtil::Load(path);
    const auto* loadedSeg = dynamic_cast<const mitk::MultiLabelSegmentation*>(loaded.front().GetPointer());
    CPPUNIT_ASSERT_MESSAGE("The file reloads as a segmentation", nullptr != loadedSeg);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The name of the reloaded label", std::string("Liver lesion"), loadedSeg->GetLabel(1)->GetName());
  }

  // DICOM counts an LO value of spaces only as empty, and DCMTK refuses an empty Segment Label, so a
  // name or description of spaces only counts as none. BuildSegWithRule gives the label the type
  // "Tissue". DICOM drops the trailing spaces of a description, so only the first case can be
  // checked against the expectation WriteSegToTempFile derives.
  void WriteTreatsBlankNamesAndDescriptionsAsNone()
  {
    {
      auto seg = BuildSegWithRule();
      seg->GetLabel(1)->SetName("   ");
      seg->GetLabel(1)->SetDescription("Liver lesion");
      const auto path = this->WriteSegToTempFile(seg, {}, "blank-name");
      CPPUNIT_ASSERT_EQUAL_MESSAGE("The Segment Label of a label with a blank name", std::string("Liver lesion"),
                                   mitk::test::ReadSegmentSequence(path).at(1)->GetName());
    }

    const std::vector<std::string> names = { "", "   " };
    for (std::size_t i = 0; i < names.size(); ++i)
    {
      auto seg = BuildSegWithRule();
      seg->GetLabel(1)->SetName(names[i]);
      seg->GetLabel(1)->SetDescription("   ");
      const auto path = this->WriteSegToTempFileUnchecked(seg, {}, "blank-description-" + std::to_string(i));
      CPPUNIT_ASSERT_EQUAL_MESSAGE("The Segment Label of a label with name \"" + names[i] + "\" and a blank description",
                                   std::string("Tissue"), mitk::test::ReadSegmentSequence(path).at(1)->GetName());
    }
  }

  // The Segment Label of a label without a name is its description, else the meaning of its type, and
  // it is LO: unlike the description (ST), it admits no backslash and no control characters, and
  // unlike a code meaning elsewhere, DCMTK checks it. A name of spaces only counts as none.
  void WriteRefusesUnnamedLabelsWhoseSegmentLabelIsNoLOValue()
  {
    const std::vector<std::string> descriptions = { "Left\\Right", "Left\nRight", "Left\tRight" };
    for (std::size_t i = 0; i < descriptions.size(); ++i)
    {
      auto seg = BuildSegWithRule();
      seg->GetLabel(1)->SetName("");
      seg->GetLabel(1)->SetDescription(descriptions[i]);
      this->AssertWriteRefused(seg, {}, "unnamed-description-" + std::to_string(i), Refusal::UnrepresentableValue,
                               {"label 1", "Segment Label", "description"});
    }

    {
      auto seg = BuildSegWithRule();
      seg->GetLabel(1)->SetName("   ");
      seg->GetLabel(1)->SetDescription("Left\\Right");
      this->AssertWriteRefused(seg, {}, "blank-name-description", Refusal::UnrepresentableValue,
                               {"label 1", "Segment Label", "description"});
    }

    // A type meaning of spaces only would be an empty Segment Label.
    const std::vector<std::string> typeMeanings = { "Gew\xC3\xA4" "be", "Left\\Right", "   " };
    for (std::size_t i = 0; i < typeMeanings.size(); ++i)
    {
      auto seg = BuildSegWithRule();
      seg->GetLabel(1)->SetName("");
      seg->GetLabel(1)->SetSegmentedPropertyType(mitk::DICOMCodeSequenceWithModifiers("T-D0050", "SRT", typeMeanings[i]));
      this->AssertWriteRefused(seg, {}, "unnamed-type-meaning-" + std::to_string(i), Refusal::UnrepresentableValue,
                               {"label 1", "Segment Label", "Segmented Property Type"});
    }
  }

  // dcmqi writes no segment for a label without voxels, so it never checks its Segment Label. The
  // writer checks every label, so such a label is refused like one with voxels. A name of spaces only
  // counts as none.
  void WriteRefusesUnnamedLabelWithoutVoxelsWhoseDescriptionIsNoLOValue()
  {
    const std::vector<std::string> names = { "", "   " };
    for (std::size_t i = 0; i < names.size(); ++i)
    {
      auto seg = BuildSegWithRule();
      auto unnamed = mitk::Label::New(2, names[i]);
      unnamed->SetDescription("Left\\Right");
      unnamed->SetAlgorithmType(mitk::Label::AlgorithmType::MANUAL);
      seg->AddLabel(unnamed, 0, true, false);
      CPPUNIT_ASSERT_MESSAGE("Precondition: label 2 has no voxels", seg->IsEmpty(2));
      this->AssertWriteRefused(seg, {}, "unnamed-without-voxels-" + std::to_string(i), Refusal::UnrepresentableValue,
                               {"label 2", "Segment Label", "description"});
    }
  }

  // A value outside the choices is refused rather than taken for one of them. The segmentation is
  // complete for strict mode, so only the options can refuse it.
  void WriteRefusesUnknownOptionValues()
  {
    const std::vector<std::pair<std::string, std::string>> unknownValues = {
      { "Strict / synthetic mode", "Synthetic" }, { "Segmentation encoding", "Labelmap" } };
    for (const auto& [name, value] : unknownValues)
    {
      mitk::IFileWriter::Options options;
      options[name] = value;
      this->AssertWriteRefused(BuildSegWithRule(), options, "unknown-" + value, Refusal::Plain, {name, value});
    }
  }

  // Only changing the label lets such a write succeed. So the labels are checked before the
  // strict-mode refusal of an incomplete contract, which promises that synthetic mode can succeed,
  // and before synthetic mode stamps the segmentation.
  void WriteRefusesUnwritableLabelBeforeTheContractCheck()
  {
    auto synthetic = BuildSegWithRule(/*stampIdentity=*/false);
    synthetic->GetLabel(1)->SetName("Left\\Right");
    CPPUNIT_ASSERT_MESSAGE("Precondition: the segmentation has no Patient ID", ReadStringProp(synthetic, 0x0010, 0x0020).empty());
    CPPUNIT_ASSERT_MESSAGE("Precondition: the segmentation has no Study Instance UID",
                           ReadStringProp(synthetic, 0x0020, 0x000D).empty());
    mitk::IFileWriter::Options options;
    options["Strict / synthetic mode"] = std::string("synthetic");
    CPPUNIT_ASSERT_THROW_MESSAGE("Synthetic mode refuses the label",
                                 mitk::test::WriteWithWriter(synthetic, this->GetTempFilePath("unwritable-synthetic.dcm"),
                                                             "DICOM Segmentation", options),
                                 mitk::DICOMSegUnrepresentableValueException);
    CPPUNIT_ASSERT_MESSAGE("The refused write leaves the segmentation unstamped",
                           ReadStringProp(synthetic, 0x0010, 0x0020).empty() &&
                             ReadStringProp(synthetic, 0x0020, 0x000D).empty());

    auto strict = BuildSegWithRule(/*stampIdentity=*/false);
    strict->GetLabel(1)->SetName("Left\\Right");
    this->AssertWriteRefused(strict, {}, "unwritable-strict", Refusal::UnrepresentableValue, {"Left\\Right"});
  }

  // A segmentation without groups has nothing to write. It is refused before synthetic mode stamps
  // it, and in strict mode before the refusal of an incomplete contract, which would promise that
  // synthetic mode succeeds.
  void WriteRefusesSegmentationWithoutGroups()
  {
    const auto withoutGroups = [](bool stampIdentity) {
      auto seg = BuildSegWithRule(stampIdentity);
      while (seg->GetNumberOfGroups() > 0)
        seg->RemoveGroup(0);
      return seg;
    };

    auto synthetic = withoutGroups(/*stampIdentity=*/false);
    CPPUNIT_ASSERT_MESSAGE("Precondition: the segmentation has no Patient ID", ReadStringProp(synthetic, 0x0010, 0x0020).empty());
    mitk::IFileWriter::Options options;
    options["Strict / synthetic mode"] = std::string("synthetic");
    const auto path = this->GetTempFilePath("no-groups-synthetic.dcm");
    CPPUNIT_ASSERT_THROW_MESSAGE("Synthetic mode refuses the segmentation",
                                 mitk::test::WriteWithWriter(synthetic, path, "DICOM Segmentation", options),
                                 mitk::Exception);
    CPPUNIT_ASSERT_MESSAGE("No file is written", !FileExists(path));
    CPPUNIT_ASSERT_MESSAGE("The refused write leaves the segmentation unstamped",
                           ReadStringProp(synthetic, 0x0010, 0x0020).empty() &&
                             ReadStringProp(synthetic, 0x0020, 0x000D).empty());

    this->AssertWriteRefused(withoutGroups(/*stampIdentity=*/false), {}, "no-groups-incomplete", Refusal::Plain, {"no groups"});
    this->AssertWriteRefused(withoutGroups(/*stampIdentity=*/true), {}, "no-groups", Refusal::Plain, {"no groups"});
  }

  // A DICOM SEG holds at least one segment, so a group without labels is refused, before synthetic
  // mode stamps the segmentation, before the strict-mode refusal and before any file is written. A
  // new segmentation has one such group.
  void WriteRefusesGroupsWithoutLabelsBeforeAnything()
  {
    auto geometryImage = mitk::Image::New();
    unsigned int dim[3] = {4u, 4u, 3u};
    geometryImage->Initialize(mitk::MakeScalarPixelType<mitk::Label::PixelType>(), 3, dim);
    auto fresh = mitk::MultiLabelSegmentation::New();
    fresh->Initialize(geometryImage);
    CPPUNIT_ASSERT_MESSAGE("Precondition: one group without labels",
                           1 == fresh->GetNumberOfGroups() && fresh->GetLabelValuesByGroup(0).empty());
    CPPUNIT_ASSERT_MESSAGE("Precondition: the segmentation has no Patient ID", ReadStringProp(fresh, 0x0010, 0x0020).empty());

    mitk::IFileWriter::Options options;
    options["Strict / synthetic mode"] = std::string("synthetic");
    const auto path = this->GetTempFilePath("no-labels-synthetic.dcm");
    CPPUNIT_ASSERT_THROW_MESSAGE("Synthetic mode refuses the group",
                                 mitk::test::WriteWithWriter(fresh, path, "DICOM Segmentation", options),
                                 mitk::Exception);
    CPPUNIT_ASSERT_MESSAGE("No file is written", !FileExists(path));
    CPPUNIT_ASSERT_MESSAGE("The refused write leaves the segmentation unstamped",
                           ReadStringProp(fresh, 0x0010, 0x0020).empty() &&
                             ReadStringProp(fresh, 0x0020, 0x000D).empty());

    this->AssertWriteRefused(fresh, {}, "no-labels-strict", Refusal::Plain, {"group 0", "no labels"});

    auto laterGroup = BuildSegWithRule();
    laterGroup->AddGroup();
    const auto laterPath = this->GetTempFilePath("no-labels-later-group.dcm");
    CPPUNIT_ASSERT_THROW_MESSAGE("The second group is refused", mitk::IOUtil::Save(laterGroup, laterPath), mitk::Exception);
    CPPUNIT_ASSERT_MESSAGE("The first group has no file", !FileExists(laterPath));
    CPPUNIT_ASSERT_MESSAGE("The second group has no file", !FileExists(GroupFilePath(laterPath, 1)));
  }

  void PropertyDrivenWriteSucceedsAndReloads()
  {
    auto seg = BuildSegWithRule();
    CPPUNIT_ASSERT_MESSAGE("Strict-mode contract is complete for the synthetic input",
                           Helper::Validate(seg).empty());

    mitk::IFileWriter::Options options;
    const auto path = this->WriteSegToTempFile(seg, options, "property-driven");

    const auto loaded = mitk::IOUtil::Load(path);
    CPPUNIT_ASSERT_MESSAGE("Round-trip load produced exactly one BaseData",
                           loaded.size() == 1);
    const auto loadedSeg = dynamic_cast<mitk::MultiLabelSegmentation*>(loaded[0].GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Round-trip load produced a MultiLabelSegmentation",
                           loadedSeg != nullptr);
  }

  // Regression guard for the dcmqi writer's skipEmptySlices flag, which the
  // writer must keep at false so every source slice is encoded. Build a
  // 3-slice seg whose single label occupies only the middle slice, leaving
  // slices 0 and 2 empty. With skipEmptySlices=true the binary path would
  // trim the written range to the label bounding box (1 frame) and the
  // labelmap path would drop the empty frames; with false every slice is
  // encoded (3 frames). A voxel-level round trip cannot catch the
  // difference - a dropped empty slice reconstructs as empty - so assert
  // directly on the written NumberOfFrames.
  void WriterPreservesEmptySlicesInOutput()
  {
    auto seg = BuildBaseSeg(/*stampIdentity=*/true);

    // Replace the all-slices foreground from BuildBaseSeg with a single
    // labelled block on the middle slice only.
    {
      auto groupImage = seg->GetGroupImage(0);
      mitk::ImageWriteAccessor writeAccessor(groupImage);
      auto *pixels = static_cast<mitk::Label::PixelType *>(writeAccessor.GetData());
      const auto dims = groupImage->GetDimensions();
      const auto sliceSize = static_cast<std::size_t>(dims[0]) * dims[1];
      for (std::size_t i = 0; i < sliceSize * dims[2]; ++i)
        pixels[i] = 0;
      const unsigned int middleSlice = dims[2] / 2;  // z == 1 for the 3-slice fixture
      pixels[middleSlice * sliceSize + 0] = 1;
      pixels[middleSlice * sliceSize + 1] = 1;
    }

    // Synthetic mode so the write does not depend on a source relation;
    // default (binary) encoding so the historic skipEmptySlices=false
    // behavior is what is exercised.
    mitk::IFileWriter::Options options;
    options["Strict / synthetic mode"] = std::string("synthetic");
    const auto path = this->WriteSegToTempFile(seg, options, "empty-slice-preservation");

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Writer must encode every source slice (skipEmptySlices=false): all 3 slices, not just the "
      "label bounding box",
      3L, GetWrittenSEGNumberOfFrames(path));
  }

  void StrictModeRefusesIncompleteSegmentation()
  {
    auto seg = BuildSegWithRule(/*stampIdentity=*/false);
    CPPUNIT_ASSERT_MESSAGE("Test precondition: incomplete seg fails Validate",
                           !Helper::Validate(seg).empty());

    mitk::IFileWriter::Options options;  // strict by default
    CPPUNIT_ASSERT_THROW_MESSAGE("Strict mode must refuse an incomplete seg",
                                 this->WriteSegToTempFile(seg, options, "strict-fail"),
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
                                 this->WriteSegToTempFile(seg, options, "strict-no-relation"),
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
      this->WriteSegToTempFile(seg, options, "strict-id-layer"),
      mitk::Exception);
  }

  void SyntheticModeFillsMissingIdentityAndWrites()
  {
    auto seg = BuildSegWithRule(/*stampIdentity=*/false);

    mitk::IFileWriter::Options options;
    options["Strict / synthetic mode"] = std::string("synthetic");
    const auto path = this->WriteSegToTempFile(seg, options, "synthetic");

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
    const auto path = this->WriteSegToTempFile(seg, options, "synthetic-stub");

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
    const auto path = this->WriteSegToTempFile(seg, options, "synthetic-id-layer");

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
      this->WriteSegToTempFile(seg, options, "vrcheck"));
  }

  void DefaultEncodingProducesLegacySOPClass()
  {
    auto seg = BuildSegWithRule();

    mitk::IFileWriter::Options options;  // no encoding option set
    const auto path = this->WriteSegToTempFile(seg, options, "default");

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
    const auto path = this->WriteSegToTempFile(seg, options, "labelmap");

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
    const auto path = this->WriteSegToTempFile(seg, options, "labelmap-bg-roundtrip");

    // Write side: WriteSegToTempFile has checked every segment; dcmqi must have added Background
    // as segment 0.
    const auto segments = mitk::test::ReadSegmentSequence(path);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Labelmap SEG must carry the auto-added Background segment plus the user's one label",
      std::size_t{2}, segments.size());
    const auto category = segments.at(0)->GetSegmentedPropertyCategory();
    CPPUNIT_ASSERT_MESSAGE("Auto-added Background segment must carry a category", category.has_value());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Auto-added Background segment must carry DCM code value 125040",
                                 std::string("125040"), category->GetValue());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Auto-added Background segment must use the DCM coding scheme",
                                 std::string("DCM"), category->GetScheme());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Auto-added Background segment must be named 'Background'",
                                 std::string("Background"), category->GetMeaning());

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
    const auto path = this->WriteSegToTempFile(seg, options, "binary");

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
    const auto path = this->WriteSegToTempFile(seg, options, "multisource");
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

  // Tracking ID/UID are DICOM Type 1C: optional, but never present without
  // a value. A label with no tracking properties must write successfully in
  // strict mode and the resulting SEG must not carry (0062,0020)/(0062,0021)
  // for that segment; reloading the SEG must leave HasTrackingID/UID false on
  // the label. The by-value check reads an empty element as absent, so
  // absence is asserted on the raw items.
  //
  // Default encoding is binary Segmentation Storage; segment 1 is the
  // user's label "L" (no auto-inserted Background segment, unlike dcmqi's
  // labelmap path).
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
    const auto path = this->WriteSegToTempFile(seg, options, "no-tracking");

    DcmFileFormat file;
    CPPUNIT_ASSERT_MESSAGE("DCMTK reads the written SEG", file.loadFile(path.c_str()).good());
    for (auto* segment : mitk::test::detail::GetSequenceItems(*file.getDataset(), DCM_SegmentSequence))
    {
      CPPUNIT_ASSERT_MESSAGE("Written SEG must omit (0062,0020) when source label had no tracking",
                             !segment->tagExists(DCM_TrackingID));
      CPPUNIT_ASSERT_MESSAGE("Written SEG must omit (0062,0021) when source label had no tracking",
                             !segment->tagExists(DCM_TrackingUID));
    }

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
  // The constructor mints a stable seg-own SeriesInstanceUID, and the writer
  // copies that (0020,000E) onto the output file (overriding dcmqi's
  // internally-minted UID). So the on-disk SeriesInstanceUID matches the
  // seg's in-memory property and is reproduced verbatim on every write.
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
    const auto path = this->WriteSegToTempFile(seg, options, "wf-strict");

    // Strict mode only guarantees the write found source items at all. That
    // they are the source's own instances is what makes the written
    // provenance true rather than merely present.
    AssertSourceReferencesAreExactly(path, source, "Strict");

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
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Written SEG SeriesInstanceUID matches the seg's in-memory (0020,000E)",
                                 segSeriesUID, writtenSeriesUID);

    // Single-group output is one series, one instance: SeriesNumber "1",
    // InstanceNumber "1" (the multi-group case varies InstanceNumber by
    // group). IS VR even-length space padding is stripped by
    // findAndGetOFString, so the exact-string compare is robust.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Single-group written SeriesNumber is the aligned constant",
                                 std::string("1"), ReadTopLevelString(path, DCM_SeriesNumber));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Single-group written InstanceNumber is 1",
                                 std::string("1"), ReadTopLevelString(path, DCM_InstanceNumber));

    // Writing the same in-memory seg again reproduces the identical
    // SeriesInstanceUID: the writer copies (0020,000E) verbatim, it is not
    // re-minted per write. Pins the construction-time mint; it is not an
    // endorsement of re-writing into the same series as a workflow.
    const auto secondPath = this->WriteSegToTempFile(seg, options, "wf-strict-2");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Re-writing the same in-memory seg reproduces the SeriesInstanceUID",
                                 writtenSeriesUID, ReadTopLevelString(secondPath, DCM_SeriesInstanceUID));
  }

  // Synthetic-mode counterpart of the workflow case above. Even with a
  // real DICOM source attached via Initialize, the user can opt into
  // synthetic mode. The write must succeed and the written SEG must carry
  // the seg's own (0020,000E): propagation applies in synthetic mode too,
  // and the synthetic source-series UID minted internally is a separate
  // value that does not leak into the SEG's own series identity.
  void SyntheticWriteSucceedsForInitializedFromDICOMSource()
  {
    auto source = mitk::test::BuildSourceImageWithDICOMIdentity();
    auto seg = InitializeSegFromSourceImage(source);
    Rule::Connect(seg, source.GetPointer());

    mitk::IFileWriter::Options options;
    options["Strict / synthetic mode"] = std::string("synthetic");
    const auto path = this->WriteSegToTempFile(seg, options, "wf-synth");

    // Synthetic is a fallback, not an override: with a real relation attached
    // the writer must not reach BuildSyntheticSourceItems, so the references
    // stay the source's own rather than freshly minted ones.
    AssertSourceReferencesAreExactly(path, source, "Synthetic");

    const auto writtenSeriesUID = ReadTopLevelString(path, DCM_SeriesInstanceUID);
    CPPUNIT_ASSERT_MESSAGE("Synthetic-mode write produces a non-empty SeriesInstanceUID",
                           !writtenSeriesUID.empty());
    const auto sourceSeriesUID = source->GetConstProperty(DICOMKey(0x0020, 0x000e))->GetValueAsString();
    CPPUNIT_ASSERT_MESSAGE("Synthetic-mode write SeriesInstanceUID differs from source's series UID",
                           writtenSeriesUID != sourceSeriesUID);

    const auto segSeriesUID = ReadStringProp(seg, 0x0020, 0x000E);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Synthetic-mode written SeriesInstanceUID matches the seg's in-memory (0020,000E)",
                                 segSeriesUID, writtenSeriesUID);
  }

  // Round-trip via the DICOM SEG reader: write a workflow seg in strict
  // mode, load it back, save again in strict mode. The reloaded seg
  // must carry every IOD-mandated identity tag - in particular
  // FrameOfReferenceUID, which only lands on the loaded seg's property
  // list when (0020,0052) is in the default DICOMTagsOfInterest.
  // SeriesInstanceUID is preserved across persist -> reload -> re-persist:
  // the writer copies the seg's (0020,000E) onto the file, the reader
  // repopulates it on load, and the re-save copies it back unchanged.
  void StrictReSaveAfterDICOMSegRoundTripPreservesIdentity()
  {
    auto source = mitk::test::BuildSourceImageWithDICOMIdentity();
    auto seg = InitializeSegFromSourceImage(source);
    Rule::Connect(seg, source.GetPointer());

    mitk::IFileWriter::Options options;
    const auto firstPath = this->WriteSegToTempFile(seg, options, "wf-roundtrip-1");
    const auto firstSeriesUID = ReadTopLevelString(firstPath, DCM_SeriesInstanceUID);

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

    // The reader repopulates (0020,000E) from the file (it is in the default
    // DICOMTagsOfInterest), so the reloaded seg carries the first file's
    // series UID, and re-saving copies it back unchanged.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Reloaded seg carries the first file's SeriesInstanceUID",
                                 firstSeriesUID, ReadStringProp(reloadedSeg, 0x0020, 0x000E));

    const auto secondPath = this->WriteSegToTempFile(reloadedSeg, options, "wf-roundtrip-2");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SeriesInstanceUID stable across persist -> reload -> re-persist",
                                 firstSeriesUID, ReadTopLevelString(secondPath, DCM_SeriesInstanceUID));
  }

  // A multi-group seg's per-group SEG files must form ONE DICOM series:
  // shared SeriesInstanceUID (== the seg's own (0020,000E)), identical
  // SeriesNumber, distinct per-group InstanceNumber. This pins the
  // writer-side groundwork for representing a multi-group seg on the wire.
  // Reader-side reassembly into one MultiLabelSegmentation is NOT asserted
  // (tracked separately); we read the two files back as DcmDatasets and
  // compare tags directly. WriteSegToTempFile checks the segments of each
  // file.
  void MultiGroupWriteFormsOneCoherentSeries()
  {
    auto seg = BuildTwoGroupSeg();
    CPPUNIT_ASSERT_MESSAGE("Seg is multi-group for this test", seg->GetNumberOfGroups() == 2);

    const auto segSeriesUID = ReadStringProp(seg, 0x0020, 0x000E);

    mitk::IFileWriter::Options options;
    const auto basePath = this->WriteSegToTempFile(seg, options, "wf-multigroup");

    // Assert existence before reading: ReadTopLevelString returns "" for a
    // missing file, which would otherwise mask a wrong-path bug as a
    // propagation bug.
    const auto file0 = GroupFilePath(basePath, 0); // == basePath; first group keeps the requested name
    const auto file1 = GroupFilePath(basePath, 1); // additional groups get the "_<index>" suffix
    CPPUNIT_ASSERT_MESSAGE("Both per-group SEG files exist", FileExists(file0) && FileExists(file1));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Group 0 file SeriesInstanceUID == seg's own",
                                 segSeriesUID, ReadTopLevelString(file0, DCM_SeriesInstanceUID));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Group 1 file SeriesInstanceUID == seg's own",
                                 segSeriesUID, ReadTopLevelString(file1, DCM_SeriesInstanceUID));
    // One series: identical, aligned SeriesNumber across the per-group files.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Group 0 SeriesNumber is the aligned constant",
                                 std::string("1"), ReadTopLevelString(file0, DCM_SeriesNumber));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Group 1 SeriesNumber is the aligned constant",
                                 std::string("1"), ReadTopLevelString(file1, DCM_SeriesNumber));
    // Distinct, 1-based InstanceNumbers (= group index + 1) so the two
    // instances are individually addressable within the shared series. IS VR
    // even-length space padding is stripped by findAndGetOFString.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Group 0 InstanceNumber is 1",
                                 std::string("1"), ReadTopLevelString(file0, DCM_InstanceNumber));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Group 1 InstanceNumber is 2",
                                 std::string("2"), ReadTopLevelString(file1, DCM_InstanceNumber));
    // Series-level date/time must also agree across the files: dcmqi stamps
    // them per call from the wall clock, so the writer reuses the first
    // group's values for the rest.
    // Capture group 0's values and assert they are present before comparing:
    // ReadTopLevelString returns "" for a missing tag, so without this guard a
    // "" == "" equality would pass vacuously if dcmqi stopped emitting these
    // tags or the writer's alignment block were removed.
    const auto seriesDate0 = ReadTopLevelString(file0, DCM_SeriesDate);
    const auto seriesTime0 = ReadTopLevelString(file0, DCM_SeriesTime);
    CPPUNIT_ASSERT_MESSAGE("SeriesDate is present (guards the equality below from passing vacuously)",
                           !seriesDate0.empty());
    CPPUNIT_ASSERT_MESSAGE("SeriesTime is present (guards the equality below from passing vacuously)",
                           !seriesTime0.empty());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Both group files share SeriesDate",
                                 seriesDate0, ReadTopLevelString(file1, DCM_SeriesDate));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Both group files share SeriesTime",
                                 seriesTime0, ReadTopLevelString(file1, DCM_SeriesTime));
  }

  // Regression pin for the GetStringProperty -> GetDICOMPropertyValue fix in
  // BuildMetaInfoHandler. ContentCreatorName and the Clinical Trial tags are
  // stored as TemporoSpatialStringProperty (the type the reader and
  // StampIdentityTags produce). The old GetStringProperty read silently
  // missed that type, so a real ContentCreatorName was overwritten with
  // "MITK" and user-set Clinical Trial tags were dropped on write - and no
  // test caught it. Here every one of them must round-trip into the file
  // verbatim. ContentCreatorName deliberately uses a non-"MITK" value: the
  // old fallback default was also "MITK", which masked the bug.
  void WriteEmitsContentCreatorAndClinicalTrialTagsWhenSet()
  {
    auto seg = BuildSegWithRule(/*stampIdentity=*/true, "1.2.826.0.1.3680043.10.999.8.1");
    seg->SetProperty(DICOMKey(0x0070, 0x0084).c_str(),
                     mitk::TemporoSpatialStringProperty::New("Test^Creator"));

    mitk::IFileWriter::Options options;
    const auto path = this->WriteSegToTempFile(seg, options, "wf-clinicaltrial");

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Non-default ContentCreatorName must survive into the written SEG",
                                 std::string("Test^Creator"), ReadTopLevelString(path, DCM_ContentCreatorName));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("User-set ClinicalTrialSeriesID must be emitted",
                                 std::string("Session A"), ReadTopLevelString(path, DCM_ClinicalTrialSeriesID));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("User-set ClinicalTrialTimePointID must be emitted",
                                 std::string("0"), ReadTopLevelString(path, DCM_ClinicalTrialTimePointID));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("User-set ClinicalTrialCoordinatingCenterName must be emitted",
                                 std::string("CenterA"),
                                 ReadTopLevelString(path, DCM_ClinicalTrialCoordinatingCenterName));
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
    const auto path = this->WriteSegToTempFile(seg, options, "no-segproperty");

    // Default encoding is binary; the user's label "L" is segment 1 (binary
    // has no auto-inserted Background segment). The literal codes pin the
    // stand-ins independently of the expectation WriteSegToTempFile derives.
    const auto segment = mitk::test::ReadSegmentSequence(path).at(1);
    const auto category = segment->GetSegmentedPropertyCategory();
    const auto type = segment->GetSegmentedPropertyType();
    CPPUNIT_ASSERT_MESSAGE("The written segment has a category and a type", category.has_value() && type.has_value());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fallback Category code value", std::string("49755003"), category->GetValue());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fallback Category coding scheme", std::string("SCT"), category->GetScheme());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fallback Category code meaning",
                                 std::string("Morphologically altered structure"), category->GetMeaning());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fallback Type code value", std::string("49755003"), type->GetValue());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fallback Type coding scheme", std::string("SCT"), type->GetScheme());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fallback Type code meaning",
                                 std::string("Morphologically altered structure"), type->GetMeaning());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("One fallback Type modifier", std::size_t{1}, type->GetModifierCount());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fallback Type modifier code value",
                                 std::string("261665006"), type->GetModifier(0).GetValue());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fallback Type modifier coding scheme",
                                 std::string("SCT"), type->GetModifier(0).GetScheme());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fallback Type modifier code meaning",
                                 std::string("Unknown (qualifier value)"), type->GetModifier(0).GetMeaning());
  }

  // The writer writes a code only if its value, scheme and meaning are all set: an incomplete
  // category or type gets the stand-in, like a missing one, and an incomplete first type modifier is
  // left out. A label without a name is then named after the stand-in type. WriteSegToTempFile checks
  // the expectation it derives; the literal values pin the writer independently of it.
  void WriteReplacesIncompleteCodesByStandIns()
  {
    struct Case
    {
      std::string nameHint;
      std::function<void(mitk::Label&)> edit;
      std::string segmentLabel;
      std::string categoryValue;
      std::string typeValue;
      std::string modifierValue; // empty if no modifier is written
    };
    const std::vector<Case> cases = {
      { "incomplete-type-of-unnamed",
        [](mitk::Label& label) {
          label.SetName("");
          label.SetSegmentedPropertyType(mitk::DICOMCodeSequenceWithModifiers("", "SCT", "Liver"));
        },
        "Morphologically altered structure", "T-D0050", "49755003", "261665006" },
      { "incomplete-type-with-modifier",
        [](mitk::Label& label) {
          mitk::DICOMCodeSequenceWithModifiers type("", "SCT", "Liver");
          type.AddModifier(mitk::DICOMCodeSequence("24028007", "SCT", "Right"));
          label.SetSegmentedPropertyType(type);
        },
        "L", "T-D0050", "49755003", "24028007" },
      { "incomplete-category",
        [](mitk::Label& label) { label.SetSegmentedPropertyCategory(mitk::DICOMCodeSequence("C1", "", "Category")); },
        "L", "49755003", "T-D0050", "" },
      { "incomplete-type-modifier",
        [](mitk::Label& label) {
          mitk::DICOMCodeSequenceWithModifiers type("T-D0050", "SRT", "Tissue");
          type.AddModifier(mitk::DICOMCodeSequence("", "SCT", "Right"));
          label.SetSegmentedPropertyType(type);
        },
        "L", "T-D0050", "T-D0050", "" },
    };

    for (const auto& c : cases)
    {
      auto seg = BuildSegWithRule();
      c.edit(*seg->GetLabel(1));
      const auto path = this->WriteSegToTempFile(seg, {}, c.nameHint);

      const auto segment = mitk::test::ReadSegmentSequence(path).at(1);
      const auto category = segment->GetSegmentedPropertyCategory();
      const auto type = segment->GetSegmentedPropertyType();
      CPPUNIT_ASSERT_MESSAGE(c.nameHint + ": the written segment has a category and a type",
                             category.has_value() && type.has_value());
      CPPUNIT_ASSERT_EQUAL_MESSAGE(c.nameHint + ": Segment Label", c.segmentLabel, segment->GetName());
      CPPUNIT_ASSERT_EQUAL_MESSAGE(c.nameHint + ": category code value", c.categoryValue, category->GetValue());
      CPPUNIT_ASSERT_EQUAL_MESSAGE(c.nameHint + ": type code value", c.typeValue, type->GetValue());
      if (c.modifierValue.empty())
      {
        CPPUNIT_ASSERT_EQUAL_MESSAGE(c.nameHint + ": no type modifier", std::size_t{0}, type->GetModifierCount());
      }
      else
      {
        CPPUNIT_ASSERT_EQUAL_MESSAGE(c.nameHint + ": one type modifier", std::size_t{1}, type->GetModifierCount());
        CPPUNIT_ASSERT_EQUAL_MESSAGE(c.nameHint + ": type modifier code value", c.modifierValue,
                                     type->GetModifier(0).GetValue());
      }
    }
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkDICOMSegmentationIOWriter)
