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
#include <mitkProperties.h>
#include <mitkSegSourceImageRelationRule.h>
#include <mitkStringProperty.h>
#include <mitkTemporoSpatialStringProperty.h>
#include <mitkTestingMacros.h>

#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcitem.h>
#include <dcmtk/ofstd/ofstring.h>

#include <set>
#include <string>

// End-to-end coverage for the referenceFiles -> rule migration the
// multi-label native reader runs on legacy segs. The unit tests of the
// helper only exercise the early-exit branches; this driver builds a
// fresh seg, attaches a real referenceFiles property pointing at the
// three CT slice files dcmqi checks in, runs the migration and asserts
// the resulting rule connection matches what the source files declare.

namespace
{
  std::string ReadTag(const std::string &path, const DcmTagKey &tag)
  {
    DcmFileFormat ff;
    if (ff.loadFile(path.c_str()).bad() || ff.getDataset() == nullptr)
      return {};
    OFString value;
    if (ff.getDataset()->findAndGetOFString(tag, value).bad())
      return {};
    return value.c_str();
  }
}

int mitkDICOMSegmentationLegacyMigrationTest(int argc, char *argv[])
{
  MITK_TEST_BEGIN("DICOMSegmentationLegacyMigration");

  MITK_TEST_CONDITION_REQUIRED(argc >= 2,
    "Expected one or more source DICOM file paths as arguments");

  std::vector<std::string> sourceFiles;
  for (int i = 1; i < argc; ++i)
    sourceFiles.push_back(argv[i]);

  // Independently parse the source files so the migration is graded
  // against the file content, not against the migration's own logic.
  std::set<std::string> expectedSopInstances;
  std::set<std::string> expectedSeriesUIDs;
  for (const auto &path : sourceFiles)
  {
    const auto sop = ReadTag(path, DCM_SOPInstanceUID);
    const auto series = ReadTag(path, DCM_SeriesInstanceUID);
    MITK_TEST_CONDITION_REQUIRED(!sop.empty(),
      "Source DICOM file declares a non-empty SOPInstanceUID");
    expectedSopInstances.insert(sop);
    if (!series.empty())
      expectedSeriesUIDs.insert(series);
  }

  // Build a minimal seg whose only DICOM-flavoured property is the
  // legacy referenceFiles lookup table. The seg's own geometry is
  // irrelevant: the migration only reads the StringLookupTable values.
  auto geometryImage = mitk::Image::New();
  unsigned int dim[3] = {2u, 2u, 2u};
  geometryImage->Initialize(mitk::MakeScalarPixelType<mitk::Label::PixelType>(), 3, dim);

  auto seg = mitk::MultiLabelSegmentation::New();
  seg->Initialize(geometryImage);

  mitk::StringLookupTable lut;
  int key = 0;
  for (const auto &path : sourceFiles)
    lut.SetTableValue(key++, path);
  seg->SetProperty("referenceFiles", mitk::StringLookupTableProperty::New(lut));

  MITK_TEST_CONDITION_REQUIRED(
    mitk::SegSourceImageRelationRule::GetSourceImageRelations(seg).empty(),
    "Test precondition: seg starts with no rule connections");

  const auto added =
    mitk::DICOMSegmentationPropertyHelper::MigrateLegacyReferenceFilesToRelation(seg);

  MITK_TEST_CONDITION(added == expectedSeriesUIDs.size(),
    "Migration established exactly one rule connection per source series");

  const auto relations = mitk::SegSourceImageRelationRule::GetSourceImageRelations(seg);
  MITK_TEST_CONDITION(relations.size() == expectedSeriesUIDs.size(),
    "Seg carries one source-image relation per source series");

  std::set<std::string> seenSeriesUIDs;
  std::set<std::string> seenSopInstances;
  for (const auto &relation : relations)
  {
    seenSeriesUIDs.insert(relation.sourceSeriesInstanceUID);
    MITK_TEST_CONDITION_REQUIRED(relation.instanceUIDsPerSlice.IsNotNull(),
      "Per-slice SOPInstance property exists on the migrated relation");

    const auto slices = relation.instanceUIDsPerSlice->GetAvailableSlices(0);
    for (const auto slice : slices)
    {
      const auto sop = relation.instanceUIDsPerSlice->GetValue(0, slice);
      if (!sop.empty())
        seenSopInstances.insert(sop);
    }
  }

  MITK_TEST_CONDITION(seenSeriesUIDs == expectedSeriesUIDs,
    "Migrated source series UIDs match the ones declared by the source files");
  MITK_TEST_CONDITION(seenSopInstances == expectedSopInstances,
    "Migrated per-slice SOPInstance UIDs match the ones declared by the source files");

  // All files were resolved; the migration must drop the legacy property
  // to prevent the next save/load cycle from re-running it. Partial
  // migrations (some files missing) keep the property for retry.
  MITK_TEST_CONDITION(seg->GetProperty("referenceFiles").IsNull(),
    "referenceFiles property is removed after a fully-resolved migration");

  MITK_TEST_END();
}
