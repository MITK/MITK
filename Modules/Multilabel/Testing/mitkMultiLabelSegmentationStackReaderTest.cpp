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

class mitkMultiLabelSegmentationStackReaderTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkMultiLabelSegmentationStackReaderTestSuite);
  MITK_TEST(Test3D_Group_based);
  MITK_TEST(Test3D_Label_based);
  MITK_TEST(Test3D_Label_binary_based);
  MITK_TEST(Test3D_mixed);
  MITK_TEST(Test3D_Custom_prop_roundtrip);
  CPPUNIT_TEST_SUITE_END();

private:
  std::string m_TempPath;

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
  }
  
  void Test3D_Group_based()
  {
    auto ref = LoadSegmentation("Multilabel/MultilabelSegmentation.nrrd");
    auto seg = LoadSegmentation("Multilabel/MultilabelSegmentation_group.mitklabel.json");

    CPPUNIT_ASSERT_MESSAGE("Error reading segmentation group stack. No Segmentation returned", seg != nullptr);

    CPPUNIT_ASSERT_MESSAGE("Error Loaded stack and reference are not equal", mitk::Equal(*ref, *seg, mitk::eps, true));
  }

  void Test3D_Label_based()
  {
    auto ref = LoadSegmentation("Multilabel/MultilabelSegmentation.nrrd");
    auto seg = LoadSegmentation("Multilabel/MultilabelSegmentation_label.mitklabel.json");

    CPPUNIT_ASSERT_MESSAGE("Error reading segmentation label stack. No Segmentation returned", seg != nullptr);

    CPPUNIT_ASSERT_MESSAGE("Error Loaded stack and reference are not equal", mitk::Equal(*ref, *seg, mitk::eps, true));
  }

  void Test3D_Label_binary_based()
  {
    auto ref = LoadSegmentation("Multilabel/MultilabelSegmentation.nrrd");
    auto seg = LoadSegmentation("Multilabel/MultilabelSegmentation_binary_label.mitklabel.json");

    CPPUNIT_ASSERT_MESSAGE("Error reading segmentation label stack. No Segmentation returned", seg != nullptr);

    CPPUNIT_ASSERT_MESSAGE("Error Loaded stack and reference are not equal", mitk::Equal(*ref, *seg, mitk::eps, true));
  }

  void Test3D_mixed()
  {
    auto ref = LoadSegmentation("Multilabel/MultilabelSegmentation.nrrd");
    auto seg = LoadSegmentation("Multilabel/MultilabelSegmentation_mixed.mitklabel.json");

    CPPUNIT_ASSERT_MESSAGE("Error reading segmentation label stack. No Segmentation returned", seg != nullptr);

    CPPUNIT_ASSERT_MESSAGE("Error Loaded stack and reference are not equal", mitk::Equal(*ref, *seg, mitk::eps, true));
  }

  void Test3D_Custom_prop_roundtrip()
  {
    auto ref = LoadSegmentation("Multilabel/MultilabelSegmentation.nrrd");
    std::string propName = "custom.test.prop";
    std::string propValue = "my test value";
    ref->SetProperty(propName, mitk::StringProperty::New(propValue));

    //save as stack and load again to check if property persistence for custom prop is set
    auto savePath = GetTempFilePath("seg.mitklabel.json");
    mitk::IOUtil::Save(ref, savePath);
    auto interimSeg = LoadSegmentation(savePath);

    //now save as nrrd and load to check if custom prop is still there
    savePath = GetTempFilePath("seg.nrrd");
    mitk::IOUtil::Save(interimSeg, savePath);
    auto testSeg = LoadSegmentation(savePath);

    auto loadedProp = testSeg->GetProperty(propName.c_str());
    CPPUNIT_ASSERT_MESSAGE("Error custom property was not persisted.", loadedProp.IsNotNull());

    auto loadedStringProp = dynamic_cast<mitk::StringProperty*>(loadedProp.GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Error custom property was persisted as wrong type.", loadedStringProp != nullptr);
    CPPUNIT_ASSERT_MESSAGE("Error persisted custom property has wrong value.", loadedStringProp->GetValue() == propValue);
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkMultiLabelSegmentationStackReader)
