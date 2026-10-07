/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkImageReadAccessor.h>
#include <mitkImageWriteAccessor.h>
#include <mitkLabelGroupPlacer.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <algorithm>
#include <functional>
#include <map>

class mitkLabelGroupPlacerTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkLabelGroupPlacerTestSuite);
  MITK_TEST(TestExtractMaskRuns);
  MITK_TEST(TestDisjointMaskGoesIntoActiveGroup);
  MITK_TEST(TestActiveGroupIsTriedFirst);
  MITK_TEST(TestMaskInsideLabelNeedsNewGroup);
  MITK_TEST(TestLabelInsideMaskNeedsNewGroup);
  MITK_TEST(TestBorderContactIsNoOverlap);
  MITK_TEST(TestBorderContactIsOverlapWithoutTolerance);
  MITK_TEST(TestToleranceOutOfRangeIsRejected);
  MITK_TEST(TestEarlierMasksCount);
  MITK_TEST(TestOverwriteTakesVoxels);
  MITK_TEST(TestMaskOfOtherSizeIsRejected);
  CPPUNIT_TEST_SUITE_END();

  using Predicate = std::function<bool(unsigned int x, unsigned int y, unsigned int z)>;

  static constexpr unsigned int SIZE = 10;
  static constexpr double TOLERANCE = 0.02;

  mitk::MultiLabelSegmentation::Pointer m_Segmentation;
  mitk::MultiLabelSegmentation::Pointer m_Preview;

public:
  void setUp() override
  {
    unsigned int dimensions[3] = { SIZE, SIZE, SIZE };

    auto reference = mitk::Image::New();
    reference->Initialize(mitk::MakeScalarPixelType<short>(), 3, dimensions);

    m_Segmentation = mitk::MultiLabelSegmentation::New();
    m_Segmentation->Initialize(reference);
  }

  void tearDown() override
  {
    m_Preview = nullptr;
    m_Segmentation = nullptr;
  }

  void TestExtractMaskRuns()
  {
    const auto mask = this->MakeMask([](unsigned int x, unsigned int y, unsigned int z)
    {
      return z == 0 && ((y == 0 && ((x >= 2 && x < 5) || x >= 7)) || y == 1);
    });

    const auto runs = mitk::ExtractMaskRuns(mask);

    CPPUNIT_ASSERT_EQUAL(std::size_t(SIZE * SIZE * SIZE), runs.VolumeSize);
    CPPUNIT_ASSERT_EQUAL(std::size_t(16), runs.VoxelCount);
    CPPUNIT_ASSERT_EQUAL(std::size_t(3), runs.Runs.size());
    CPPUNIT_ASSERT_EQUAL(std::size_t(2), runs.Runs[0].Offset);
    CPPUNIT_ASSERT_EQUAL(std::size_t(3), runs.Runs[0].Length);
    CPPUNIT_ASSERT_EQUAL(std::size_t(7), runs.Runs[1].Offset);
    CPPUNIT_ASSERT_EQUAL(std::size_t(3), runs.Runs[1].Length);
    CPPUNIT_ASSERT_EQUAL(std::size_t(SIZE), runs.Runs[2].Offset);
    CPPUNIT_ASSERT_EQUAL(std::size_t(SIZE), runs.Runs[2].Length);
  }

  void TestDisjointMaskGoesIntoActiveGroup()
  {
    this->AddSegmentationLabel(1, 0, [](unsigned int x, unsigned int, unsigned int) { return x < 5; });
    this->MakePreview();

    mitk::LabelGroupPlacer placer(m_Segmentation, m_Preview, 0, TOLERANCE);
    const auto mask = this->MakeMaskRuns([](unsigned int x, unsigned int, unsigned int) { return x >= 5; });

    CPPUNIT_ASSERT_EQUAL(mitk::MultiLabelSegmentation::GroupIndexType(0), placer.FindGroup(mask));
  }

  void TestActiveGroupIsTriedFirst()
  {
    m_Segmentation->AddGroup();
    this->AddSegmentationLabel(1, 1, [](unsigned int x, unsigned int, unsigned int) { return x < 5; });
    m_Segmentation->SetActiveLabel(1);
    this->MakePreview();

    mitk::LabelGroupPlacer placer(m_Segmentation, m_Preview, 0, TOLERANCE);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("A mask that fits into the active group should go there",
      mitk::MultiLabelSegmentation::GroupIndexType(1),
      placer.FindGroup(this->MakeMaskRuns([](unsigned int x, unsigned int, unsigned int) { return x >= 5; })));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("A mask that overlaps a label of the active group should go into another group",
      mitk::MultiLabelSegmentation::GroupIndexType(0),
      placer.FindGroup(this->MakeMaskRuns([](unsigned int x, unsigned int, unsigned int) { return x < 3; })));
  }

  void TestMaskInsideLabelNeedsNewGroup()
  {
    this->AddSegmentationLabel(1, 0, [](unsigned int x, unsigned int, unsigned int) { return x < 5; });
    this->MakePreview();

    mitk::LabelGroupPlacer placer(m_Segmentation, m_Preview, 0, TOLERANCE);
    const auto mask = this->MakeMaskRuns([](unsigned int x, unsigned int y, unsigned int) { return x < 2 && y < 2; });
    const auto group = placer.FindGroup(mask);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("A mask that fits into no group should ask for a new one",
      mitk::MultiLabelSegmentation::GroupIndexType(m_Preview->GetNumberOfGroups()), group);

    m_Preview->AddGroup();
    m_Preview->AddLabel(mitk::Label::New(7, "part"), group, false, false);
    placer.Write(mask, group, 7, mitk::LabelGroupPlacer::WriteMode::KeepOccupiedVoxels);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("A mask in a group of its own should keep all of its voxels",
      mask.VoxelCount, placer.GetVoxelCount(7));
    CPPUNIT_ASSERT_EQUAL(mask.VoxelCount, this->CountPreviewVoxels(group, 7));
  }

  // The share of the smaller label decides, so a large mask around a small
  // label overlaps it, although it shares few of its own voxels.
  void TestLabelInsideMaskNeedsNewGroup()
  {
    this->AddSegmentationLabel(1, 0, [](unsigned int x, unsigned int y, unsigned int z) { return x == 0 && y == 0 && z < 5; });
    this->MakePreview();

    mitk::LabelGroupPlacer placer(m_Segmentation, m_Preview, 0, TOLERANCE);
    const auto mask = this->MakeMaskRuns([](unsigned int, unsigned int, unsigned int) { return true; });

    CPPUNIT_ASSERT_EQUAL(mitk::MultiLabelSegmentation::GroupIndexType(1), placer.FindGroup(mask));
  }

  void TestBorderContactIsNoOverlap()
  {
    this->AddSegmentationLabel(1, 0, [](unsigned int x, unsigned int, unsigned int) { return x < 5; });
    this->MakePreview();

    mitk::LabelGroupPlacer placer(m_Segmentation, m_Preview, 0, TOLERANCE);

    // Shares one voxel with the label, 0.2% of the 500 voxels of the label.
    const auto mask = this->MakeMaskRuns([](unsigned int x, unsigned int y, unsigned int z) { return x >= 5 || (x == 4 && y == 0 && z == 0); });
    const auto group = placer.FindGroup(mask);

    CPPUNIT_ASSERT_EQUAL(mitk::MultiLabelSegmentation::GroupIndexType(0), group);

    m_Preview->AddLabel(mitk::Label::New(2, "neighbor"), group, false, false);
    placer.Write(mask, group, 2, mitk::LabelGroupPlacer::WriteMode::KeepOccupiedVoxels);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("The shared voxel should stay with the label of the segmentation",
      std::size_t(500), placer.GetVoxelCount(2));
    CPPUNIT_ASSERT_EQUAL(std::size_t(500), this->CountPreviewVoxels(0, 2));
  }

  void TestBorderContactIsOverlapWithoutTolerance()
  {
    this->AddSegmentationLabel(1, 0, [](unsigned int x, unsigned int, unsigned int) { return x < 5; });
    this->MakePreview();

    mitk::LabelGroupPlacer placer(m_Segmentation, m_Preview, 0, 0.0);
    const auto mask = this->MakeMaskRuns([](unsigned int x, unsigned int y, unsigned int z) { return x >= 5 || (x == 4 && y == 0 && z == 0); });

    CPPUNIT_ASSERT_EQUAL(mitk::MultiLabelSegmentation::GroupIndexType(1), placer.FindGroup(mask));
  }

  void TestToleranceOutOfRangeIsRejected()
  {
    this->MakePreview();

    CPPUNIT_ASSERT_THROW(mitk::LabelGroupPlacer(m_Segmentation, m_Preview, 0, -0.01), mitk::Exception);
    CPPUNIT_ASSERT_THROW(mitk::LabelGroupPlacer(m_Segmentation, m_Preview, 0, 1.01), mitk::Exception);
  }

  void TestEarlierMasksCount()
  {
    this->MakePreview();

    mitk::LabelGroupPlacer placer(m_Segmentation, m_Preview, 0, TOLERANCE);

    const auto part = this->MakeMaskRuns([](unsigned int x, unsigned int, unsigned int) { return x < 3; });
    m_Preview->AddLabel(mitk::Label::New(1, "part"), 0, false, false);
    placer.Write(part, 0, 1, mitk::LabelGroupPlacer::WriteMode::KeepOccupiedVoxels);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("A mask around an earlier one should not go into its group",
      mitk::MultiLabelSegmentation::GroupIndexType(1),
      placer.FindGroup(this->MakeMaskRuns([](unsigned int x, unsigned int, unsigned int) { return x < 5; })));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("A mask beside an earlier one should go into its group",
      mitk::MultiLabelSegmentation::GroupIndexType(0),
      placer.FindGroup(this->MakeMaskRuns([](unsigned int x, unsigned int, unsigned int) { return x >= 3; })));
  }

  void TestOverwriteTakesVoxels()
  {
    this->MakePreview();
    m_Preview->AddLabel(mitk::Label::New(1, "first"), 0, false, false);
    m_Preview->AddLabel(mitk::Label::New(2, "second"), 0, false, false);

    mitk::LabelGroupPlacer placer(m_Segmentation, m_Preview, 0, TOLERANCE);

    const auto first = this->MakeMaskRuns([](unsigned int x, unsigned int, unsigned int) { return x < 5; });
    const auto second = this->MakeMaskRuns([](unsigned int x, unsigned int, unsigned int) { return x >= 3; });

    placer.Write(first, 0, 1, mitk::LabelGroupPlacer::WriteMode::OverwriteVoxels);
    placer.Write(second, 0, 2, mitk::LabelGroupPlacer::WriteMode::OverwriteVoxels);

    CPPUNIT_ASSERT_EQUAL(std::size_t(300), placer.GetVoxelCount(1));
    CPPUNIT_ASSERT_EQUAL(std::size_t(700), placer.GetVoxelCount(2));
    CPPUNIT_ASSERT_EQUAL(std::size_t(300), this->CountPreviewVoxels(0, 1));
    CPPUNIT_ASSERT_EQUAL(std::size_t(700), this->CountPreviewVoxels(0, 2));
  }

  void TestMaskOfOtherSizeIsRejected()
  {
    this->MakePreview();

    unsigned int dimensions[3] = { SIZE, SIZE, SIZE + 1 };
    auto mask = mitk::Image::New();
    mask->Initialize(mitk::MakeScalarPixelType<unsigned char>(), 3, dimensions);

    mitk::LabelGroupPlacer placer(m_Segmentation, m_Preview, 0, TOLERANCE);

    CPPUNIT_ASSERT_THROW(placer.FindGroup(mitk::ExtractMaskRuns(mask)), mitk::Exception);
  }

private:
  static std::size_t Index(unsigned int x, unsigned int y, unsigned int z)
  {
    return (static_cast<std::size_t>(z) * SIZE + y) * SIZE + x;
  }

  template <typename TPixel>
  static void Fill(TPixel* pixels, TPixel value, const Predicate& predicate)
  {
    for (unsigned int z = 0; z < SIZE; ++z)
    {
      for (unsigned int y = 0; y < SIZE; ++y)
      {
        for (unsigned int x = 0; x < SIZE; ++x)
        {
          if (predicate(x, y, z))
            pixels[Index(x, y, z)] = value;
        }
      }
    }
  }

  mitk::Image::Pointer MakeMask(const Predicate& predicate) const
  {
    unsigned int dimensions[3] = { SIZE, SIZE, SIZE };

    auto mask = mitk::Image::New();
    mask->Initialize(mitk::MakeScalarPixelType<unsigned char>(), 3, dimensions);

    mitk::ImageWriteAccessor accessor(mask);
    auto* pixels = static_cast<unsigned char*>(accessor.GetData());
    std::fill(pixels, pixels + Index(0, 0, SIZE), static_cast<unsigned char>(0));
    Fill(pixels, static_cast<unsigned char>(1), predicate);

    return mask;
  }

  mitk::MaskRuns MakeMaskRuns(const Predicate& predicate) const
  {
    return mitk::ExtractMaskRuns(this->MakeMask(predicate));
  }

  void AddSegmentationLabel(mitk::Label::PixelType value, mitk::MultiLabelSegmentation::GroupIndexType group, const Predicate& predicate)
  {
    m_Segmentation->AddLabel(mitk::Label::New(value, "label"), group, false, false);

    auto* groupImage = m_Segmentation->GetGroupImage(group);
    mitk::ImageWriteAccessor accessor(groupImage, groupImage->GetVolumeData(0));
    Fill(static_cast<mitk::Label::PixelType*>(accessor.GetData()), value, predicate);
  }

  // Like the preview of a tool that holds nothing but its own results.
  void MakePreview()
  {
    m_Preview = m_Segmentation->Clone();

    std::map<mitk::MultiLabelSegmentation::GroupIndexType, mitk::MultiLabelSegmentation::ConstLabelVectorType> emptyGroups;

    for (mitk::MultiLabelSegmentation::GroupIndexType group = 0; group < m_Preview->GetNumberOfGroups(); ++group)
      emptyGroups[group] = {};

    m_Preview->ReplaceGroupLabels(emptyGroups);
    m_Preview->ClearGroupImages();
  }

  std::size_t CountPreviewVoxels(mitk::MultiLabelSegmentation::GroupIndexType group, mitk::Label::PixelType value) const
  {
    const auto* groupImage = m_Preview->GetGroupImage(group);
    mitk::ImageReadAccessor accessor(groupImage, groupImage->GetVolumeData(0));
    const auto* pixels = static_cast<const mitk::Label::PixelType*>(accessor.GetData());

    return static_cast<std::size_t>(std::count(pixels, pixels + Index(0, 0, SIZE), value));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkLabelGroupPlacer)
