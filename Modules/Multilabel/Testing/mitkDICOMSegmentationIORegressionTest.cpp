/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkIOUtil.h>
#include <mitkLabelSetImage.h>
#include <mitkTestingMacros.h>

// Regression guard for the Phase 1 migration of
// mitk::DICOMSegmentationIO::DoRead to dcmqi's new SOP-Class factory API.
// A binary DICOM SEG input fixture is loaded through MITK's autoload path
// and the resulting MultiLabelSegmentation is compared against a reference
// segmentation that was generated against the pre-migration reader and
// checked into MITK-Data. Any drift in voxel layout or label metadata
// (geometry, group images, label names/colours/codes, tracking IDs) lands
// on the migration commit during bisect rather than silently shipping.

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

  MITK_TEST_END();
}
