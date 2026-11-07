/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkDICOMSegFileContentTestHelpers.h>
#include <mitkSegIOTestComparator.h>

#include <mitkIOUtil.h>
#include <mitkLabelSetImage.h>
#include <mitkTestingMacros.h>

#include <string>

// Reads a DICOM SEG file that MITK did not write and checks the segmentation against the
// file's own content, read with DCMTK: every segment's attributes and voxels.
int mitkDICOMSegmentationIOFileContentTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("DICOMSegmentationIOFileContent");

  MITK_TEST_CONDITION_REQUIRED(argc == 2, "Expected arguments: <dicom-seg-file>");
  const std::string path = argv[1];

  const auto content = mitk::test::ReadDICOMSegFileContent(path);

  const auto loaded = mitk::IOUtil::Load(path);
  MITK_TEST_CONDITION_REQUIRED(loaded.size() == 1, "The file loads as exactly one data object");
  const auto* segmentation = dynamic_cast<const mitk::MultiLabelSegmentation*>(loaded.front().GetPointer());
  MITK_TEST_CONDITION_REQUIRED(nullptr != segmentation, "The file loads as a segmentation");

  const auto differences = mitk::test::CompareWithDICOMSegFileContent(*segmentation, content);
  MITK_TEST_CONDITION(differences.empty(),
                      "The segmentation matches the file's content" << mitk::test::JoinDifferences(differences));

  MITK_TEST_END();
}
