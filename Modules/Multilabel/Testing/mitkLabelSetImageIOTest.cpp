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
#include <mitkLabelSetImageIO.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

std::string pathToImage;

class mitkLabelSetImageIOTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkLabelSetImageIOTestSuite);
  MITK_TEST(TestWriteLabelSetImage);
  MITK_TEST(TestReadLabelSetImage);
  CPPUNIT_TEST_SUITE_END();

private:
  mitk::Image::Pointer regularImage;
  mitk::LabelSetImage::Pointer multilabelImage;

public:

  void setUp() override
  {
    regularImage = mitk::Image::New();
    unsigned int dimensions[3] = {256,256,312};
    regularImage->Initialize(mitk::MakeScalarPixelType<int>(), 3, dimensions);

    multilabelImage = mitk::LabelSetImage::New();
    multilabelImage->Initialize(regularImage);
    mitk::LabelSet::Pointer newlayer = mitk::LabelSet::New();
    newlayer->SetLayer(1);
    mitk::Label::Pointer label1 = mitk::Label::New();
    label1->SetName("Label1");
    label1->SetValue(1);

    mitk::Label::Pointer label2 = mitk::Label::New();
    label2->SetName("Label2");
    label2->SetValue(200);

    newlayer->AddLabel(label1);
    newlayer->AddLabel(label2);
    newlayer->SetActiveLabel(200);
    // TODO assert that the layer od labelset and labels is correct - TEST

    multilabelImage->AddLayer(newlayer);
  }

  void tearDown() override
  {
    regularImage = 0;
    multilabelImage = 0;
  }

  void TestWriteLabelSetImage()
  {
    pathToImage = mitk::IOUtil::CreateTemporaryDirectory();
    pathToImage.append("/LabelSetTestImage.nrrd");

    mitk::IOUtil::Save(multilabelImage, pathToImage);

    //dynamic_cast<mitk::IFileWriter*>(lsetIO)->SetInput(multilabelImage);
    //dynamic_cast<mitk::IFileWriter*>(lsetIO)->SetOutputLocation(pathToImage);
    //dynamic_cast<mitk::IFileWriter*>(lsetIO)->Write();
  }

  void TestReadLabelSetImage()
  {
    dynamic_cast<mitk::IFileReader*>(lsetIO)->SetInput(pathToImage);
    std::vector<mitk::BaseData::Pointer> data = dynamic_cast<mitk::IFileReader*>(lsetIO)->Read();
    CPPUNIT_ASSERT_MESSAGE("Too many images have been read", data.size() == 1);
    mitk::LabelSetImage::Pointer loadedImage = dynamic_cast<mitk::LabelSetImage*>(data.at(0).GetPointer());

    // This information is currently not serialized but also checked within the Equals function
    loadedImage->SetActiveLayer(multilabelImage->GetActiveLayer());

    CPPUNIT_ASSERT_MESSAGE("Error reading label set image", loadedImage.IsNotNull());
    CPPUNIT_ASSERT_MESSAGE("Error reading label set image", mitk::Equal(*multilabelImage, *loadedImage, 0.0001, true));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkLabelSetImageIO)
