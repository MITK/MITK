/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkDICOMSegFileContentTestHelpers.h>
#include <mitkSegIOTestCapabilities.h>
#include <mitkSegIOTestComparator.h>
#include <mitkSegIOTestFixtures.h>
#include <mitkSegSourceReferenceTestHelpers.h>

#include <mitkIOUtil.h>
#include <mitkImageWriteAccessor.h>
#include <mitkLabelSetImage.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/dcmdata/dcfilefo.h>

#include <itksys/SystemTools.hxx>

#include <string>
#include <vector>

/**
 * DICOM SEG round trips by value, in both encodings, through the capability table: the rich
 * fixture, its 1..N variant, a binary SEG whose overlap is undefined, and labelmaps on both
 * sides of the switch from 8 to 16 bit. Every file written is also checked on the wire.
 */
class mitkDICOMSegmentationIORoundTripTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkDICOMSegmentationIORoundTripTestSuite);
  MITK_TEST(TestRichBinary);
  MITK_TEST(TestRichLabelmap);
  MITK_TEST(TestOneToNBinary);
  MITK_TEST(TestOneToNLabelmap);
  MITK_TEST(TestBinaryOverlapUndefined);
  MITK_TEST(TestLabelmap255Labels);
  MITK_TEST(TestLabelmap256Labels);
  CPPUNIT_TEST_SUITE_END();

private:
  using SegmentationVector = std::vector<mitk::MultiLabelSegmentation::ConstPointer>;

  std::string m_TempPath;

  static mitk::MultiLabelSegmentation::Pointer BuildRichFixture()
  {
    mitk::test::RichSegmentationOptions options;
    options.dynamic = false;
    options.dicomLegalNames = true;
    options.withEmptyLabel = false;
    options.withEmptyGroup = false;
    return mitk::test::BuildRichSegmentation(options);
  }

  /** The writer names the file of group 0 as requested and appends "_<group>" for the others. */
  static std::string GroupFilePath(const std::string& path, mitk::MultiLabelSegmentation::GroupIndexType group)
  {
    if (0 == group)
      return path;
    return path.substr(0, path.size() - std::string(".dcm").size()) + "_" + std::to_string(group) + ".dcm";
  }

  /** Writes a clone of the segmentation and checks every file written on the wire; returns the path of group 0. */
  std::string Write(const mitk::MultiLabelSegmentation& segmentation, const std::string& encoding, const std::string& fileName) const
  {
    const auto path = m_TempPath + mitk::IOUtil::GetDirectorySeparator() + fileName;
    mitk::IFileWriter::Options options;
    options["Segmentation encoding"] = encoding;
    mitk::IOUtil::Save(mitk::test::CloneKeepingUID(segmentation), path, options);

    for (mitk::MultiLabelSegmentation::GroupIndexType group = 0; group < segmentation.GetNumberOfGroups(); ++group)
    {
      const auto file = GroupFilePath(path, group);
      const auto differences = mitk::test::CompareWrittenSegments(file, segmentation, group, "labelmap" == encoding);
      CPPUNIT_ASSERT_MESSAGE(file + " differs on the wire:" + mitk::test::JoinDifferences(differences), differences.empty());
    }
    return path;
  }

  static mitk::MultiLabelSegmentation::ConstPointer Load(const std::string& path)
  {
    const auto loaded = mitk::IOUtil::Load(path);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Number of data loaded from " + path, std::size_t{ 1 }, loaded.size());
    const auto* segmentation = dynamic_cast<const mitk::MultiLabelSegmentation*>(loaded.front().GetPointer());
    CPPUNIT_ASSERT_MESSAGE(path + " did not load as a segmentation", nullptr != segmentation);
    return segmentation;
  }

  /** Writes the segmentation and loads the file of every group back. */
  SegmentationVector WriteAndReload(const mitk::MultiLabelSegmentation& segmentation,
                                    const std::string& encoding,
                                    const std::string& fileName) const
  {
    const auto path = this->Write(segmentation, encoding, fileName);
    SegmentationVector reloaded;
    for (mitk::MultiLabelSegmentation::GroupIndexType group = 0; group < segmentation.GetNumberOfGroups(); ++group)
      reloaded.push_back(Load(GroupFilePath(path, group)));
    return reloaded;
  }

  void TestRich(const std::string& encoding)
  {
    const auto rich = BuildRichFixture();
    const auto reloaded = this->WriteAndReload(*rich, encoding, "rich-" + encoding + ".dcm");
    mitk::test::AssertReloadMatchesCapabilities(
      mitk::test::SplitIntoGroups(*rich), reloaded, mitk::test::DICOMSegCapabilities(false));
  }

  /** The groups of the rich fixture, each written on its own with its values mapped to 1..N, keep the values. */
  void TestOneToN(const std::string& encoding)
  {
    SegmentationVector originals;
    SegmentationVector reloaded;
    for (const auto& group : mitk::test::SplitIntoGroups(*BuildRichFixture()))
    {
      auto oneToN = mitk::test::CloneKeepingUID(*group);
      mitk::test::RenumberToRanks(*oneToN);
      const auto fileName = "one-to-n-" + encoding + "-" + std::to_string(originals.size()) + ".dcm";
      reloaded.push_back(this->WriteAndReload(*oneToN, encoding, fileName).front());
      originals.push_back(oneToN.GetPointer());
    }
    mitk::test::AssertReloadMatchesCapabilities(originals, reloaded, mitk::test::DICOMSegCapabilities(true));
  }

  /** Labelmaps store segment numbers as pixel values, with 8 bit up to 255 segments and 16 bit beyond. */
  void TestLabelmap(unsigned int labelCount, const std::string& bitsAllocated)
  {
    const auto segmentation = mitk::test::BuildManyLabelSegmentation(labelCount);
    const auto path = this->Write(*segmentation, "labelmap", "labelmap-" + std::to_string(labelCount) + ".dcm");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Bits Allocated of a labelmap with " + std::to_string(labelCount) + " segments",
                                 bitsAllocated, mitk::test::ReadTag(path, DCM_BitsAllocated));

    mitk::test::AssertReloadMatchesCapabilities(
      { segmentation.GetPointer() }, { Load(path) }, mitk::test::DICOMSegCapabilities(true));
  }

public:
  void setUp() override
  {
    m_TempPath = mitk::IOUtil::CreateTemporaryDirectory("mitkDICOMSegRoundTripTest_XXXXXX");
  }

  void tearDown() override
  {
    itksys::SystemTools::RemoveADirectory(m_TempPath);
  }

  void TestRichBinary()
  {
    this->TestRich("binary");
  }

  void TestRichLabelmap()
  {
    this->TestRich("labelmap");
  }

  void TestOneToNBinary()
  {
    this->TestOneToN("binary");
  }

  void TestOneToNLabelmap()
  {
    this->TestOneToN("labelmap");
  }

  /**
   * MITK always writes SegmentsOverlap NO. Other producers write UNDEFINED, for which the reader
   * gives every segment a group of its own. The named group of the rich fixture is used, so that
   * the group name loss has something to change.
   */
  void TestBinaryOverlapUndefined()
  {
    const auto group = mitk::test::SplitIntoGroups(*BuildRichFixture()).back();
    const auto path = this->Write(*group, "binary", "overlap.dcm");

    DcmFileFormat file;
    CPPUNIT_ASSERT_MESSAGE("DCMTK cannot read " + path, file.loadFile(path.c_str()).good());
    CPPUNIT_ASSERT(file.getDataset()->putAndInsertString(DCM_SegmentsOverlap, "UNDEFINED").good());
    const auto undefinedPath = m_TempPath + mitk::IOUtil::GetDirectorySeparator() + "overlap-undefined.dcm";
    CPPUNIT_ASSERT_MESSAGE("DCMTK cannot write " + undefinedPath,
                           file.saveFile(undefinedPath.c_str(), EXS_LittleEndianExplicit).good());

    auto capabilities = mitk::test::DICOMSegCapabilities(false);
    capabilities.losses.push_back({
      "SegmentsOverlap UNDEFINED: the reader gives every segment after the first a group of its own, in segment order",
      [](mitk::MultiLabelSegmentation& segmentation) {
        const auto values = segmentation.GetLabelValuesByGroup(0);
        for (std::size_t i = 1; i < values.size(); ++i)
        {
          auto groupImage = segmentation.GetGroupImage(0)->Clone();
          {
            mitk::ImageWriteAccessor accessor(groupImage);
            auto* voxels = static_cast<mitk::Label::PixelType*>(accessor.GetData());
            const auto voxelCount = static_cast<std::size_t>(groupImage->GetDimension(0)) *
                                    groupImage->GetDimension(1) * groupImage->GetDimension(2);
            for (std::size_t voxel = 0; voxel < voxelCount; ++voxel)
            {
              if (voxels[voxel] != values[i])
                voxels[voxel] = mitk::MultiLabelSegmentation::UNLABELED_VALUE;
            }
          }
          const mitk::Label::ConstPointer label = segmentation.GetLabel(values[i])->Clone().GetPointer();
          segmentation.RemoveLabel(values[i]);
          segmentation.AddGroup(groupImage, { label });
        }
      } });

    mitk::test::AssertReloadMatchesCapabilities({ group }, { Load(undefinedPath) }, capabilities);
  }

  void TestLabelmap255Labels()
  {
    this->TestLabelmap(255, "8");
  }

  void TestLabelmap256Labels()
  {
    this->TestLabelmap(256, "16");
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkDICOMSegmentationIORoundTrip)
