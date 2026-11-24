/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkDICOMSegFileContentTestHelpers.h>
#include <mitkSegIOTestComparator.h>
#include <mitkSegIOTestFixtures.h>

#include <mitkImageReadAccessor.h>
#include <mitkImageWriteAccessor.h>
#include <mitkLabelSetImage.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>

/**
 * Self-test of the DICOM SEG file-content oracles: they must report readers that get a file
 * wrong, and refuse input they do not model.
 */
class mitkDICOMSegFileContentTestHelpersTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkDICOMSegFileContentTestHelpersTestSuite);
  MITK_TEST(StrayValueIsDetected);
  MITK_TEST(SegmentInOtherGroupIsDetected);
  MITK_TEST(SubVoxelShiftIsDetected);
  MITK_TEST(ExpectedSegmentSequenceRefusesLabelWithoutVoxels);
  CPPUNIT_TEST_SUITE_END();

private:
  static std::string FixturePath(const std::string& fileName)
  {
    return GetTestDataFilePath("Multilabel/DICOMSegmentationIO/ExternalProducer/" + fileName);
  }

  static bool Mentions(const std::vector<std::string>& differences, const std::string& text)
  {
    return std::any_of(differences.begin(), differences.end(),
      [&text](const std::string& difference) { return difference.find(text) != std::string::npos; });
  }

  /** Loads the fixture, which the oracle must find equal to its file before the test doctors it. */
  static mitk::MultiLabelSegmentation::Pointer LoadUndoctored(const std::string& path)
  {
    auto segmentation = mitk::test::LoadSingleSegmentation(path);
    const auto differences = mitk::test::CompareWithDICOMSegFile(*segmentation, path);
    CPPUNIT_ASSERT_MESSAGE("Precondition: " + path + " is read as it is:" + mitk::test::JoinDifferences(differences),
                           differences.empty());
    return segmentation;
  }

  static void AssertDetected(const mitk::MultiLabelSegmentation& segmentation, const std::string& path, const std::string& text)
  {
    const auto differences = mitk::test::CompareWithDICOMSegFile(segmentation, path);
    CPPUNIT_ASSERT_MESSAGE("Expected a difference mentioning \"" + text + "\"; got:" + mitk::test::JoinDifferences(differences),
                           Mentions(differences, text));
  }

public:
  // A reader that leaves a value no segment of the group has.
  void StrayValueIsDetected()
  {
    const auto path = FixturePath("ct3slice_binary.dcm");
    auto segmentation = LoadUndoctored(path);

    {
      auto* image = segmentation->GetGroupImage(0);
      mitk::ImageWriteAccessor accessor(image);
      auto* voxels = static_cast<mitk::Label::PixelType*>(accessor.GetData());
      const std::size_t voxelCount =
        static_cast<std::size_t>(image->GetDimension(0)) * image->GetDimension(1) * image->GetDimension(2);
      auto* unlabeled = std::find(voxels, voxels + voxelCount, mitk::MultiLabelSegmentation::UNLABELED_VALUE);
      CPPUNIT_ASSERT_MESSAGE("Precondition: the group image has an unlabeled voxel", unlabeled != voxels + voxelCount);
      *unlabeled = 999;
    }

    AssertDetected(*segmentation, path, "which no segment of the group has");
  }

  // A reader that writes the voxels of a segment into the image of another group as well. The
  // segments of this file overlap, so each has its own group.
  void SegmentInOtherGroupIsDetected()
  {
    const auto path = FixturePath("ct3slice_binary_overlap.dcm");
    auto segmentation = LoadUndoctored(path);
    CPPUNIT_ASSERT_MESSAGE("Precondition: one group per segment", segmentation->GetNumberOfGroups() > 1);

    const auto* source = segmentation->GetGroupImage(1);
    auto* target = segmentation->GetGroupImage(0);
    const std::size_t voxelCount =
      static_cast<std::size_t>(target->GetDimension(0)) * target->GetDimension(1) * target->GetDimension(2);
    const auto valueOfGroup1 = segmentation->GetLabelValuesByGroup(1).front();

    std::size_t copied = 0;
    {
      mitk::ImageReadAccessor sourceAccessor(source);
      const auto* sourceVoxels = static_cast<const mitk::Label::PixelType*>(sourceAccessor.GetData());
      mitk::ImageWriteAccessor targetAccessor(target);
      auto* targetVoxels = static_cast<mitk::Label::PixelType*>(targetAccessor.GetData());
      for (std::size_t offset = 0; offset < voxelCount; ++offset)
      {
        // Only voxels that group 0 leaves unlabeled, so the doctoring trips no check of group 0's own segment.
        if (sourceVoxels[offset] == valueOfGroup1 && targetVoxels[offset] == mitk::MultiLabelSegmentation::UNLABELED_VALUE)
        {
          targetVoxels[offset] = valueOfGroup1;
          ++copied;
        }
      }
    }
    CPPUNIT_ASSERT_MESSAGE("Precondition: group 1 has voxels that group 0 leaves unlabeled", copied > 0);

    AssertDetected(*segmentation, path, "which no segment of the group has");
  }

  // A reader whose group image lies less than half a voxel off the frames.
  void SubVoxelShiftIsDetected()
  {
    const auto path = FixturePath("ct3slice_binary.dcm");
    auto segmentation = LoadUndoctored(path);

    auto* geometry = segmentation->GetGroupImage(0)->GetGeometry();
    mitk::Vector3D indexShift;
    indexShift.Fill(0.0);
    indexShift[0] = 0.3;
    mitk::Vector3D worldShift;
    geometry->IndexToWorld(indexShift, worldShift);
    geometry->SetOrigin(geometry->GetOrigin() + worldShift);

    AssertDetected(*segmentation, path, "off the voxel centres");
  }

  // dcmqi writes no segment for a label without voxels, so the writer oracle must not predict one.
  void ExpectedSegmentSequenceRefusesLabelWithoutVoxels()
  {
    const auto segmentation = mitk::test::BuildRichSegmentation({ false, true, true, false });
    const auto hasEmptyLabel = [&segmentation](mitk::MultiLabelSegmentation::GroupIndexType group) {
      const auto values = segmentation->GetLabelValuesByGroup(group);
      return std::any_of(values.begin(), values.end(), [&segmentation](auto value) { return segmentation->IsEmpty(value); });
    };
    CPPUNIT_ASSERT_MESSAGE("Precondition: group 0 has a label without voxels", hasEmptyLabel(0));
    CPPUNIT_ASSERT_MESSAGE("Precondition: group 1 has none", !hasEmptyLabel(1));

    CPPUNIT_ASSERT_THROW(mitk::test::ExpectedSegmentSequence(*segmentation, 0, false), mitk::Exception);
    CPPUNIT_ASSERT_THROW(mitk::test::ExpectedSegmentSequence(*segmentation, 0, true), mitk::Exception);
    CPPUNIT_ASSERT_NO_THROW(mitk::test::ExpectedSegmentSequence(*segmentation, 1, false));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkDICOMSegFileContentTestHelpers)
