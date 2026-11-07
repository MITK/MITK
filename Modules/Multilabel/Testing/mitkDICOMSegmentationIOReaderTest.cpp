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

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <string>
#include <vector>

/**
 * The DICOM SEG reader against files it did not write: SEGs from highdicom, read by value
 * against their own content.
 */
class mitkDICOMSegmentationIOReaderTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkDICOMSegmentationIOReaderTestSuite);
  MITK_TEST(TestReadBinary);
  MITK_TEST(TestReadBinaryOverlap);
  MITK_TEST(TestReadLabelmap);
  CPPUNIT_TEST_SUITE_END();

private:
  static std::string FixturePath(const std::string& fileName)
  {
    return GetTestDataFilePath("Multilabel/DICOMSegmentationIO/ExternalProducer/" + fileName);
  }

  static mitk::MultiLabelSegmentation::Pointer LoadSegmentation(const std::string& path)
  {
    const auto loaded = mitk::IOUtil::Load(path);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Number of data loaded from " + path, std::size_t{1}, loaded.size());
    auto* segmentation = dynamic_cast<mitk::MultiLabelSegmentation*>(loaded.front().GetPointer());
    CPPUNIT_ASSERT_MESSAGE(path + " did not load as a segmentation", nullptr != segmentation);
    return segmentation;
  }

  static void AssertMatchesFile(const mitk::MultiLabelSegmentation& segmentation, const std::string& path)
  {
    const auto differences = mitk::test::CompareWithDICOMSegFile(segmentation, path);
    CPPUNIT_ASSERT_MESSAGE(path + " was read with differences to its content:" + mitk::test::JoinDifferences(differences),
                           differences.empty());
  }

  static void AssertGroupValues(const mitk::MultiLabelSegmentation& segmentation,
                                const std::vector<mitk::MultiLabelSegmentation::LabelValueVectorType>& expected)
  {
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Number of groups", expected.size(), std::size_t{ segmentation.GetNumberOfGroups() });
    for (std::size_t group = 0; group < expected.size(); ++group)
    {
      CPPUNIT_ASSERT_MESSAGE("Label values of group " + std::to_string(group),
                             expected[group] == segmentation.GetLabelValuesByGroup(static_cast<mitk::MultiLabelSegmentation::GroupIndexType>(group)));
    }
  }

public:
  void TestReadBinary()
  {
    const auto path = FixturePath("ct3slice_binary.dcm");
    const auto segmentation = LoadSegmentation(path);
    // SegmentsOverlap is NO, so the segments share one group, valued by their segment numbers.
    AssertGroupValues(*segmentation, { { 1, 2, 3, 4 } });
    AssertMatchesFile(*segmentation, path);
  }

  void TestReadBinaryOverlap()
  {
    const auto path = FixturePath("ct3slice_binary_overlap.dcm");
    const auto segmentation = LoadSegmentation(path);
    AssertGroupValues(*segmentation, { { 1 }, { 2 }, { 3 } });
    AssertMatchesFile(*segmentation, path);
  }

  void TestReadLabelmap()
  {
    const auto path = FixturePath("ct3slice_labelmap.dcm");
    const auto segmentation = LoadSegmentation(path);
    // The segment numbers are not consecutive, and the background segment 0 yields no label.
    AssertGroupValues(*segmentation, { { 1, 3, 7 } });
    AssertMatchesFile(*segmentation, path);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkDICOMSegmentationIOReader)
