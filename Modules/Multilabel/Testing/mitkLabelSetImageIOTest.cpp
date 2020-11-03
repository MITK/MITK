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

class mitkLabelSetImageIOTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkLabelSetImageIOTestSuite);
  MITK_TEST(TestReadWrite3DLabelSetImage);
  MITK_TEST(TestReadWrite3DplusTLabelSetImage);
  MITK_TEST(TestReadWrite3DplusTLabelSetImageWithArbitraryGeometry);
  MITK_TEST(TestReadWriteProperties);
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
    regularImage = nullptr;
    multilabelImage = nullptr;
  }
  
  void TestReadWrite3DLabelSetImage()
  {
    unsigned int dimensions[3] = {30, 20, 10};
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

    auto loadedImage =
      mitk::IOUtil::Load<mitk::LabelSetImage >(pathToImage);

    // This information is currently not serialized but also checked within the Equals function
    loadedImage->SetActiveLayer(multilabelImage->GetActiveLayer());

    CPPUNIT_ASSERT_MESSAGE("Error reading label set image", loadedImage.IsNotNull());
    CPPUNIT_ASSERT_MESSAGE("Error reading label set image", mitk::Equal(*multilabelImage, *loadedImage, 0.0001, true));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Error, read image has different UID", multilabelImage->GetUID(), loadedImage->GetUID());

    itksys::SystemTools::RemoveFile(pathToImage);
  }

  void TestReadWrite3DplusTLabelSetImage()
  {
    unsigned int dimensions[4] = {30, 20, 15, 10};
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

    auto loadedImage =
      mitk::IOUtil::Load<mitk::LabelSetImage >(pathToImage);

    // This information is currently not serialized but also checked within the Equals function
    loadedImage->SetActiveLayer(multilabelImage->GetActiveLayer());

    CPPUNIT_ASSERT_MESSAGE("Error reading label set image", loadedImage.IsNotNull());
    CPPUNIT_ASSERT_MESSAGE("Error reading label set image", mitk::Equal(*multilabelImage, *loadedImage, 0.0001, true));
    CPPUNIT_ASSERT_MESSAGE("Error reading time geometry of label set image", mitk::Equal(*(multilabelImage->GetTimeGeometry()), *(loadedImage->GetTimeGeometry()), 0.000000001, true));

    itksys::SystemTools::RemoveFile(pathToImage);
  }

  void TestReadWrite3DplusTLabelSetImageWithArbitraryGeometry()
  {
    unsigned int dimensions[4] = { 30, 20, 10, 4 };
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

    auto geometry = multilabelImage->GetGeometry()->Clone();

    auto refTimeGeometry = mitk::ArbitraryTimeGeometry::New();
    refTimeGeometry->AppendNewTimeStep(geometry, 0., 0.5);
    refTimeGeometry->AppendNewTimeStep(geometry, 0.5, 1.);
    refTimeGeometry->AppendNewTimeStep(geometry, 1., 2.);
    refTimeGeometry->AppendNewTimeStep(geometry, 2., 5.5);
    multilabelImage->SetTimeGeometry(refTimeGeometry);

    pathToImage = mitk::IOUtil::CreateTemporaryDirectory();
    pathToImage.append("/LabelSetTestImage3DplusTWithArbitraryTimeGeometry.nrrd");

    mitk::IOUtil::Save(multilabelImage, pathToImage);

    auto loadedImage =
      mitk::IOUtil::Load<mitk::LabelSetImage >(pathToImage);

    // This information is currently not serialized but also checked within the Equals function
    loadedImage->SetActiveLayer(multilabelImage->GetActiveLayer());

    CPPUNIT_ASSERT_MESSAGE("Error reading label set image", loadedImage.IsNotNull());
    CPPUNIT_ASSERT_MESSAGE("Error reading label set image", mitk::Equal(*multilabelImage, *loadedImage, 0.0001, true));
    CPPUNIT_ASSERT_MESSAGE("Error reading time geometry of label set image", mitk::Equal(*refTimeGeometry, *(loadedImage->GetTimeGeometry()), 0.000000001, true));
    itksys::SystemTools::RemoveFile(pathToImage);
  }

  void TestReadWriteProperties()
  {
    unsigned int dimensions[3] = { 30, 20, 10 };
    regularImage->Initialize(mitk::MakeScalarPixelType<int>(), 3, dimensions);

    multilabelImage = mitk::LabelSetImage::New();
    multilabelImage->Initialize(regularImage);
    mitk::LabelSet::Pointer newlayer = mitk::LabelSet::New();
    newlayer->SetLayer(1);
    mitk::Label::Pointer label0 = mitk::Label::New();
    label0->SetName("Background");
    label0->SetValue(0);
    newlayer->AddLabel(label0);
    multilabelImage->AddLayer(newlayer);

    auto propPersistenceInfo = mitk::PropertyPersistenceInfo::New();
    propPersistenceInfo->SetNameAndKey("my.cool.test.property", "my_cool_test_property");
    mitk::CoreServicePointer<mitk::IPropertyPersistence> propPersService(mitk::CoreServices::GetPropertyPersistence());
    propPersService->AddInfo(propPersistenceInfo);

    multilabelImage->SetProperty("my.cool.test.property", mitk::StringProperty::New("test_content"));

    pathToImage = mitk::IOUtil::CreateTemporaryDirectory();
    pathToImage.append("/LabelSetPropertiesTestImage.nrrd");

    mitk::IOUtil::Save(multilabelImage, pathToImage);

    auto loadedImage =
      mitk::IOUtil::Load<mitk::LabelSetImage >(pathToImage);

    auto loadedProp = loadedImage->GetProperty("my.cool.test.property");
    CPPUNIT_ASSERT_MESSAGE("Error reading properties of label set image", loadedProp.IsNotNull());
    CPPUNIT_ASSERT_MESSAGE("Error reading properties of label set image", loadedProp->GetValueAsString() == "test_content");
    itksys::SystemTools::RemoveFile(pathToImage);
  }


};

MITK_TEST_SUITE_REGISTRATION(mitkLabelSetImageIO)
