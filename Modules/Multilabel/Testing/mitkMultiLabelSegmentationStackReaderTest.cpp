/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkCoreServices.h>
#include <mitkIOUtil.h>
#include <mitkIPropertyPersistence.h>
#include <mitkLabelSetImage.h>
#include <mitkSegIOTestCapabilities.h>
#include <mitkSegIOTestComparator.h>
#include <mitkSegIOTestFixtures.h>
#include <mitkStringProperty.h>
#include <mitkTemporoSpatialStringProperty.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <filesystem>
#include <fstream>

class mitkMultiLabelSegmentationStackReaderTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkMultiLabelSegmentationStackReaderTestSuite);
  MITK_TEST(Test3D_Group_based);
  MITK_TEST(Test3D_Label_based);
  MITK_TEST(Test3D_Label_binary_based);
  MITK_TEST(Test3D_mixed);
  MITK_TEST(Test3D_Custom_prop_roundtrip);
  MITK_TEST(Test3D_Outdated_label_properties_are_discarded);
  MITK_TEST(TestReadAbsoluteFilePath);
  MITK_TEST(TestReadVersionAbove4Fails);
  MITK_TEST(TestReadWrongTypeFindsNoReader);
  MITK_TEST(TestReadMalformedJsonFails);
  MITK_TEST(TestReadWithoutGroupsFails);
  MITK_TEST(TestReadEmptyGroupsFails);
  MITK_TEST(TestReadMalformedLabelFails);
  MITK_TEST(TestReadUnopenableFileFails);
  MITK_TEST(TestReadMissingImageFails);
  MITK_TEST(TestReadUnreadableImageFails);
  CPPUNIT_TEST_SUITE_END();

private:
  std::string m_TempPath;

  static const std::string& ReaderDescription()
  {
    static const std::string description = "MITK Multilabel Segmentation Stack";
    return description;
  }

  static mitk::IFileWriter::Options GroupStrategy()
  {
    return { { "Save strategy", us::Any(std::string("group")) } };
  }

  /**
   * MultilabelSegmentation.nrrd as the native reader yields it, plus the segmentation
   * properties that the MITK-Data stacks of the same content carry in addition,
   * transcribed from their JSON.
   */
  mitk::MultiLabelSegmentation::Pointer ExpectedForStackFixtures()
  {
    auto expected = this->LoadSegmentation("Multilabel/MultilabelSegmentation.nrrd");
    expected->SetProperty("custom.test.information", mitk::StringProperty::New("Custom test content"));
    const std::vector<std::pair<std::string, std::string>> dicomTags = {
      { "DICOM.0008.0060", "SEG" },          { "DICOM.0008.103E", "MITK Segmentation" },
      { "DICOM.0012.0050", "0" },            { "DICOM.0012.0060", "Unknown" },
      { "DICOM.0012.0071", "Session 1" },    { "DICOM.0070.0084", "MITK" } };
    for (const auto& [key, value] : dicomTags)
      expected->SetProperty(key, mitk::TemporoSpatialStringProperty::New(value));
    return expected;
  }

  void AssertMatchesStackFixtureExpectation(const mitk::MultiLabelSegmentation* segmentation, const std::string& fixture)
  {
    auto options = mitk::test::StackCapabilities().options;
    // Neither the stacks nor the NRRD carry a series instance UID, so every load mints its own.
    options.notComparedKeys.insert("DICOM.0020.000E");

    const auto differences = mitk::test::CompareSegmentations(*this->ExpectedForStackFixtures(), *segmentation, options);
    CPPUNIT_ASSERT_MESSAGE(fixture + " differs from its expectation:" + mitk::test::JoinDifferences(differences),
                           differences.empty());
  }

  /** Saves the 3D rich fixture as a stack with the group strategy and returns the path of its meta file. */
  std::string SaveValidStack(const std::string& fileName)
  {
    const auto path = this->GetTempFilePath(fileName);
    mitk::IOUtil::Save(mitk::test::BuildRichSegmentation({ false, false, false, false }), path, GroupStrategy());
    return path;
  }

  /** The reader itself must throw mitk::Exception mentioning text; IOUtil must report the failure too. */
  static void AssertReadFails(const std::string& path, const std::string& text)
  {
    try
    {
      mitk::test::ReadWithReader(path, ReaderDescription());
      CPPUNIT_FAIL("Reading " + path + " did not fail");
    }
    catch (const mitk::Exception& e)
    {
      CPPUNIT_ASSERT_MESSAGE("The reader's exception should mention \"" + text + "\"; got: " + e.GetDescription(),
                             std::string(e.GetDescription()).find(text) != std::string::npos);
    }
    CPPUNIT_ASSERT_THROW(mitk::IOUtil::Load(path), mitk::Exception);
  }

  /**
   * Like AssertReadFails for files the reader rates Unsupported (unopenable or not JSON):
   * IOUtil then finds no reader at all, so the reader is called without its confidence check.
   */
  static void AssertReadFailsIgnoringConfidence(const std::string& path, const std::string& text)
  {
    try
    {
      mitk::test::ReadWithReaderIgnoringConfidence(path, ReaderDescription());
      CPPUNIT_FAIL("Reading " + path + " did not fail");
    }
    catch (const mitk::Exception& e)
    {
      CPPUNIT_ASSERT_MESSAGE("The reader's exception should mention \"" + text + "\"; got: " + e.GetDescription(),
                             std::string(e.GetDescription()).find(text) != std::string::npos);
    }
    CPPUNIT_ASSERT_THROW(mitk::IOUtil::Load(path), mitk::Exception);
  }

  void WriteTextFile(const std::string& path, const std::string& content)
  {
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    output << content;
  }

public:

  std::string GetTempFilePath(const std::string& fileName)
  {
    return m_TempPath + mitk::IOUtil::GetDirectorySeparator() + fileName;
  }

  mitk::MultiLabelSegmentation::Pointer LoadSegmentation(const std::string& path)
  {
    auto loadedData = mitk::IOUtil::Load(GetTestDataFilePath(path));

    CPPUNIT_ASSERT_MESSAGE("Error reading segmentation group stack images", loadedData.size() == 1);

    return dynamic_cast<mitk::MultiLabelSegmentation*>(loadedData[0].GetPointer());
  }

  void setUp() override
  {
    m_TempPath = mitk::IOUtil::CreateTemporaryDirectory("mitk-SegmentationStackReaderTest-XXXXXX");
  }

  void tearDown() override
  {
    // The reader registers a persistence info for every property it reads, process-wide.
    mitk::CoreServicePointer<mitk::IPropertyPersistence> persistence(mitk::CoreServices::GetPropertyPersistence());
    persistence->RemoveInfo(mitk::test::RichFixtureCustomPropertyName());

    std::error_code ec;
    std::filesystem::remove_all(m_TempPath, ec);
  }

  void Test3D_Group_based()
  {
    auto seg = this->LoadSegmentation("Multilabel/MultilabelSegmentation_group.mitklabel.json");

    CPPUNIT_ASSERT_MESSAGE("Error reading segmentation group stack. No Segmentation returned", seg != nullptr);

    this->AssertMatchesStackFixtureExpectation(seg, "MultilabelSegmentation_group.mitklabel.json");
  }

  void Test3D_Label_based()
  {
    auto seg = this->LoadSegmentation("Multilabel/MultilabelSegmentation_label.mitklabel.json");

    CPPUNIT_ASSERT_MESSAGE("Error reading segmentation label stack. No Segmentation returned", seg != nullptr);

    this->AssertMatchesStackFixtureExpectation(seg, "MultilabelSegmentation_label.mitklabel.json");
  }

  void Test3D_Label_binary_based()
  {
    auto seg = this->LoadSegmentation("Multilabel/MultilabelSegmentation_binary_label.mitklabel.json");

    CPPUNIT_ASSERT_MESSAGE("Error reading segmentation label stack. No Segmentation returned", seg != nullptr);

    this->AssertMatchesStackFixtureExpectation(seg, "MultilabelSegmentation_binary_label.mitklabel.json");
  }

  void Test3D_mixed()
  {
    auto seg = this->LoadSegmentation("Multilabel/MultilabelSegmentation_mixed.mitklabel.json");

    CPPUNIT_ASSERT_MESSAGE("Error reading segmentation label stack. No Segmentation returned", seg != nullptr);

    this->AssertMatchesStackFixtureExpectation(seg, "MultilabelSegmentation_mixed.mitklabel.json");
  }

  void Test3D_Custom_prop_roundtrip()
  {
    // A custom property reaches a native NRRD header only through a persistence info. Nothing
    // here registers one for the fixture's property; the stack reader does it for every property
    // it reads, so a stack converted to native NRRD keeps its custom properties.
    const auto original = mitk::test::BuildRichSegmentation({ false, false, false, false });

    const auto stackPath = this->GetTempFilePath("seg.mitklabel.json");
    mitk::IOUtil::Save(original, stackPath, GroupStrategy());
    const auto interimSeg = this->LoadSegmentation(stackPath);

    const auto nrrdPath = this->GetTempFilePath("seg.nrrd");
    mitk::IOUtil::Save(interimSeg, nrrdPath);
    const auto testSeg = this->LoadSegmentation(nrrdPath);

    auto capabilities = mitk::test::StackCapabilities();
    const auto native = mitk::test::NativeNrrdCapabilities();
    capabilities.formatName = "stack, then native NRRD";
    capabilities.options.geometryTolerance = std::max(capabilities.options.geometryTolerance, native.options.geometryTolerance);
    capabilities.options.timeTolerance = std::max(capabilities.options.timeTolerance, native.options.timeTolerance);
    capabilities.losses.insert(capabilities.losses.end(), native.losses.begin(), native.losses.end());

    mitk::test::AssertReloadMatchesCapabilities({ original.GetPointer() }, { testSeg.GetPointer() }, capabilities);
  }

  void Test3D_Outdated_label_properties_are_discarded()
  {
    // A label loaded from a file of MITK before 2025.12 carried this stamp, and later
    // versions wrote it into stacks too.
    auto segmentation = this->LoadSegmentation("Multilabel/MultilabelSegmentation.nrrd");
    const auto value = segmentation->GetAllLabelValues().front();
    segmentation->GetLabel(value)->SetProperty("DICOM.0062.0002.0062.0008",
                                               mitk::TemporoSpatialStringProperty::New("SEMIAUTOMATIC"));
    const auto path = this->GetTempFilePath("outdated.mitklabel.json");
    mitk::IOUtil::Save(segmentation, path, GroupStrategy());

    mitk::test::WarningRecorder warnings("DICOM.0062.0002.");
    const auto loaded = mitk::IOUtil::Load(path);
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), loaded.size());
    const auto* reloaded = dynamic_cast<const mitk::MultiLabelSegmentation*>(loaded.front().GetPointer());
    CPPUNIT_ASSERT(nullptr != reloaded);

    CPPUNIT_ASSERT_MESSAGE("Outdated label properties survived the read",
                           mitk::test::FindOutdatedLabelProperties(*reloaded).empty());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("One warning per file", std::size_t(1), warnings.GetWarnings().size());
    CPPUNIT_ASSERT_MESSAGE("The warning names the file: " + warnings.GetWarnings().front(),
                           warnings.GetWarnings().front().find("outdated.mitklabel.json") != std::string::npos);
  }

  void TestReadAbsoluteFilePath()
  {
    const auto original = mitk::test::BuildRichSegmentation({ false, false, false, false });
    const auto path = this->GetTempFilePath("absolute.mitklabel.json");
    mitk::IOUtil::Save(original, path, GroupStrategy());

    // Moved into a subdirectory, the meta file resolves its images only through absolute paths.
    auto content = mitk::test::ReadJsonFile(path);
    for (auto& group : content["groups"])
    {
      const auto fileName = std::filesystem::path(group["_file"].get<std::string>()).filename().string();
      group["_file"] = std::filesystem::absolute(this->GetTempFilePath(fileName)).generic_string();
    }
    std::filesystem::create_directory(this->GetTempFilePath("moved"));
    const auto movedPath = this->GetTempFilePath("moved/absolute.mitklabel.json");
    mitk::test::WriteJsonFile(movedPath, content);

    const auto reloaded = this->LoadSegmentation(movedPath);
    mitk::test::AssertReloadMatchesCapabilities({ original.GetPointer() }, { reloaded.GetPointer() },
                                                mitk::test::StackCapabilities());
  }

  void TestReadVersionAbove4Fails()
  {
    const auto path = this->SaveValidStack("version.mitklabel.json");
    auto content = mitk::test::ReadJsonFile(path);
    content["version"] = 5;
    mitk::test::WriteJsonFile(path, content);

    AssertReadFails(path, "unsupported version");
  }

  void TestReadWrongTypeFindsNoReader()
  {
    const auto path = this->SaveValidStack("preset.mitklabel.json");
    auto content = mitk::test::ReadJsonFile(path);
    content["type"] = "org.mitk.multilabel.segmentation.preset";
    mitk::test::WriteJsonFile(path, content);

    CPPUNIT_ASSERT_THROW(mitk::IOUtil::Load(path), mitk::Exception);
  }

  void TestReadMalformedJsonFails()
  {
    const auto path = this->GetTempFilePath("malformed.mitklabel.json");
    this->WriteTextFile(path, R"({"version": 4, "type": "org.mitk.multilabel.segmentation.stack", "groups": [)");

    AssertReadFailsIgnoringConfidence(path, "malformed.mitklabel.json");
  }

  void TestReadWithoutGroupsFails()
  {
    const auto path = this->GetTempFilePath("no-groups.mitklabel.json");
    this->WriteTextFile(path, R"({"version": 4, "type": "org.mitk.multilabel.segmentation.stack"})");

    AssertReadFails(path, "no-groups.mitklabel.json");
  }

  void TestReadEmptyGroupsFails()
  {
    const auto path = this->GetTempFilePath("empty-groups.mitklabel.json");
    this->WriteTextFile(path, R"({"version": 4, "type": "org.mitk.multilabel.segmentation.stack", "groups": []})");

    AssertReadFails(path, "empty-groups.mitklabel.json");
  }

  void TestReadMalformedLabelFails()
  {
    const auto path = this->SaveValidStack("malformed-label.mitklabel.json");
    auto content = mitk::test::ReadJsonFile(path);
    content["groups"][0]["labels"][0]["value"] = "seven";
    mitk::test::WriteJsonFile(path, content);

    AssertReadFails(path, "malformed-label.mitklabel.json");
  }

  void TestReadUnopenableFileFails()
  {
    AssertReadFailsIgnoringConfidence(this->GetTempFilePath("missing.mitklabel.json"), "missing.mitklabel.json");
  }

  void TestReadMissingImageFails()
  {
    const auto path = this->SaveValidStack("missing-image.mitklabel.json");
    std::filesystem::remove(this->GetTempFilePath("missing-image_group_1.nii.gz"));

    AssertReadFails(path, "missing-image_group_1.nii.gz");
  }

  void TestReadUnreadableImageFails()
  {
    const auto path = this->SaveValidStack("unreadable-image.mitklabel.json");
    this->WriteTextFile(this->GetTempFilePath("unreadable-image_group_1.nii.gz"), "not an image");

    AssertReadFails(path, "unreadable-image_group_1.nii.gz");
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkMultiLabelSegmentationStackReader)
