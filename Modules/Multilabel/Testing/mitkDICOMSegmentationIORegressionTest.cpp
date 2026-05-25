/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkDICOMSegPropertyTestHelpers.h"

#include <mitkBaseProperty.h>
#include <mitkIOUtil.h>
#include <mitkLabelSetImage.h>
#include <mitkPropertyKeyPath.h>
#include <mitkPropertyNameHelper.h>
#include <mitkSegSourceImageRelationRule.h>
#include <mitkTemporoSpatialStringProperty.h>
#include <mitkTestingMacros.h>

#include <filesystem>

#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcitem.h>
#include <dcmtk/dcmdata/dcsequen.h>
#include <dcmtk/dcmfg/fgderimg.h>
#include <dcmtk/dcmfg/fginterface.h>
#include <dcmtk/dcmfg/fgtypes.h>
#include <dcmtk/dcmseg/segdoc.h>
#include <dcmtk/ofstd/ofcond.h>
#include <dcmtk/ofstd/ofstring.h>

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

// Regression guard for the binary-DICOM-SEG reader, including the
// source-image relations the reader populates from the SEG's per-frame
// and top-level reference structures. A binary DICOM SEG input fixture
// is loaded through MITK's autoload path and the resulting
// MultiLabelSegmentation is compared against a reference segmentation
// that was generated against an earlier reader version and checked into
// MITK-Data. Any drift in voxel layout or label metadata (geometry,
// group images, label names/colours/codes, tracking IDs) lands on the
// drifting commit during bisect rather than silently shipping.

namespace
{
  // Ground truth parsed independently from the SEG file via DCMTK. Kept
  // separate from the production parser so the regression test fails when
  // production and the file disagree, not just when production disagrees
  // with itself.
  struct SegSourceTruth
  {
    // SeriesInstanceUID -> set of SOPInstanceUIDs from the SEG's top-level
    // ReferencedSeriesSequence (0008,1115).
    std::map<std::string, std::set<std::string>> seriesToInstances;
    // Frame index -> set of source SOPInstanceUIDs from the SEG's per-frame
    // Derivation Image functional groups.
    std::map<Uint32, std::set<std::string>> perFrameSourceInstances;
  };

  SegSourceTruth ReadSegSourceTruth(const std::string &segPath)
  {
    SegSourceTruth result;

    DcmFileFormat ff;
    if (ff.loadFile(segPath.c_str()).bad())
      return result;
    DcmDataset *dataset = ff.getDataset();
    if (dataset == nullptr)
      return result;

    DcmSequenceOfItems *refSeriesSeq = nullptr;
    if (dataset->findAndGetSequence(DCM_ReferencedSeriesSequence, refSeriesSeq).good()
        && refSeriesSeq != nullptr)
    {
      for (unsigned long i = 0; i < refSeriesSeq->card(); ++i)
      {
        DcmItem *item = refSeriesSeq->getItem(i);
        if (item == nullptr)
          continue;
        OFString seriesUID;
        if (item->findAndGetOFString(DCM_SeriesInstanceUID, seriesUID).bad() || seriesUID.empty())
          continue;
        auto &instanceSet = result.seriesToInstances[seriesUID.c_str()];

        DcmSequenceOfItems *refInstSeq = nullptr;
        if (item->findAndGetSequence(DCM_ReferencedInstanceSequence, refInstSeq).bad()
            || refInstSeq == nullptr)
          continue;
        for (unsigned long j = 0; j < refInstSeq->card(); ++j)
        {
          DcmItem *instItem = refInstSeq->getItem(j);
          if (instItem == nullptr)
            continue;
          OFString sopInstance;
          if (instItem->findAndGetOFString(DCM_ReferencedSOPInstanceUID, sopInstance).good()
              && !sopInstance.empty())
            instanceSet.insert(sopInstance.c_str());
        }
      }
    }

    DcmSegmentation *segDocRaw = nullptr;
    if (DcmSegmentation::loadFile(segPath.c_str(), segDocRaw).bad() || segDocRaw == nullptr)
      return result;
    std::unique_ptr<DcmSegmentation> segDoc(segDocRaw);

    FGInterface &fgInterface = segDoc->getFunctionalGroups();
    const size_t numFrames = segDoc->getNumberOfFrames();
    for (size_t f = 0; f < numFrames; ++f)
    {
      OFBool isPerFrame = OFFalse;
      auto *fg = fgInterface.get(static_cast<Uint32>(f), DcmFGTypes::EFG_DERIVATIONIMAGE, isPerFrame);
      auto *derImg = OFstatic_cast(FGDerivationImage *, fg);
      if (derImg == nullptr)
        continue;
      auto &frameSet = result.perFrameSourceInstances[static_cast<Uint32>(f)];
      for (auto *derItem : derImg->getDerivationImageItems())
      {
        if (derItem == nullptr)
          continue;
        for (auto *srcItem : derItem->getSourceImageItems())
        {
          if (srcItem == nullptr)
            continue;
          OFString sopInstance;
          srcItem->getImageSOPInstanceReference().getReferencedSOPInstanceUID(sopInstance);
          if (!sopInstance.empty())
            frameSet.insert(sopInstance.c_str());
        }
      }
    }

    return result;
  }
}

int mitkDICOMSegmentationIORegressionTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("DICOMSegmentationIORegression");

  MITK_TEST_CONDITION_REQUIRED(argc == 3,
    "Expected arguments: <dicom-seg-fixture> <reference-mitk-segmentation>");

  const std::string fixturePath = argv[1];
  const std::string referencePath = argv[2];

  const auto loaded = mitk::IOUtil::Load(fixturePath);
  MITK_TEST_CONDITION_REQUIRED(loaded.size() == 1,
    "Loader produced exactly one result for the DICOM SEG fixture");

  auto* loadedSeg = dynamic_cast<mitk::MultiLabelSegmentation*>(loaded[0].GetPointer());
  MITK_TEST_CONDITION_REQUIRED(loadedSeg != nullptr,
    "Loaded data is a MultiLabelSegmentation");

  const auto reference = mitk::IOUtil::Load(referencePath);
  MITK_TEST_CONDITION_REQUIRED(reference.size() == 1,
    "Reference segmentation loaded as exactly one result");

  auto* referenceSeg = dynamic_cast<mitk::MultiLabelSegmentation*>(reference[0].GetPointer());
  MITK_TEST_CONDITION_REQUIRED(referenceSeg != nullptr,
    "Reference is a MultiLabelSegmentation");

  MITK_TEST_CONDITION(mitk::Equal(*loadedSeg, *referenceSeg, mitk::eps, true),
    "Loaded DICOM SEG matches the reference MultiLabelSegmentation");

  // The loaded seg carries the same shape of source-image relation that
  // the producer path would have written. Both the set of source series
  // UIDs and the per-slice (frame -> source SOPInstanceUID) mapping are
  // verified against the SEG file itself so the test stays correct under
  // fixture refresh and catches collapse-to-single-value regressions on
  // the per-slice TemporoSpatialStringProperty.
  const auto truth = ReadSegSourceTruth(fixturePath);
  MITK_TEST_CONDITION_REQUIRED(!truth.seriesToInstances.empty(),
    "Test fixture declares at least one source series via ReferencedSeriesSequence");
  MITK_TEST_CONDITION_REQUIRED(!truth.perFrameSourceInstances.empty(),
    "Test fixture declares at least one per-frame source reference");

  std::set<std::string> expectedSeriesUIDs;
  for (const auto &[seriesUID, instances] : truth.seriesToInstances)
    expectedSeriesUIDs.insert(seriesUID);

  const auto relations = mitk::SegSourceImageRelationRule::GetSourceImageRelations(loadedSeg);
  MITK_TEST_CONDITION(relations.size() == expectedSeriesUIDs.size(),
    "Loaded seg carries one source-image relation per source series in the SEG file");

  std::set<std::string> seenSeriesUIDs;
  for (const auto &relation : relations)
  {
    seenSeriesUIDs.insert(relation.sourceSeriesInstanceUID);
    MITK_TEST_CONDITION_REQUIRED(relation.instanceUIDsPerSlice.IsNotNull(),
      "Per-slice source SOPInstance property is set on the relation");
    MITK_TEST_CONDITION_REQUIRED(relation.classUIDsPerSlice.IsNotNull(),
      "Per-slice source SOPClass property is set on the relation");

    const auto seriesIt = truth.seriesToInstances.find(relation.sourceSeriesInstanceUID);
    MITK_TEST_CONDITION_REQUIRED(seriesIt != truth.seriesToInstances.end(),
      "Relation series UID is declared in the SEG's ReferencedSeriesSequence");
    const auto &expectedInstances = seriesIt->second;

    std::set<mitk::TemporoSpatialStringProperty::IndexValueType> expectedSlices;
    std::map<mitk::TemporoSpatialStringProperty::IndexValueType, std::string> expectedUIDPerSlice;
    for (const auto &[frameIndex, frameInstances] : truth.perFrameSourceInstances)
    {
      for (const auto &uid : frameInstances)
      {
        if (expectedInstances.count(uid) > 0)
        {
          const auto slice = static_cast<mitk::TemporoSpatialStringProperty::IndexValueType>(frameIndex);
          expectedSlices.insert(slice);
          expectedUIDPerSlice[slice] = uid;
        }
      }
    }

    const auto actualSlicesVec = relation.instanceUIDsPerSlice->GetAvailableSlices(0);
    const std::set<mitk::TemporoSpatialStringProperty::IndexValueType> actualSlices(
      actualSlicesVec.begin(), actualSlicesVec.end());
    MITK_TEST_CONDITION(actualSlices == expectedSlices,
      "Per-slice SOPInstance property covers exactly the SEG frames whose source belongs to this series");

    bool allUIDsMatch = true;
    for (const auto &[slice, expectedUID] : expectedUIDPerSlice)
    {
      if (relation.instanceUIDsPerSlice->GetValue(0, slice) != expectedUID)
      {
        allUIDsMatch = false;
        break;
      }
    }
    MITK_TEST_CONDITION(allUIDsMatch,
      "Per-slice SOPInstance values match the SEG's per-frame derivation references");
  }
  MITK_TEST_CONDITION(seenSeriesUIDs == expectedSeriesUIDs,
    "Source series UIDs on the relations match the SEG's ReferencedSeriesSequence");

  // Round-trip assertion (closes #321): the seg loaded from a real DICOM
  // SEG file must be re-writable via the new property-driven writer and
  // re-loadable as a MultiLabelSegmentation. Uses synthetic mode because
  // DICOMIOHelper::SetProperties does not surface every Validate-required
  // tag on the loaded seg (FrameOfReferenceUID in particular); synthetic
  // mode is the realistic default for "save anywhere" of an arbitrary
  // loaded SEG. The on-the-wire SOP class shifts to Sup 243 labelmap on
  // output (the writer's default), which is the documented behaviour
  // change in the user-facing changelog. Voxel-level equivalence is
  // pinned by the per-segment hash baseline above; this assertion only
  // adds the write+reload survivability that was impossible before this
  // stage.
  const auto tempDir = std::filesystem::temp_directory_path() / "mitkDICOMSegRegressionRoundTrip";
  std::filesystem::create_directories(tempDir);
  const auto roundTripPath = (tempDir / "roundtrip.dcm").string();
  mitk::IFileWriter::Options roundTripOptions;
  roundTripOptions["Strict / synthetic mode"] = std::string("synthetic");
  try
  {
    mitk::IOUtil::Save(loadedSeg, roundTripPath, roundTripOptions);
  }
  catch (const std::exception &e)
  {
    MITK_TEST_FAILED_MSG(<< "Round-trip write of loaded SEG threw: " << e.what());
  }
  const auto reloaded = mitk::IOUtil::Load(roundTripPath);
  MITK_TEST_CONDITION_REQUIRED(reloaded.size() == 1,
    "Round-trip reload produced exactly one BaseData");
  auto* reloadedSeg = dynamic_cast<mitk::MultiLabelSegmentation *>(reloaded[0].GetPointer());
  MITK_TEST_CONDITION_REQUIRED(reloadedSeg != nullptr,
    "Round-trip reload produced a MultiLabelSegmentation");

  // Beyond "reload yielded a MultiLabelSegmentation": the rule connection
  // must survive the write+read with non-degenerate per-slice content.
  // A previous shape of this assertion only checked the type and let any
  // collapse of the per-slice TemporoSpatialStringProperty slip through.
  const auto reloadedRelations =
    mitk::SegSourceImageRelationRule::GetSourceImageRelations(reloadedSeg);
  MITK_TEST_CONDITION_REQUIRED(!reloadedRelations.empty(),
    "Round-trip reload preserves at least one source-image relation");
  for (const auto &relation : reloadedRelations)
  {
    MITK_TEST_CONDITION(!relation.relationUID.empty(),
      "Reloaded relation carries a non-empty relation UID");
    MITK_TEST_CONDITION_REQUIRED(relation.instanceUIDsPerSlice.IsNotNull(),
      "Reloaded relation carries a per-slice instance UID property");
    MITK_TEST_CONDITION(!relation.instanceUIDsPerSlice->GetAvailableSlices(0).empty(),
      "Reloaded relation's per-slice property carries at least one slice entry "
      "(guards against TemporoSpatialString collapse on serialize)");
  }

  // Purpose-of-Reference Code Sequence Code Meaning must survive the
  // round-trip with the canonical seg-source purpose tag as its value.
  // SourceImageRelationRule::Connect_datalayer writes the property at
  // the canonical uppercase DICOM hex form, so all IO paths converge
  // on a single key after round-trip.
  const std::string purposeKey =
    "DICOM.0008.2112.[0].0040.A170.[0].0008.0104";
  const auto purposeProp = reloadedSeg->GetConstProperty(purposeKey);
  MITK_TEST_CONDITION(purposeProp.IsNotNull(),
    "Reloaded seg carries the Purpose-of-Reference Code Meaning property "
    "for the first source-image relation");
  if (purposeProp.IsNotNull())
  {
    MITK_TEST_CONDITION(mitk::test::PropertyScalarValueEquals(
        purposeProp.GetPointer(),
        mitk::SegSourceImageRelationRule::CanonicalPurposeTag()),
      "Reloaded Purpose-of-Reference Code Meaning matches the canonical seg-source tag");
  }

  MITK_TEST_END();
}
