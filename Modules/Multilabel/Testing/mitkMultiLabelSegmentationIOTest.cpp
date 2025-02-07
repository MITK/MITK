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

class mitkMultiLabelSegmentationIOTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkMultiLabelSegmentationIOTestSuite);
  MITK_TEST(TestReadEmptyMultiLabelSegmentation);
  MITK_TEST(TestReadEmptyMultiLabelSegmentation_withNoMetaInformation);
  MITK_TEST(TestReadEmptyMultiLabelSegmentation_withNoMetaInformation_butContent);
  CPPUNIT_TEST_SUITE_END();

private:
  mitk::MultiLabelSegmentation::ConstLabelVectorType m_labelSet1;
  mitk::MultiLabelSegmentation::ConstLabelVectorType m_labelSet2;
  mitk::MultiLabelSegmentation::ConstLabelVectorType m_labelSet2_adapted;

public:
  mitk::Label::Pointer GenerateLabel(mitk::Label::PixelType value, const std::string& name, float r, float g, float b) const
  {
    auto label = mitk::Label::New(value, name);
    mitk::Color color;
    color.SetRed(r);
    color.SetGreen(g);
    color.SetBlue(b);
    label->SetColor(color);

    return label;
  }

  void setUp() override
  {
    auto label = GenerateLabel(1, "Label 1", 0.745098054f, 0.f, 0.196078435f);
    auto label2 = GenerateLabel(2, "Label 2", 0.952941179, 0.764705896, 0);
    m_labelSet1 = { label, label2 };

    label = GenerateLabel(1, "Label 3", 0.552941203, 0.713725507, 0);
    label2 = GenerateLabel(2, "Label 4", 0.631372571, 0.792156875, 0.945098042);
    auto label3 = GenerateLabel(3, "Label 5", 0.639215708, 0.250980407, 0.725490212);
    m_labelSet2 = { label, label2, label3 };

    label = GenerateLabel(3, "Label 3", 0.552941203, 0.713725507, 0);
    label2 = GenerateLabel(4, "Label 4", 0.631372571, 0.792156875, 0.945098042);
    label3 = GenerateLabel(5, "Label 5", 0.639215708, 0.250980407, 0.725490212);
    m_labelSet2_adapted = { label, label2, label3 };
  }

  void tearDown() override
  {
    m_labelSet1.clear();
    m_labelSet2.clear();
    m_labelSet2_adapted.clear();
  }
  
  void TestReadEmptyMultiLabelSegmentation()
  {
    auto testImages = mitk::IOUtil::Load(GetTestDataFilePath("Multilabel/EmptyMultiLabelSegmentation_labels_meta.nrrd"));

    CPPUNIT_ASSERT_MESSAGE("Error reading empty MultiLabel segmentation", testImages.size()==1);

    auto lsimage1 = dynamic_cast<mitk::MultiLabelSegmentation*>(testImages[0].GetPointer());

    CPPUNIT_ASSERT_MESSAGE("Number of layers is not correct", lsimage1->GetNumberOfLayers() == 1);
    CPPUNIT_ASSERT_MESSAGE("Number of labels is not correct", lsimage1->GetTotalNumberOfLabels() == 0);
  }

  void TestReadEmptyMultiLabelSegmentation_withNoMetaInformation()
  {
    auto testImages = mitk::IOUtil::Load(GetTestDataFilePath("Multilabel/EmptyMultiLabelSegmentation_no_labels_meta.nrrd"));

    CPPUNIT_ASSERT_MESSAGE("Error reading empty MultiLabel segmentation", testImages.size() == 1);

    auto lsimage1 = dynamic_cast<mitk::MultiLabelSegmentation*>(testImages[0].GetPointer());

    CPPUNIT_ASSERT_MESSAGE("Number of layers is not correct", lsimage1->GetNumberOfLayers() == 1);
    CPPUNIT_ASSERT_MESSAGE("Number of labels is not correct", lsimage1->GetTotalNumberOfLabels() == 0);


    testImages = mitk::IOUtil::Load(GetTestDataFilePath("Multilabel/EmptyMultiLabelSegmentation_no_labels_meta_but_layer_dim.nrrd"));

    CPPUNIT_ASSERT_MESSAGE("Error reading empty MultiLabel segmentation", testImages.size() == 1);

    lsimage1 = dynamic_cast<mitk::MultiLabelSegmentation*>(testImages[0].GetPointer());

    CPPUNIT_ASSERT_MESSAGE("Number of layers is not correct", lsimage1->GetNumberOfLayers() == 1);
    CPPUNIT_ASSERT_MESSAGE("Number of labels is not correct", lsimage1->GetTotalNumberOfLabels() == 0);
  }

  void TestReadEmptyMultiLabelSegmentation_withNoMetaInformation_butContent()
  {
    CPPUNIT_ASSERT_THROW_MESSAGE("Loading empty segmentation with invalid pixel content did not fail.",
      mitk::IOUtil::Load(GetTestDataFilePath("Multilabel/EmptyMultiLabelSegmentation_no_labels_meta_but_pixel_content.nrrd")),
      mitk::Exception);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkMultiLabelSegmentationIO)
