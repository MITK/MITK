/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include <mitkIOUtil.h>
#include <mitkLabelSetImage.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

std::string pathToImage;

class mitkLabelSetImageIOTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkLabelSetImageIOTestSuite);
  MITK_TEST(TestReadWrite3DLabelSetImage);
  MITK_TEST(TestReadWrite3DplusTLabelSetImage);
  CPPUNIT_TEST_SUITE_END();

private:
  mitk::Image::Pointer regularImage;
  mitk::LabelSetImage::Pointer multilabelImage;

public:
  void setUp() override
  {
    regularImage = mitk::Image::New();
  }

  void tearDown() override
  {
    regularImage = 0;
    multilabelImage = 0;
  }

  void TestReadWrite3DLabelSetImage()
  {
    unsigned int dimensions[3] = {256, 256, 312};
    regularImage->Initialize(mitk::MakeScalarPixelType<int>(), 3, dimensions);

    multilabelImage = mitk::LabelSetImage::New();
    multilabelImage->Initialize(regularImage);
    mitk::LabelSet::Pointer newlayer = mitk::LabelSet::New();
    newlayer->SetLayer(1);
    mitk::Label::Pointer label0 = mitk::Label::New();
    label0->SetName("Background");
    label0->SetValue(0);

    mitk::Label::Pointer label1 = mitk::Label::New();
    label1->SetName("Label1");
    label1->SetValue(1);

    mitk::Label::Pointer label2 = mitk::Label::New();
    label2->SetName("Label2");
    label2->SetValue(200);

    newlayer->AddLabel(label0);
    newlayer->AddLabel(label1);
    newlayer->AddLabel(label2);
    newlayer->SetActiveLabel(200);

    multilabelImage->AddLayer(newlayer);

    pathToImage = mitk::IOUtil::CreateTemporaryDirectory();
    pathToImage.append("/LabelSetTestImage3D.nrrd");

    mitk::IOUtil::Save(multilabelImage, pathToImage);

    mitk::LabelSetImage::Pointer loadedImage =
      dynamic_cast<mitk::LabelSetImage *>(mitk::IOUtil::LoadBaseData(pathToImage).GetPointer());

    // This information is currently not serialized but also checked within the Equals function
    loadedImage->SetActiveLayer(multilabelImage->GetActiveLayer());

    CPPUNIT_ASSERT_MESSAGE("Error reading label set image", loadedImage.IsNotNull());
    CPPUNIT_ASSERT_MESSAGE("Error reading label set image", mitk::Equal(*multilabelImage, *loadedImage, 0.0001, true));

    itksys::SystemTools::RemoveFile(pathToImage);
  }

  void TestReadWrite3DplusTLabelSetImage()
  {
    unsigned int dimensions[4] = {256, 256, 312, 10};
    regularImage->Initialize(mitk::MakeScalarPixelType<int>(), 4, dimensions);

    multilabelImage = mitk::LabelSetImage::New();
    multilabelImage->Initialize(regularImage);
    mitk::LabelSet::Pointer newlayer = mitk::LabelSet::New();
    newlayer->SetLayer(1);
    mitk::Label::Pointer label0 = mitk::Label::New();
    label0->SetName("Background");
    label0->SetValue(0);

    mitk::Label::Pointer label1 = mitk::Label::New();
    label1->SetName("Label1");
    label1->SetValue(1);

    mitk::Label::Pointer label2 = mitk::Label::New();
    label2->SetName("Label2");
    label2->SetValue(200);

    newlayer->AddLabel(label0);
    newlayer->AddLabel(label1);
    newlayer->AddLabel(label2);
    newlayer->SetActiveLabel(200);

    multilabelImage->AddLayer(newlayer);

    pathToImage = mitk::IOUtil::CreateTemporaryDirectory();
    pathToImage.append("/LabelSetTestImage3DplusT.nrrd");

    mitk::IOUtil::Save(multilabelImage, pathToImage);

    mitk::LabelSetImage::Pointer loadedImage =
      dynamic_cast<mitk::LabelSetImage *>(mitk::IOUtil::LoadBaseData(pathToImage).GetPointer());

    // This information is currently not serialized but also checked within the Equals function
    loadedImage->SetActiveLayer(multilabelImage->GetActiveLayer());

    CPPUNIT_ASSERT_MESSAGE("Error reading label set image", loadedImage.IsNotNull());
    CPPUNIT_ASSERT_MESSAGE("Error reading label set image", mitk::Equal(*multilabelImage, *loadedImage, 0.0001, true));

    itksys::SystemTools::RemoveFile(pathToImage);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkLabelSetImageIO)
