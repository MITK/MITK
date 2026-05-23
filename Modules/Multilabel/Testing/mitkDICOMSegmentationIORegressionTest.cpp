/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkIOUtil.h>
#include <mitkLabelSetImage.h>
#include <mitkSegSourceImageRelationRule.h>
#include <mitkTemporoSpatialStringProperty.h>
#include <mitkTestingMacros.h>

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

  MITK_TEST_END();
}
