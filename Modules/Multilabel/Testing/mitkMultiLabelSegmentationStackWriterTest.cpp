/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkIOUtil.h>
#include <mitkLabelSetImage.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>
#include <mitkArbitraryTimeGeometry.h>
#include <mitkCoreServices.h>
#include <mitkPropertyPersistenceInfo.h>
#include <mitkIPropertyPersistence.h>

class mitkMultiLabelSegmentationStackWriterTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkMultiLabelSegmentationStackWriterTestSuite);
  MITK_TEST(Test3D_Group_based);
  MITK_TEST(Test3D_Label_based);
  MITK_TEST(Test3D_Label_binary_based);
  MITK_TEST(Test3D_Group_roundtrip);
  MITK_TEST(Test3D_Label_roundtrip);
  MITK_TEST(Test3D_Label_binary_roundtrip);
  CPPUNIT_TEST_SUITE_END();

private:
  using FilePathVectorType = std::vector<std::string>;
  using ImageVectorType = std::vector<mitk::Image::Pointer>;

  std::string m_TempPath;
  mitk::MultiLabelSegmentation::Pointer m_Segmentation;

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

public:

  void setUp() override
  {
    m_TempPath = mitk::IOUtil::CreateTemporaryDirectory("mitk-SegmentationStackWriterTest-XXXXXX");
    m_Segmentation = LoadSegmentation("Multilabel/MultilabelSegmentation.nrrd");
  }

  void tearDown() override
  {
    itksys::SystemTools::RemoveADirectory(m_TempPath);
  }
  
  void Test3D_Group_based()
  {
    mitk::IFileWriter::Options options = { {"Save strategy", us::Any(std::string("group"))} };
    mitk::IOUtil::Save(m_Segmentation, GetTempFilePath("stack_groupbased.mitklabel.json"), options);

    FilePathVectorType refFiles = { GetTestDataFilePath("Multilabel/MultilabelSegmentation_group_0.nii.gz"),
                                    GetTestDataFilePath("Multilabel/MultilabelSegmentation_group_1.nii.gz"),
                                    GetTestDataFilePath("Multilabel/MultilabelSegmentation_group_0.nii.gz") };
    FilePathVectorType testFiles = { GetTempFilePath("stack_groupbased_group_0.nii.gz"),
                                    GetTempFilePath("stack_groupbased_group_1.nii.gz"),
                                    GetTempFilePath("stack_groupbased_group_0.nii.gz") };

    auto imagesAreEqual = CheckStackFilesForEquality(refFiles, testFiles);

    CPPUNIT_ASSERT_MESSAGE("Error stack of group based saved segmentation stack is not equal.", imagesAreEqual);
  }

  void Test3D_Label_based()
  {
    mitk::IFileWriter::Options options = { {"Save strategy", us::Any(std::string("instance"))},
                                           {"Instance value", us::Any(std::string("original"))} };
    mitk::IOUtil::Save(m_Segmentation, GetTempFilePath("stack_labelbased.mitklabel.json"), options);

    FilePathVectorType refFiles = { GetTestDataFilePath("Multilabel/MultilabelSegmentation_label_1.nii.gz"),
                                    GetTestDataFilePath("Multilabel/MultilabelSegmentation_label_2.nii.gz"),
                                    GetTestDataFilePath("Multilabel/MultilabelSegmentation_label_3.nii.gz"),
                                    GetTestDataFilePath("Multilabel/MultilabelSegmentation_label_4.nii.gz"),
                                    GetTestDataFilePath("Multilabel/MultilabelSegmentation_label_5.nii.gz"),
                                    GetTestDataFilePath("Multilabel/MultilabelSegmentation_label_6.nii.gz"),
                                    GetTestDataFilePath("Multilabel/MultilabelSegmentation_label_7.nii.gz"),
                                    GetTestDataFilePath("Multilabel/MultilabelSegmentation_label_8.nii.gz") };
    FilePathVectorType testFiles = { GetTempFilePath("stack_labelbased_label_1.nii.gz"),
                                    GetTempFilePath("stack_labelbased_label_2.nii.gz"),
                                    GetTempFilePath("stack_labelbased_label_3.nii.gz"),
                                    GetTempFilePath("stack_labelbased_label_4.nii.gz"),
                                    GetTempFilePath("stack_labelbased_label_5.nii.gz"),
                                    GetTempFilePath("stack_labelbased_label_6.nii.gz"),
                                    GetTempFilePath("stack_labelbased_label_7.nii.gz"),
                                    GetTempFilePath("stack_labelbased_label_8.nii.gz") };

    auto imagesAreEqual = CheckStackFilesForEquality(refFiles, testFiles);

    CPPUNIT_ASSERT_MESSAGE("Error stack of group based saved segmentation stack is not equal.", imagesAreEqual);
  }

  void Test3D_Label_binary_based()
  {
    mitk::IFileWriter::Options options = { {"Save strategy", us::Any(std::string("instance"))},
                                           {"Instance value", us::Any(std::string("binary"))} };
    mitk::IOUtil::Save(m_Segmentation, GetTempFilePath("stack_binarybased.mitklabel.json"), options);

    FilePathVectorType refFiles = { GetTestDataFilePath("Multilabel/MultilabelSegmentation_binary_label_1.nii.gz"),
                                    GetTestDataFilePath("Multilabel/MultilabelSegmentation_binary_label_2.nii.gz"),
                                    GetTestDataFilePath("Multilabel/MultilabelSegmentation_binary_label_3.nii.gz"),
                                    GetTestDataFilePath("Multilabel/MultilabelSegmentation_binary_label_4.nii.gz"),
                                    GetTestDataFilePath("Multilabel/MultilabelSegmentation_binary_label_5.nii.gz"),
                                    GetTestDataFilePath("Multilabel/MultilabelSegmentation_binary_label_6.nii.gz"),
                                    GetTestDataFilePath("Multilabel/MultilabelSegmentation_binary_label_7.nii.gz"),
                                    GetTestDataFilePath("Multilabel/MultilabelSegmentation_binary_label_8.nii.gz") };
    FilePathVectorType testFiles = { GetTempFilePath("stack_binarybased_label_1.nii.gz"),
                                    GetTempFilePath("stack_binarybased_label_2.nii.gz"),
                                    GetTempFilePath("stack_binarybased_label_3.nii.gz"),
                                    GetTempFilePath("stack_binarybased_label_4.nii.gz"),
                                    GetTempFilePath("stack_binarybased_label_5.nii.gz"),
                                    GetTempFilePath("stack_binarybased_label_6.nii.gz"),
                                    GetTempFilePath("stack_binarybased_label_7.nii.gz"),
                                    GetTempFilePath("stack_binarybased_label_8.nii.gz") };

    auto imagesAreEqual = CheckStackFilesForEquality(refFiles, testFiles);

    CPPUNIT_ASSERT_MESSAGE("Error stack of group based saved segmentation stack is not equal.", imagesAreEqual);
  }

  void Test3D_Group_roundtrip()
  {
    mitk::IFileWriter::Options options = { {"Save strategy", us::Any(std::string("group"))} };
    mitk::IOUtil::Save(m_Segmentation, GetTempFilePath("stack.mitklabel.json"), options);

    auto tempSeg = LoadSegmentation(GetTempFilePath("stack.mitklabel.json"));

    CPPUNIT_ASSERT_MESSAGE("Error saved and loaded segmentation stack is not equal.", mitk::Equal(*m_Segmentation, *tempSeg, mitk::eps, true));
  }

  void Test3D_Label_roundtrip()
  {
    mitk::IFileWriter::Options options = { {"Save strategy", us::Any(std::string("instance"))},
                                           {"Instance value", us::Any(std::string("original"))} };
    mitk::IOUtil::Save(m_Segmentation, GetTempFilePath("stack.mitklabel.json"), options);

    auto tempSeg = LoadSegmentation(GetTempFilePath("stack.mitklabel.json"));

    CPPUNIT_ASSERT_MESSAGE("Error saved and loaded segmentation stack is not equal.", mitk::Equal(*m_Segmentation, *tempSeg, mitk::eps, true));
  }

  void Test3D_Label_binary_roundtrip()
  {
    mitk::IFileWriter::Options options = { {"Save strategy", us::Any(std::string("instance"))},
                                           {"Instance value", us::Any(std::string("binary"))} };
    mitk::IOUtil::Save(m_Segmentation, GetTempFilePath("stack.mitklabel.json"), options);

    auto tempSeg = LoadSegmentation(GetTempFilePath("stack.mitklabel.json"));

    CPPUNIT_ASSERT_MESSAGE("Error saved and loaded segmentation stack is not equal.", mitk::Equal(*m_Segmentation, *tempSeg, mitk::eps, true));
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkMultiLabelSegmentationStackWriter)
