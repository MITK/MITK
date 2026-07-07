/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkBaseGeometry.h>
#include <mitkIOUtil.h>
#include <mitkImage.h>
#include <mitkLabelSetImage.h>
#include <mitkSegSourceImageRelationRule.h>
#include <mitkTemporoSpatialStringProperty.h>
#include <mitkTestingMacros.h>

#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcitem.h>
#include <dcmtk/dcmdata/dcsequen.h>
#include <dcmtk/dcmdata/dcvr.h>
#include <dcmtk/ofstd/ofcond.h>
#include <dcmtk/ofstd/ofstring.h>

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <set>
#include <string>
#include <vector>

// Regression guard for per-frame source-image keying: a DICOM SEG whose
// per-frame functional groups are not stored in physical-Z order must still associate
// each geometry slice with the source image whose content dcmqi placed at that
// slice (resolved from the frame's ImagePositionPatient), not with the source
// of the frame at the same ordinal.
//
// There is no checked-in non-Z-ordered fixture, and neither dcmqi nor the MITK
// writer can produce one (both normalise frames to Z order), so the test
// manufactures one in-memory: it loads the Z-ordered dcmqi liver.dcm and
// reverses its PerFrameFunctionalGroupsSequence. Each item carries its own
// Plane Position and Derivation Image, so reversing keeps every frame's
// IPP<->source pairing intact and only changes the frame ordinal order; the
// PixelData is left untouched (cardinality unchanged) so the SEG stays
// loadable. Correct (geometry-keyed) loading therefore reproduces the original
// slice->source mapping; ordinal-keyed loading produces the reversed mapping.
// The test asserts only the per-slice source mapping, the slice coverage, and
// the geometry origin convention - never voxel content, which is intentionally
// decoupled from the relabeled IPPs by this construction.

namespace
{
  struct FrameInfo
  {
    mitk::Point3D ipp;
    std::string source;
  };

  // Read each frame's ImagePositionPatient and Derivation-Image source
  // SOPInstanceUID straight from the file's PerFrameFunctionalGroupsSequence,
  // independently of the production parser, so the oracle grades the loaded
  // relation against the file rather than against the code under test.
  std::vector<FrameInfo> ReadFramesInfo(const std::string &path)
  {
    std::vector<FrameInfo> frames;

    DcmFileFormat ff;
    if (ff.loadFile(path.c_str()).bad() || ff.getDataset() == nullptr)
      return frames;

    DcmSequenceOfItems *perFrame = nullptr;
    if (ff.getDataset()->findAndGetSequence(DCM_PerFrameFunctionalGroupsSequence, perFrame).bad()
        || perFrame == nullptr)
      return frames;

    for (unsigned long i = 0; i < perFrame->card(); ++i)
    {
      DcmItem *frame = perFrame->getItem(i);
      if (frame == nullptr)
        continue;

      FrameInfo info;
      info.ipp.Fill(0.0);

      DcmItem *planePos = nullptr;
      if (frame->findAndGetSequenceItem(DCM_PlanePositionSequence, planePos, 0).good() && planePos != nullptr)
      {
        for (unsigned long j = 0; j < 3; ++j)
        {
          OFString component;
          if (planePos->findAndGetOFString(DCM_ImagePositionPatient, component, j).good())
            info.ipp[j] = std::atof(component.c_str());
        }
      }

      DcmItem *derImg = nullptr;
      if (frame->findAndGetSequenceItem(DCM_DerivationImageSequence, derImg, 0).good() && derImg != nullptr)
      {
        DcmItem *srcImg = nullptr;
        if (derImg->findAndGetSequenceItem(DCM_SourceImageSequence, srcImg, 0).good() && srcImg != nullptr)
        {
          OFString sop;
          if (srcImg->findAndGetOFString(DCM_ReferencedSOPInstanceUID, sop).good())
            info.source = sop.c_str();
        }
      }

      frames.push_back(info);
    }

    return frames;
  }
}

int mitkDICOMSegmentationIONonZOrderedTest(int argc, char *argv[])
{
  MITK_TEST_BEGIN("DICOMSegmentationIONonZOrdered");

  MITK_TEST_CONDITION_REQUIRED(argc == 2, "Expected arguments: <dicom-seg-fixture>");
  const std::string fixturePath = argv[1];

  // 1. Load the Z-ordered fixture and reverse its per-frame functional groups.
  DcmFileFormat ff;
  MITK_TEST_CONDITION_REQUIRED(ff.loadFile(fixturePath.c_str()).good(),
    "Loaded the DICOM SEG fixture");
  DcmDataset *dataset = ff.getDataset();
  MITK_TEST_CONDITION_REQUIRED(dataset != nullptr, "Fixture has a dataset");

  DcmSequenceOfItems *perFrame = nullptr;
  MITK_TEST_CONDITION_REQUIRED(
    dataset->findAndGetSequence(DCM_PerFrameFunctionalGroupsSequence, perFrame).good()
      && perFrame != nullptr,
    "Fixture has a PerFrameFunctionalGroupsSequence");

  const unsigned long numFrames = perFrame->card();
  MITK_TEST_CONDITION_REQUIRED(numFrames >= 2,
    "Fixture has at least two frames (required to be non-trivially non-Z-ordered)");

  // remove() hands ownership to the caller; prepend() takes it back. Pulling
  // every item from the front in order and prepending each leaves the list
  // reversed, with no item deleted in between.
  {
    std::vector<DcmItem *> items;
    items.reserve(numFrames);
    for (unsigned long i = 0; i < numFrames; ++i)
      items.push_back(perFrame->remove(0UL));
    for (DcmItem *item : items)
    {
      MITK_TEST_CONDITION_REQUIRED(item != nullptr, "Removed per-frame item is non-null");
      perFrame->prepend(item);
    }
  }

  const auto tempDir = std::filesystem::temp_directory_path() / "mitkDICOMSegNonZOrdered";
  std::filesystem::create_directories(tempDir);
  const auto reversedPath = (tempDir / "reversed.dcm").string();
  MITK_TEST_CONDITION_REQUIRED(
    ff.saveFile(reversedPath.c_str(), EXS_LittleEndianExplicit).good(),
    "Wrote the reversed temp SEG");

  // 2. Build the oracle from the reversed file: slice k (geometry index) holds
  //    the source of the k-th-smallest-Z frame. liver.dcm is axial +Z with
  //    contiguous slices, so the geometry slice index equals the Z-ascending
  //    rank; sorting the frames by Z yields the expected per-slice source.
  auto frames = ReadFramesInfo(reversedPath);
  MITK_TEST_CONDITION_REQUIRED(frames.size() == numFrames,
    "Read every per-frame (IPP, source) pair back from the reversed file");
  for (const auto &frame : frames)
  {
    MITK_TEST_CONDITION_REQUIRED(!frame.source.empty(),
      "Each frame declares a Derivation-Image source SOPInstanceUID");
  }

  std::sort(frames.begin(), frames.end(),
    [](const FrameInfo &a, const FrameInfo &b) { return a.ipp[2] < b.ipp[2]; });

  // 3. Load via MITK and grade the populated source-image relation.
  const auto loaded = mitk::IOUtil::Load(reversedPath);
  MITK_TEST_CONDITION_REQUIRED(loaded.size() == 1,
    "Loader produced exactly one result for the reversed SEG");
  auto *seg = dynamic_cast<mitk::MultiLabelSegmentation *>(loaded[0].GetPointer());
  MITK_TEST_CONDITION_REQUIRED(seg != nullptr, "Loaded data is a MultiLabelSegmentation");

  const auto relations = mitk::SegSourceImageRelationRule::GetSourceImageRelations(seg);
  MITK_TEST_CONDITION_REQUIRED(relations.size() == 1,
    "Reversed SEG carries exactly one source-image relation (single source series)");
  const auto &relation = relations.front();
  MITK_TEST_CONDITION_REQUIRED(relation.instanceUIDsPerSlice.IsNotNull(),
    "Per-slice source SOPInstance property is set on the relation");

  const auto availableVec = relation.instanceUIDsPerSlice->GetAvailableSlices(0);
  const std::set<mitk::TemporoSpatialStringProperty::IndexValueType> availableSlices(
    availableVec.begin(), availableVec.end());
  std::set<mitk::TemporoSpatialStringProperty::IndexValueType> expectedSlices;
  for (unsigned long k = 0; k < numFrames; ++k)
    expectedSlices.insert(static_cast<mitk::TemporoSpatialStringProperty::IndexValueType>(k));
  MITK_TEST_CONDITION(availableSlices == expectedSlices,
    "Resolved slice set is exactly {0 .. N-1} (every frame mapped to a distinct geometry slice)");

  bool allSourcesMatch = true;
  for (unsigned long k = 0; k < numFrames; ++k)
  {
    const auto slice = static_cast<mitk::TemporoSpatialStringProperty::IndexValueType>(k);
    if (relation.instanceUIDsPerSlice->GetValue(0, slice) != frames[k].source)
    {
      allSourcesMatch = false;
      break;
    }
  }
  // The discriminating assertion: under ordinal keying (the pre-fix bug) slice 0
  // would carry the reversed frame's source, not the min-Z frame's, so this
  // fails on the pre-fix code.
  MITK_TEST_CONDITION(allSourcesMatch,
    "Each geometry slice's source SOPInstanceUID matches the frame whose IPP maps to that slice");

  // 4. Convention guard (independent of the keying fix): the reconstructed group
  //    geometry's voxel (0,0,0) centre coincides with the min-Z frame's raw IPP,
  //    i.e. plane origins are centre-based and equal to DICOM ImagePositionPatient.
  const auto *groupImage = seg->GetGroupImage(0);
  MITK_TEST_CONDITION_REQUIRED(groupImage != nullptr && groupImage->GetGeometry() != nullptr,
    "Loaded seg has a group-0 image geometry");
  mitk::Point3D indexOrigin;
  indexOrigin.Fill(0.0);
  mitk::Point3D worldOrigin;
  groupImage->GetGeometry()->IndexToWorld(indexOrigin, worldOrigin);
  const mitk::Point3D minZIpp = frames.front().ipp;
  bool originMatches = true;
  for (unsigned int d = 0; d < 3; ++d)
    if (std::abs(worldOrigin[d] - minZIpp[d]) > 1e-3)
      originMatches = false;
  MITK_TEST_CONDITION(originMatches,
    "Group geometry IndexToWorld({0,0,0}) equals the min-Z frame's IPP (centre-based convention)");

  MITK_TEST_END();
}
