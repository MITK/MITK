/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkIOMimeTypes.h>
#include <mitkIOUtil.h>
#include <mitkImageWriteAccessor.h>
#include <mitkLabelSetImage.h>
#include <mitkSegIOTestCapabilities.h>
#include <mitkSegIOTestComparator.h>
#include <mitkSegIOTestFixtures.h>
#include <mitkStringProperty.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <filesystem>
#include <optional>

class mitkMultiLabelSegmentationStackWriterTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkMultiLabelSegmentationStackWriterTestSuite);
  MITK_TEST(Test3D_Group_based);
  MITK_TEST(Test3D_Label_based);
  MITK_TEST(Test3D_Label_binary_based);
  MITK_TEST(TestRoundTrip3D);
  MITK_TEST(TestRoundTripWithEmptyLabel);
  MITK_TEST(TestRoundTripWithEmptyGroup);
  MITK_TEST(TestRoundTripWithoutLabels);
  MITK_TEST(TestRoundTripMaxLabelValue);
  MITK_TEST(TestRoundTripKeepsNamesakesOfFilteredKeys);
  MITK_TEST(TestWriteWithoutOptionsUsesGroupStrategy);
  MITK_TEST(TestWriteUnknownSaveStrategyFails);
  MITK_TEST(TestWriteUnknownInstanceValueFails);
  MITK_TEST(TestWriteRelativePath);
  MITK_TEST(TestWriteDottedStemKeepsDots);
  MITK_TEST(TestWriteWithoutMatchingExtensionFails);
  MITK_TEST(TestWrite4DFails);
  MITK_TEST(TestWriteWithoutGroupsFails);
  CPPUNIT_TEST_SUITE_END();

private:
  using FilePathVectorType = std::vector<std::string>;

  struct Strategy
  {
    std::string name;
    mitk::IFileWriter::Options options;
  };

  std::string m_TempPath;
  std::optional<std::filesystem::path> m_PreviousWorkingDirectory;

  std::string GetTempFilePath(const std::string& fileName)
  {
    return m_TempPath + mitk::IOUtil::GetDirectorySeparator() + fileName;
  }

  bool TempFileExists(const std::string& fileName)
  {
    return std::filesystem::exists(this->GetTempFilePath(fileName));
  }

  mitk::MultiLabelSegmentation::Pointer LoadSegmentation(const std::string& path)
  {
    auto loadedData = mitk::IOUtil::Load(GetTestDataFilePath(path));

    CPPUNIT_ASSERT_MESSAGE("Error reading segmentation group stack images", loadedData.size() == 1);

    return dynamic_cast<mitk::MultiLabelSegmentation*>(loadedData[0].GetPointer());
  }

  bool CheckStackFilesForEquality(const FilePathVectorType& refPaths, const FilePathVectorType& testPaths)
  {
    if (refPaths.size() != testPaths.size())
      mitkThrow() << "Cannot check stack images. Ref and test count mismatch.";

    bool result = true;
    for (FilePathVectorType::size_type i = 0; i < refPaths.size(); ++i)
    {
      auto refImage = mitk::IOUtil::Load<mitk::Image>(refPaths[i]);
      auto testImage = mitk::IOUtil::Load<mitk::Image>(testPaths[i]);

      if (!mitk::Equal(*refImage, *testImage, mitk::eps, true))
      {
        std::cerr << "ERROR: At list one pair of the stack is not equal. Ref path: " << refPaths[i] << "; Test path : " << testPaths[i];
        return false;
      }
    }

    return result;
  }

  static mitk::IFileWriter::Options GroupStrategy()
  {
    return { { "Save strategy", us::Any(std::string("group")) } };
  }

  static std::vector<Strategy> Strategies()
  {
    return {
      { "group", GroupStrategy() },
      { "instance, original values",
        { { "Save strategy", us::Any(std::string("instance")) }, { "Instance value", us::Any(std::string("original")) } } },
      { "instance, binary values",
        { { "Save strategy", us::Any(std::string("instance")) }, { "Instance value", us::Any(std::string("binary")) } } },
    };
  }

  static mitk::MultiLabelSegmentation::Pointer BuildRich3D()
  {
    return mitk::test::BuildRichSegmentation({ false, false, false, false });
  }

  void AssertRoundTrip(const mitk::MultiLabelSegmentation* original, const Strategy& strategy, const std::string& path)
  {
    mitk::IOUtil::Save(mitk::test::CloneKeepingUID(*original), path, strategy.options);
    const auto reloaded = mitk::test::LoadSingleSegmentation(path);

    auto capabilities = mitk::test::StackCapabilities();
    capabilities.formatName += " (" + strategy.name + ")";
    mitk::test::AssertReloadMatchesCapabilities({ original }, { reloaded.GetPointer() }, capabilities);
  }

  void AssertRoundTripInEveryStrategy(const mitk::MultiLabelSegmentation* original)
  {
    const auto strategies = Strategies();
    for (std::size_t i = 0; i < strategies.size(); ++i)
      this->AssertRoundTrip(original, strategies[i], this->GetTempFilePath("roundtrip_" + std::to_string(i) + ".mitklabel.json"));
  }

  static void AssertExceptionMentions(const std::function<void()>& action, const std::vector<std::string>& texts)
  {
    try
    {
      action();
    }
    catch (const mitk::Exception& e)
    {
      const std::string message = e.GetDescription();
      for (const auto& text : texts)
      {
        CPPUNIT_ASSERT_MESSAGE("The error should mention \"" + text + "\"; got: " + message,
                               message.find(text) != std::string::npos);
      }
      return;
    }
    CPPUNIT_FAIL("No mitk::Exception was thrown");
  }

public:

  void setUp() override
  {
    m_TempPath = mitk::IOUtil::CreateTemporaryDirectory("mitk-SegmentationStackWriterTest-XXXXXX");
  }

  void tearDown() override
  {
    if (m_PreviousWorkingDirectory.has_value())
    {
      std::filesystem::current_path(*m_PreviousWorkingDirectory);
      m_PreviousWorkingDirectory.reset();
    }

    itksys::SystemTools::RemoveADirectory(m_TempPath);
  }

  void Test3D_Group_based()
  {
    mitk::IFileWriter::Options options = { {"Save strategy", us::Any(std::string("group"))} };
    mitk::IOUtil::Save(this->LoadSegmentation("Multilabel/MultilabelSegmentation.nrrd"), this->GetTempFilePath("stack_groupbased.mitklabel.json"), options);

    FilePathVectorType refFiles = { GetTestDataFilePath("Multilabel/MultilabelSegmentation_group_0.nii.gz"),
                                    GetTestDataFilePath("Multilabel/MultilabelSegmentation_group_1.nii.gz"),
                                    GetTestDataFilePath("Multilabel/MultilabelSegmentation_group_2.nii.gz") };
    FilePathVectorType testFiles = { this->GetTempFilePath("stack_groupbased_group_0.nii.gz"),
                                    this->GetTempFilePath("stack_groupbased_group_1.nii.gz"),
                                    this->GetTempFilePath("stack_groupbased_group_2.nii.gz") };

    auto imagesAreEqual = this->CheckStackFilesForEquality(refFiles, testFiles);

    CPPUNIT_ASSERT_MESSAGE("Error stack of group based saved segmentation stack is not equal.", imagesAreEqual);
  }

  void Test3D_Label_based()
  {
    mitk::IFileWriter::Options options = { {"Save strategy", us::Any(std::string("instance"))},
                                           {"Instance value", us::Any(std::string("original"))} };
    mitk::IOUtil::Save(this->LoadSegmentation("Multilabel/MultilabelSegmentation.nrrd"), this->GetTempFilePath("stack_labelbased.mitklabel.json"), options);

    FilePathVectorType refFiles = { GetTestDataFilePath("Multilabel/MultilabelSegmentation_label_1.nii.gz"),
                                    GetTestDataFilePath("Multilabel/MultilabelSegmentation_label_2.nii.gz"),
                                    GetTestDataFilePath("Multilabel/MultilabelSegmentation_label_3.nii.gz"),
                                    GetTestDataFilePath("Multilabel/MultilabelSegmentation_label_4.nii.gz"),
                                    GetTestDataFilePath("Multilabel/MultilabelSegmentation_label_5.nii.gz"),
                                    GetTestDataFilePath("Multilabel/MultilabelSegmentation_label_6.nii.gz"),
                                    GetTestDataFilePath("Multilabel/MultilabelSegmentation_label_7.nii.gz"),
                                    GetTestDataFilePath("Multilabel/MultilabelSegmentation_label_8.nii.gz") };
    FilePathVectorType testFiles = { this->GetTempFilePath("stack_labelbased_label_1.nii.gz"),
                                    this->GetTempFilePath("stack_labelbased_label_2.nii.gz"),
                                    this->GetTempFilePath("stack_labelbased_label_3.nii.gz"),
                                    this->GetTempFilePath("stack_labelbased_label_4.nii.gz"),
                                    this->GetTempFilePath("stack_labelbased_label_5.nii.gz"),
                                    this->GetTempFilePath("stack_labelbased_label_6.nii.gz"),
                                    this->GetTempFilePath("stack_labelbased_label_7.nii.gz"),
                                    this->GetTempFilePath("stack_labelbased_label_8.nii.gz") };

    auto imagesAreEqual = this->CheckStackFilesForEquality(refFiles, testFiles);

    CPPUNIT_ASSERT_MESSAGE("Error stack of group based saved segmentation stack is not equal.", imagesAreEqual);
  }

  void Test3D_Label_binary_based()
  {
    mitk::IFileWriter::Options options = { {"Save strategy", us::Any(std::string("instance"))},
                                           {"Instance value", us::Any(std::string("binary"))} };
    mitk::IOUtil::Save(this->LoadSegmentation("Multilabel/MultilabelSegmentation.nrrd"), this->GetTempFilePath("stack_binarybased.mitklabel.json"), options);

    FilePathVectorType refFiles = { GetTestDataFilePath("Multilabel/MultilabelSegmentation_binary_label_1.nii.gz"),
                                    GetTestDataFilePath("Multilabel/MultilabelSegmentation_binary_label_2.nii.gz"),
                                    GetTestDataFilePath("Multilabel/MultilabelSegmentation_binary_label_3.nii.gz"),
                                    GetTestDataFilePath("Multilabel/MultilabelSegmentation_binary_label_4.nii.gz"),
                                    GetTestDataFilePath("Multilabel/MultilabelSegmentation_binary_label_5.nii.gz"),
                                    GetTestDataFilePath("Multilabel/MultilabelSegmentation_binary_label_6.nii.gz"),
                                    GetTestDataFilePath("Multilabel/MultilabelSegmentation_binary_label_7.nii.gz"),
                                    GetTestDataFilePath("Multilabel/MultilabelSegmentation_binary_label_8.nii.gz") };
    FilePathVectorType testFiles = { this->GetTempFilePath("stack_binarybased_label_1.nii.gz"),
                                    this->GetTempFilePath("stack_binarybased_label_2.nii.gz"),
                                    this->GetTempFilePath("stack_binarybased_label_3.nii.gz"),
                                    this->GetTempFilePath("stack_binarybased_label_4.nii.gz"),
                                    this->GetTempFilePath("stack_binarybased_label_5.nii.gz"),
                                    this->GetTempFilePath("stack_binarybased_label_6.nii.gz"),
                                    this->GetTempFilePath("stack_binarybased_label_7.nii.gz"),
                                    this->GetTempFilePath("stack_binarybased_label_8.nii.gz") };

    auto imagesAreEqual = this->CheckStackFilesForEquality(refFiles, testFiles);

    CPPUNIT_ASSERT_MESSAGE("Error stack of group based saved segmentation stack is not equal.", imagesAreEqual);
  }

  void TestRoundTrip3D()
  {
    this->AssertRoundTripInEveryStrategy(BuildRich3D());
  }

  void TestRoundTripWithEmptyLabel()
  {
    this->AssertRoundTripInEveryStrategy(mitk::test::BuildRichSegmentation({ false, false, true, false }));
  }

  void TestRoundTripWithEmptyGroup()
  {
    this->AssertRoundTripInEveryStrategy(mitk::test::BuildRichSegmentation({ false, false, false, true }));
  }

  void TestRoundTripWithoutLabels()
  {
    // Without labels, the instance strategy has no label image to initialise the reload from.
    auto segmentation = BuildRich3D();
    for (const auto value : segmentation->GetAllLabelValues())
      segmentation->RemoveLabel(value);
    this->AssertRoundTripInEveryStrategy(segmentation);
  }

  void TestRoundTripMaxLabelValue()
  {
    auto segmentation = BuildRich3D();
    segmentation->AddLabel(mitk::Label::New(mitk::Label::MAX_LABEL_VALUE, "Maximum value"), 1, true, false);
    {
      // Index 0 lies outside every box of group 1.
      mitk::ImageWriteAccessor accessor(segmentation->GetGroupImage(1));
      static_cast<mitk::Label::PixelType*>(accessor.GetData())[0] = mitk::Label::MAX_LABEL_VALUE;
    }
    this->AssertRoundTripInEveryStrategy(segmentation);
  }

  void TestRoundTripKeepsNamesakesOfFilteredKeys()
  {
    // The writer leaves out "path", "modality" and "org.mitk.uid"; other keys merely starting
    // with them are user content.
    auto segmentation = BuildRich3D();
    segmentation->SetProperty("pathology.grade", mitk::StringProperty::New("G2"));
    segmentation->SetProperty("pathologist", mitk::StringProperty::New("unknown"));
    segmentation->SetProperty("modality.source", mitk::StringProperty::New("CT"));
    this->AssertRoundTrip(segmentation, Strategies().front(), this->GetTempFilePath("namesakes.mitklabel.json"));
  }

  void TestWriteWithoutOptionsUsesGroupStrategy()
  {
    mitk::IOUtil::Save(BuildRich3D(), this->GetTempFilePath("default.mitklabel.json"));

    CPPUNIT_ASSERT_MESSAGE("The default strategy writes group images", this->TempFileExists("default_group_0.nii.gz"));
    CPPUNIT_ASSERT_MESSAGE("The default strategy writes group images", this->TempFileExists("default_group_1.nii.gz"));
    CPPUNIT_ASSERT_MESSAGE("The default strategy writes no label images", !this->TempFileExists("default_label_7.nii.gz"));
  }

  void TestWriteUnknownSaveStrategyFails()
  {
    const mitk::IFileWriter::Options options = { { "Save strategy", us::Any(std::string("label")) } };
    AssertExceptionMentions([&]() { mitk::IOUtil::Save(BuildRich3D(), this->GetTempFilePath("unknown.mitklabel.json"), options); },
                            { "Unknown value \"label\" of the option \"Save strategy\"" });
  }

  void TestWriteUnknownInstanceValueFails()
  {
    const mitk::IFileWriter::Options options = { { "Save strategy", us::Any(std::string("instance")) },
                                                 { "Instance value", us::Any(std::string("mask")) } };
    AssertExceptionMentions([&]() { mitk::IOUtil::Save(BuildRich3D(), this->GetTempFilePath("unknown.mitklabel.json"), options); },
                            { "Unknown value \"mask\" of the option \"Instance value\"" });
  }

  void TestWriteRelativePath()
  {
    m_PreviousWorkingDirectory = std::filesystem::current_path();
    std::filesystem::current_path(m_TempPath);

    mitk::IOUtil::Save(BuildRich3D(), "relative.mitklabel.json", GroupStrategy());

    CPPUNIT_ASSERT_MESSAGE("The meta file lands in the working directory", this->TempFileExists("relative.mitklabel.json"));
    CPPUNIT_ASSERT_MESSAGE("The group images land in the working directory", this->TempFileExists("relative_group_0.nii.gz"));
  }

  void TestWriteDottedStemKeepsDots()
  {
    const auto original = BuildRich3D();
    this->AssertRoundTrip(original, Strategies().front(), this->GetTempFilePath("case.001.mitklabel.json"));

    CPPUNIT_ASSERT_MESSAGE("The group images keep the dotted stem", this->TempFileExists("case.001_group_0.nii.gz"));
    CPPUNIT_ASSERT_MESSAGE("Nothing is written under the stem cut at the first dot", !this->TempFileExists("case_group_0.nii.gz"));
  }

  void TestWriteWithoutMatchingExtensionFails()
  {
    // An explicit mime type without adding its extension hands the writer a path whose
    // file name has none of the stack's extensions, so there is no stem to derive.
    const auto mimeType = mitk::IOMimeTypes::DEFAULT_BASE_NAME() + ".multilabel.segmentation.stack.nifti";
    AssertExceptionMentions(
      [&]() { mitk::IOUtil::Save(BuildRich3D(), mimeType, this->GetTempFilePath("stack"), GroupStrategy(), false); },
      { "has none of the extensions" });
  }

  void TestWrite4DFails()
  {
    const auto segmentation = mitk::test::BuildRichSegmentation({ true, false, false, false });
    AssertExceptionMentions(
      [&]() { mitk::IOUtil::Save(segmentation, this->GetTempFilePath("dynamic.mitklabel.json"), GroupStrategy()); },
      { "No suitable writer" });
  }

  void TestWriteWithoutGroupsFails()
  {
    auto segmentation = BuildRich3D();
    while (segmentation->GetNumberOfGroups() > 0)
      segmentation->RemoveGroup(0);

    AssertExceptionMentions(
      [&]() { mitk::IOUtil::Save(segmentation, this->GetTempFilePath("no-groups.mitklabel.json"), GroupStrategy()); },
      { "no groups" });
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkMultiLabelSegmentationStackWriter)
