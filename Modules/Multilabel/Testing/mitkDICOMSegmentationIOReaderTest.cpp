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

#include <mitkIOUtil.h>
#include <mitkLabelSetImage.h>
#include <mitkPropertyNameHelper.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcsequen.h>
#include <dcmtk/dcmdata/dcuid.h>

#include <itksys/SystemTools.hxx>

#include <vtkLookupTable.h>

#include <algorithm>
#include <fstream>
#include <functional>
#include <string>
#include <vector>

/**
 * The DICOM SEG reader against files it did not write: SEGs from highdicom, read by value
 * against their own content, and malformed or unsupported files, which the reader refuses.
 */
class mitkDICOMSegmentationIOReaderTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkDICOMSegmentationIOReaderTestSuite);
  MITK_TEST(TestReadBinary);
  MITK_TEST(TestReadBinaryOverlap);
  MITK_TEST(TestReadLabelmap);
  MITK_TEST(TestReadFromStream);
  MITK_TEST(TestReadFractionalFails);
  MITK_TEST(TestReadNonDICOMFileFails);
  MITK_TEST(TestReadNonSegmentationSOPClassFails);
  MITK_TEST(TestReadLabelmapPixelWithoutSegmentFails);
  MITK_TEST(TestReadLabelmapSegmentWithoutPixelsFails);
  MITK_TEST(TestReadInvalidAlgorithmTypeFails);
  CPPUNIT_TEST_SUITE_END();

private:
  std::string m_TempPath;

  static const std::string& ReaderDescription()
  {
    static const std::string description = "DICOM Segmentation";
    return description;
  }

  static std::string FixturePath(const std::string& fileName)
  {
    return GetTestDataFilePath("Multilabel/DICOMSegmentationIO/ExternalProducer/" + fileName);
  }

  std::string GetTempFilePath(const std::string& fileName) const
  {
    return m_TempPath + mitk::IOUtil::GetDirectorySeparator() + fileName;
  }

  // The mappers render the lookup table, not the labels, so the table must show what the labels say.
  static void AssertLookupTableMatchesLabels(const mitk::MultiLabelSegmentation& segmentation)
  {
    const auto table = segmentation.GetLookupTable()->GetVtkLookupTable();
    // The table holds 8 bit per channel.
    const double tolerance = 1.0 / 255.0;
    for (const auto& label : segmentation.GetLabels())
    {
      double rgba[4];
      table->GetTableValue(label->GetValue(), rgba);
      const auto context = "Lookup table entry of label " + std::to_string(label->GetValue());
      const auto& color = label->GetColor();
      CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(context + ", red", color.GetRed(), rgba[0], tolerance);
      CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(context + ", green", color.GetGreen(), rgba[1], tolerance);
      CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(context + ", blue", color.GetBlue(), rgba[2], tolerance);
      CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(context + ", alpha",
                                           label->GetVisible() ? static_cast<double>(label->GetOpacity()) : 0.0,
                                           rgba[3], tolerance);
    }
  }

  static void AssertMatchesFile(const mitk::MultiLabelSegmentation& segmentation, const std::string& path)
  {
    const auto differences = mitk::test::CompareWithDICOMSegFile(segmentation, path);
    CPPUNIT_ASSERT_MESSAGE(path + " was read with differences to its content:" + mitk::test::JoinDifferences(differences),
                           differences.empty());
    AssertLookupTableMatchesLabels(segmentation);
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

  /** Writes a copy of a fixture into the temporary directory, changed by edit. */
  std::string WriteEditedCopy(const std::string& fixture,
                              const std::string& fileName,
                              const std::function<void(DcmDataset&)>& edit) const
  {
    DcmFileFormat file;
    CPPUNIT_ASSERT_MESSAGE("DCMTK cannot read " + fixture, file.loadFile(FixturePath(fixture).c_str()).good());
    edit(*file.getDataset());
    const auto path = this->GetTempFilePath(fileName);
    CPPUNIT_ASSERT_MESSAGE("DCMTK cannot write " + path, file.saveFile(path.c_str(), EXS_LittleEndianExplicit).good());
    return path;
  }

  static DcmItem* FindSegmentItem(DcmDataset& dataset, Uint16 number)
  {
    for (auto* item : mitk::test::detail::GetSequenceItems(dataset, DCM_SegmentSequence))
    {
      Uint16 itemNumber = 0;
      if (item->findAndGetUint16(DCM_SegmentNumber, itemNumber).good() && itemNumber == number)
        return item;
    }
    CPPUNIT_FAIL("The fixture has no segment " + std::to_string(number));
    return nullptr;
  }

  static void AssertExceptionMentions(const mitk::Exception& exception, const std::vector<std::string>& texts)
  {
    const std::string message = exception.GetDescription();
    for (const auto& text : texts)
    {
      CPPUNIT_ASSERT_MESSAGE("The exception should mention \"" + text + "\"; got: " + message,
                             message.find(text) != std::string::npos);
    }
  }

  /** The reader, called directly, must throw mitk::Exception mentioning every text. */
  static void AssertReaderFails(const std::string& path, const std::vector<std::string>& texts)
  {
    try
    {
      mitk::test::ReadWithReaderOfMimeType(path, mitk::test::DICOMSegMimeTypeName(), ReaderDescription());
      CPPUNIT_FAIL("Reading " + path + " did not fail");
    }
    catch (const mitk::Exception& e)
    {
      AssertExceptionMentions(e, texts);
    }
  }

  /** Like AssertReaderFails; IOUtil, which picks the reader for the file, must report the same. */
  static void AssertReadFails(const std::string& path, const std::vector<std::string>& texts)
  {
    AssertReaderFails(path, texts);
    try
    {
      mitk::IOUtil::Load(path);
      CPPUNIT_FAIL("Loading " + path + " did not fail");
    }
    catch (const mitk::Exception& e)
    {
      AssertExceptionMentions(e, texts);
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
    const auto segmentation = mitk::test::LoadSingleSegmentation(path);
    // SegmentsOverlap is NO, so the segments share one group, valued by their segment numbers.
    AssertGroupValues(*segmentation, { { 1, 2, 3, 4 } });
    AssertMatchesFile(*segmentation, path);
  }

  void TestReadBinaryOverlap()
  {
    const auto path = FixturePath("ct3slice_binary_overlap.dcm");
    const auto segmentation = mitk::test::LoadSingleSegmentation(path);
    AssertGroupValues(*segmentation, { { 1 }, { 2 }, { 3 } });
    AssertMatchesFile(*segmentation, path);
  }

  void TestReadLabelmap()
  {
    const auto path = FixturePath("ct3slice_labelmap.dcm");
    const auto segmentation = mitk::test::LoadSingleSegmentation(path);
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
    const auto result = mitk::test::ReadWithReaderOfMimeType(this->GetTempFilePath("stream-only.dcm"), mitk::test::DICOMSegMimeTypeName(),
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

  void TestReadFractionalFails()
  {
    AssertReadFails(FixturePath("ct3slice_fractional.dcm"), { "ct3slice_fractional.dcm", "FRACTIONAL" });
  }

  void TestReadNonDICOMFileFails()
  {
    const auto path = this->GetTempFilePath("not-dicom.dcm");
    {
      std::ofstream output(path, std::ios::binary | std::ios::trunc);
      output << "This is no DICOM file.";
    }
    AssertReaderFails(path, { "not-dicom.dcm" });
    // The mime type rejects the file, so IOUtil finds no reader for it.
    CPPUNIT_ASSERT_THROW(mitk::IOUtil::Load(path), mitk::Exception);
  }

  void TestReadNonSegmentationSOPClassFails()
  {
    // Surface Segmentation Storage shares the modality SEG with the classes the reader reads.
    const auto path = this->WriteEditedCopy("ct3slice_binary.dcm", "surface-segmentation.dcm", [](DcmDataset& dataset) {
      dataset.putAndInsertString(DCM_SOPClassUID, UID_SurfaceSegmentationStorage);
    });
    // The mime type rejects the SOP class, so only the reader itself sees the file.
    AssertReaderFails(path, { "surface-segmentation.dcm", UID_SurfaceSegmentationStorage });
  }

  void TestReadLabelmapPixelWithoutSegmentFails()
  {
    const auto path = this->WriteEditedCopy("ct3slice_labelmap.dcm", "pixel-without-segment.dcm", [](DcmDataset& dataset) {
      DcmSequenceOfItems* segments = nullptr;
      CPPUNIT_ASSERT(dataset.findAndGetSequence(DCM_SegmentSequence, segments).good());
      delete segments->remove(FindSegmentItem(dataset, 3));
    });
    AssertReadFails(path, { "pixel-without-segment.dcm", "pixel value 3" });
  }

  void TestReadLabelmapSegmentWithoutPixelsFails()
  {
    const auto path = this->WriteEditedCopy("ct3slice_labelmap.dcm", "segment-without-pixels.dcm", [](DcmDataset& dataset) {
      DcmElement* pixelData = nullptr;
      Uint8* pixels = nullptr;
      CPPUNIT_ASSERT(dataset.findAndGetElement(DCM_PixelData, pixelData).good());
      CPPUNIT_ASSERT(pixelData->getUint8Array(pixels).good());
      std::replace(pixels, pixels + pixelData->getLength(), Uint8{ 7 }, Uint8{ 0 });
    });
    AssertReadFails(path, { "segment-without-pixels.dcm", "segment 7" });
  }

  void TestReadInvalidAlgorithmTypeFails()
  {
    const auto path = this->WriteEditedCopy("ct3slice_binary.dcm", "invalid-algorithm-type.dcm", [](DcmDataset& dataset) {
      CPPUNIT_ASSERT(FindSegmentItem(dataset, 1)->putAndInsertString(DCM_SegmentAlgorithmType, "GUESSED").good());
    });
    // dcmqi throws an int for an algorithm type it does not know.
    AssertReadFails(path, { "invalid-algorithm-type.dcm", "dcmqi failed" });
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkDICOMSegmentationIOReader)
