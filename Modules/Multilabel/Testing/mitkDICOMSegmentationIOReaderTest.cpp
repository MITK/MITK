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

#include <mitkIOMimeTypes.h>
#include <mitkIOUtil.h>
#include <mitkLabelSetImage.h>
#include <mitkPropertyNameHelper.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/dcmdata/dcfilefo.h>

#include <itksys/SystemTools.hxx>

#include <fstream>
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
  MITK_TEST(TestReadFromStream);
  CPPUNIT_TEST_SUITE_END();

private:
  std::string m_TempPath;

  static const std::string& ReaderDescription()
  {
    static const std::string description = "DICOM Segmentation";
    return description;
  }

  /** The DICOM SEG mime type is not exported from its autoload module, so its name is spelled out. */
  static std::string MimeTypeName()
  {
    return mitk::IOMimeTypes::DEFAULT_BASE_NAME() + ".image.dicom.seg";
  }

  static std::string FixturePath(const std::string& fileName)
  {
    return GetTestDataFilePath("Multilabel/DICOMSegmentationIO/ExternalProducer/" + fileName);
  }

  std::string GetTempFilePath(const std::string& fileName) const
  {
    return m_TempPath + mitk::IOUtil::GetDirectorySeparator() + fileName;
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
  void setUp() override
  {
    m_TempPath = mitk::IOUtil::CreateTemporaryDirectory("mitk-DICOMSegmentationIOReaderTest-XXXXXX");
  }

  void tearDown() override
  {
    itksys::SystemTools::RemoveADirectory(m_TempPath);
  }

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

  void TestReadFromStream()
  {
    const auto path = FixturePath("ct3slice_binary.dcm");
    std::ifstream stream(path, std::ios::binary);
    CPPUNIT_ASSERT_MESSAGE("Cannot open " + path, stream.is_open());

    // A stream comes with a location that need not exist as a file.
    const auto result = mitk::test::ReadWithReaderOfMimeType(this->GetTempFilePath("stream-only.dcm"), MimeTypeName(),
                                                             ReaderDescription(), &stream);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Number of data read from the stream", std::size_t{1}, result.size());
    const auto* segmentation = dynamic_cast<const mitk::MultiLabelSegmentation*>(result.front().GetPointer());
    CPPUNIT_ASSERT_MESSAGE("The stream did not read as a segmentation", nullptr != segmentation);
    AssertMatchesFile(*segmentation, path);

    DcmFileFormat file;
    CPPUNIT_ASSERT(file.loadFile(path.c_str()).good());
    OFString studyInstanceUID;
    CPPUNIT_ASSERT(file.getDataset()->findAndGetOFString(DCM_StudyInstanceUID, studyInstanceUID).good());
    std::string readStudyInstanceUID;
    CPPUNIT_ASSERT_MESSAGE("The DICOM tags of the stream were not read",
                           mitk::GetDICOMPropertyValue(0x0020, 0x000D, segmentation->GetPropertyList(), readStudyInstanceUID));
    CPPUNIT_ASSERT_EQUAL(std::string(studyInstanceUID.c_str()), readStudyInstanceUID);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkDICOMSegmentationIOReader)
