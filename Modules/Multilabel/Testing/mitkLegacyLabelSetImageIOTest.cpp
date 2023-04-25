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

std::string pathToImage;

class mitkLegacyLabelSetImageIOTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkLegacyLabelSetImageIOTestSuite);
  MITK_TEST(TestRead3DLabelSetImage_Default);
  MITK_TEST(TestRead3DLabelSetImage_Adapt);
  MITK_TEST(TestRead3DLabelSetImage_Split);
  MITK_TEST(TestRead3DplusTLabelSetImage);
  CPPUNIT_TEST_SUITE_END();

private:
  mitk::LabelSet::Pointer m_labelSet1;
  mitk::LabelSet::Pointer m_labelSet2;
  mitk::LabelSet::Pointer m_labelSet2_adapted;

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
    m_labelSet1 = mitk::LabelSet::New();
    auto label = GenerateLabel(1, "Label 1", 0.745098054f, 0.f, 0.196078435f);
    m_labelSet1->AddLabel(label,false);
    label = GenerateLabel(2, "Label 2", 0.952941179, 0.764705896, 0);
    m_labelSet1->AddLabel(label, false);

    m_labelSet2 = mitk::LabelSet::New();
    label = GenerateLabel(1, "Label 3", 0.552941203, 0.713725507, 0);
    m_labelSet2->AddLabel(label, false);
    label = GenerateLabel(2, "Label 4", 0.631372571, 0.792156875, 0.945098042);
    m_labelSet2->AddLabel(label, false);
    label = GenerateLabel(3, "Label 5", 0.639215708, 0.250980407, 0.725490212);
    m_labelSet2->AddLabel(label, false);

    m_labelSet2_adapted = mitk::LabelSet::New();
    label = GenerateLabel(3, "Label 3", 0.552941203, 0.713725507, 0);
    m_labelSet2_adapted->AddLabel(label, false);
    label = GenerateLabel(4, "Label 4", 0.631372571, 0.792156875, 0.945098042);
    m_labelSet2_adapted->AddLabel(label, false);
    label = GenerateLabel(5, "Label 5", 0.639215708, 0.250980407, 0.725490212);
    m_labelSet2_adapted->AddLabel(label, false);
    m_labelSet2_adapted->SetLayer(1);
  }

  void tearDown() override
  {
    m_labelSet1 = nullptr;
    m_labelSet2 = nullptr;
    m_labelSet2_adapted = nullptr;
  }
  
  void TestRead3DLabelSetImage_Default()
  {
    auto testImages = mitk::IOUtil::Load(GetTestDataFilePath("Multilabel/LegacyLabelSetTestImage3D.nrrd"));

    CPPUNIT_ASSERT_MESSAGE("Error reading label set image", testImages.size()==1);

    auto lsimage1 = dynamic_cast<mitk::LabelSetImage*>(testImages[0].GetPointer());

    CPPUNIT_ASSERT_MESSAGE("Number of layers is not correct", lsimage1->GetNumberOfLayers() == 2);
    CPPUNIT_ASSERT_MESSAGE("Error layer 0 is not equal", mitk::Equal(*m_labelSet1, *(lsimage1->GetLabelSet(0)), mitk::eps, true));
    CPPUNIT_ASSERT_MESSAGE("Error layer 1 is not equal", mitk::Equal(*m_labelSet2_adapted, *(lsimage1->GetLabelSet(1)), mitk::eps, true));

    CPPUNIT_ASSERT_MESSAGE("Error, read image has different UID", "c236532b-f95a-4f22-a4c6-7abe4e41ad10"== lsimage1->GetUID());
  }

  void TestRead3DLabelSetImage_Adapt()
  {
    mitk::IFileReader::Options options = { {"Multi layer handling", us::Any(std::string("Adapt label values"))} };
    auto testImages = mitk::IOUtil::Load(GetTestDataFilePath("Multilabel/LegacyLabelSetTestImage3D.nrrd"), options);

    CPPUNIT_ASSERT_MESSAGE("Error reading label set image", testImages.size() == 1);

    auto lsimage1 = dynamic_cast<mitk::LabelSetImage*>(testImages[0].GetPointer());

    CPPUNIT_ASSERT_MESSAGE("Number of layers is not correct", lsimage1->GetNumberOfLayers() == 2);
    CPPUNIT_ASSERT_MESSAGE("Error layer 0 is not equal", mitk::Equal(*m_labelSet1, *(lsimage1->GetLabelSet(0)), mitk::eps, true));
    CPPUNIT_ASSERT_MESSAGE("Error layer 1 is not equal", mitk::Equal(*m_labelSet2_adapted, *(lsimage1->GetLabelSet(1)), mitk::eps, true));

    CPPUNIT_ASSERT_MESSAGE("Error, read image has different UID", "c236532b-f95a-4f22-a4c6-7abe4e41ad10" == lsimage1->GetUID());
  }

  void TestRead3DLabelSetImage_Split()
  {
    mitk::IFileReader::Options options = { {"Multi layer handling", us::Any(std::string("Split layers"))} };
    auto testImages = mitk::IOUtil::Load(GetTestDataFilePath("Multilabel/LegacyLabelSetTestImage3D.nrrd"), options);

    CPPUNIT_ASSERT_MESSAGE("Error reading label set image", testImages.size() == 2);

    auto lsimage1 = dynamic_cast<mitk::LabelSetImage*>(testImages[0].GetPointer());
    auto lsimage2 = dynamic_cast<mitk::LabelSetImage*>(testImages[1].GetPointer());

    CPPUNIT_ASSERT_MESSAGE("Number of layers in image 1 isnot correct", lsimage1->GetNumberOfLayers() == 1);
    CPPUNIT_ASSERT_MESSAGE("Number of layers in image 2 is not correct", lsimage2->GetNumberOfLayers() == 1);
    CPPUNIT_ASSERT_MESSAGE("Error layer 0 is not equal", mitk::Equal(*m_labelSet1, *(lsimage1->GetLabelSet(0)), mitk::eps, true));
    CPPUNIT_ASSERT_MESSAGE("Error layer 1 is not equal", mitk::Equal(*m_labelSet2, *(lsimage2->GetLabelSet(0)), mitk::eps, true));

    CPPUNIT_ASSERT_MESSAGE("Error, read image has same UID", "c236532b-f95a-4f22-a4c6-7abe4e41ad10" != lsimage1->GetUID());

  }

  void TestRead3DplusTLabelSetImage()
  {

  }

};

MITK_TEST_SUITE_REGISTRATION(mitkLegacyLabelSetImageIO)
