/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkIOUtil.h>
#include <mitkImageStatisticsHolder.h>
#include <mitkLabelSetImage.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

class mitkLabelSetImageTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkLabelSetImageTestSuite);
  MITK_TEST(TestInitialize);
  MITK_TEST(TestAddLayer);
  MITK_TEST(TestGetActiveLabelSet);
  MITK_TEST(TestGetActiveLabel);
  MITK_TEST(TestInitializeByLabeledImage);
  MITK_TEST(TestGetLabelSet);
  MITK_TEST(TestGetLabel);
  MITK_TEST(TestSetExteriorLabel);
  MITK_TEST(TestGetTotalNumberOfLabels);
  MITK_TEST(TestExistsLabel);
  MITK_TEST(TestExistsLabelSet);
  MITK_TEST(TestSetActiveLayer);
  MITK_TEST(TestRemoveLayer);
  MITK_TEST(TestRemoveLabels);
  MITK_TEST(TestMergeLabel);
  CPPUNIT_TEST_SUITE_END();

private:
  mitk::LabelSetImage::Pointer m_LabelSetImage;

public:
  void setUp() override
  {
    // Create a new labelset image
    m_LabelSetImage = mitk::LabelSetImage::New();
    mitk::Image::Pointer regularImage = mitk::Image::New();
    unsigned int dimensions[3] = { 96, 128, 52 };
    regularImage->Initialize(mitk::MakeScalarPixelType<char>(), 3, dimensions);
    m_LabelSetImage->Initialize(regularImage);
  }

  void tearDown() override
  {
    // Delete LabelSetImage
    m_LabelSetImage = nullptr;
  }

  void TestInitialize()
  {
    // LabelSet image should always has the pixel type mitk::Label::PixelType
    CPPUNIT_ASSERT_MESSAGE("LabelSetImage has wrong pixel type",
                           m_LabelSetImage->GetPixelType() == mitk::MakeScalarPixelType<mitk::Label::PixelType>());

    mitk::Image::Pointer regularImage = mitk::Image::New();
    unsigned int dimensions[3] = { 96, 128, 52 };
    regularImage->Initialize(mitk::MakeScalarPixelType<char>(), 3, dimensions);

    mitk::BaseGeometry::Pointer regularImageGeo = regularImage->GetGeometry();
    mitk::BaseGeometry::Pointer labelImageGeo = m_LabelSetImage->GetGeometry();
    MITK_ASSERT_EQUAL(labelImageGeo, regularImageGeo, "LabelSetImage has wrong geometry");

    // By default one layer containing the exterior label should be added
    CPPUNIT_ASSERT_MESSAGE("Image was not correctly initialized - number of layers is not one",
                           m_LabelSetImage->GetNumberOfLayers() == 1);
    CPPUNIT_ASSERT_MESSAGE("Image was not correctly initialized - active layer has wrong ID",
                           m_LabelSetImage->GetActiveLayer() == 0);

    CPPUNIT_ASSERT_MESSAGE("Image was not correctly initialized - active label is not the exterior label",
                           m_LabelSetImage->GetActiveLabel()->GetValue() == 0);
  }

  void TestAddLayer()
  {
    CPPUNIT_ASSERT_MESSAGE("Number of layers is not zero", m_LabelSetImage->GetNumberOfLayers() == 1);

    m_LabelSetImage->AddLayer();
    CPPUNIT_ASSERT_MESSAGE("Layer was not added correctly to image - number of layers is not one",
                           m_LabelSetImage->GetNumberOfLayers() == 2);
    CPPUNIT_ASSERT_MESSAGE("Layer was not added correctly to image - active layer has wrong ID",
                           m_LabelSetImage->GetActiveLayer() == 1);

    CPPUNIT_ASSERT_MESSAGE("Layer was not added correctly to image - active label is not the exterior label",
                           m_LabelSetImage->GetActiveLabel()->GetValue() == 0);

    mitk::LabelSet::Pointer newlayer = mitk::LabelSet::New();
    mitk::Label::Pointer label1 = mitk::Label::New();
    label1->SetName("Label1");
    label1->SetValue(1);

    mitk::Label::Pointer label2 = mitk::Label::New();
    label2->SetName("Label2");
    label2->SetValue(200);

    newlayer->AddLabel(label1);
    newlayer->AddLabel(label2);
    newlayer->SetActiveLabel(200);

    unsigned int layerID = m_LabelSetImage->AddLayer(newlayer);
    CPPUNIT_ASSERT_MESSAGE("Layer was not added correctly to image - number of layers is not two",
                           m_LabelSetImage->GetNumberOfLayers() == 3);
    CPPUNIT_ASSERT_MESSAGE("Layer was not added correctly to image - active layer has wrong ID",
                           m_LabelSetImage->GetActiveLayer() == layerID);
    CPPUNIT_ASSERT_MESSAGE("Layer was not added correctly to image - active label is wrong",
                           m_LabelSetImage->GetActiveLabel(layerID)->GetValue() == 200);
  }

  void TestGetActiveLabelSet()
  {
    mitk::LabelSet::Pointer newlayer = mitk::LabelSet::New();
    mitk::Label::Pointer label1 = mitk::Label::New();
    label1->SetName("Label1");
    label1->SetValue(1);

    mitk::Label::Pointer label2 = mitk::Label::New();
    label2->SetName("Label2");
    label2->SetValue(200);

    newlayer->AddLabel(label1);
    newlayer->AddLabel(label2);
    newlayer->SetActiveLabel(200);

    unsigned int layerID = m_LabelSetImage->AddLayer(newlayer);

    mitk::LabelSet::Pointer activeLayer = m_LabelSetImage->GetActiveLabelSet();

    CPPUNIT_ASSERT_MESSAGE("Wrong layer ID was returned", layerID == 1);
    CPPUNIT_ASSERT_MESSAGE("Wrong active labelset returned", mitk::Equal(*newlayer, *activeLayer, 0.00001, true));

    mitk::LabelSet::ConstPointer constActiveLayer = const_cast<const mitk::LabelSetImage*>(m_LabelSetImage.GetPointer())->GetActiveLabelSet();
    CPPUNIT_ASSERT_MESSAGE("Wrong active labelset returned", mitk::Equal(*newlayer, *constActiveLayer, 0.00001, true));
  }

  void TestGetActiveLabel()
  {
    mitk::Label::Pointer label1 = mitk::Label::New();
    label1->SetName("Label1");
    mitk::Label::PixelType value1 = 1;
    label1->SetValue(value1);

    mitk::Label::Pointer label2 = mitk::Label::New();
    label2->SetName("Label2");
    mitk::Label::PixelType value2 = 200;
    label2->SetValue(value2);

    m_LabelSetImage->GetActiveLabelSet()->AddLabel(label1);
    m_LabelSetImage->GetActiveLabelSet()->AddLabel(label2);
    m_LabelSetImage->GetActiveLabelSet()->SetActiveLabel(1);
    CPPUNIT_ASSERT_MESSAGE("Layer was not added correctly to image - active label is wrong",
                           m_LabelSetImage->GetActiveLabel()->GetValue() == value1);
    m_LabelSetImage->GetActiveLabelSet()->SetActiveLabel(value2);
    CPPUNIT_ASSERT_MESSAGE("Layer was not added correctly to image - active label is wrong",
                           m_LabelSetImage->GetActiveLabel()->GetValue() == value2);

    CPPUNIT_ASSERT_MESSAGE("Active Label was not correctly retreived with const getter",
      const_cast<const mitk::LabelSetImage*>(m_LabelSetImage.GetPointer())->GetActiveLabel()->GetValue() == value2);

  }

  void TestInitializeByLabeledImage()
  {
    mitk::Image::Pointer image =
      mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("Multilabel/LabelSetTestInitializeImage.nrrd"));
    m_LabelSetImage->InitializeByLabeledImage(image);
    CPPUNIT_ASSERT_MESSAGE("Image - number of labels is not 6", m_LabelSetImage->GetNumberOfLabels() == 6);
  }

  void TestGetLabelSet()
  {
    // Test get non existing lset
    mitk::LabelSet::ConstPointer lset = m_LabelSetImage->GetLabelSet(10000);
    CPPUNIT_ASSERT_MESSAGE("Non existing labelset is not nullptr", lset.IsNull());

    lset = m_LabelSetImage->GetLabelSet(0);
    CPPUNIT_ASSERT_MESSAGE("Existing labelset is nullptr", lset.IsNotNull());
  }

  void TestGetLabel()
  {
    mitk::Label::Pointer label1 = mitk::Label::New();
    label1->SetName("Label1");
    mitk::Label::PixelType value1 = 1;
    label1->SetValue(value1);

    mitk::Label::Pointer label2 = mitk::Label::New();
    label2->SetName("Label2");
    mitk::Label::PixelType value2 = 200;
    label2->SetValue(value2);

    m_LabelSetImage->GetActiveLabelSet()->AddLabel(label1);
    m_LabelSetImage->AddLayer();
    m_LabelSetImage->GetLabelSet(1)->AddLabel(label2);

    CPPUNIT_ASSERT_MESSAGE("Wrong label retrieved for active layer",
                           mitk::Equal(*m_LabelSetImage->GetLabel(1), *label1, 0.0001, true));
    CPPUNIT_ASSERT_MESSAGE("Wrong label retrieved for layer 1",
                           mitk::Equal(*m_LabelSetImage->GetLabel(200, 1), *label2, 0.0001, true));

    // Try to get a non existing label
    mitk::Label *label3 = m_LabelSetImage->GetLabel(1000);
    CPPUNIT_ASSERT_MESSAGE("Non existing label should be nullptr", label3 == nullptr);

    // Try to get a label from a non existing layer
    label3 = m_LabelSetImage->GetLabel(200, 1000);
    CPPUNIT_ASSERT_MESSAGE("Label from non existing layer should be nullptr", label3 == nullptr);
  }

  void TestSetExteriorLabel()
  {
    mitk::Label::Pointer exteriorLabel = mitk::Label::New();
    exteriorLabel->SetName("MyExteriorSpecialLabel");
    mitk::Label::PixelType value1 = 10000;
    exteriorLabel->SetValue(value1);

    m_LabelSetImage->SetExteriorLabel(exteriorLabel);
    CPPUNIT_ASSERT_MESSAGE("Wrong label retrieved for layer 1",
                           mitk::Equal(*m_LabelSetImage->GetExteriorLabel(), *exteriorLabel, 0.0001, true));

    // Exterior label should be set automatically for each new layer
    m_LabelSetImage->AddLayer();
    CPPUNIT_ASSERT_MESSAGE("Wrong label retrieved for layer 1",
                           mitk::Equal(*m_LabelSetImage->GetLabel(10000, 1), *exteriorLabel, 0.0001, true));
  }

  void TestGetTotalNumberOfLabels()
  {
    mitk::Label::Pointer label1 = mitk::Label::New();
    label1->SetName("Label1");
    mitk::Label::PixelType value1 = 1;
    label1->SetValue(value1);

    mitk::Label::Pointer label2 = mitk::Label::New();
    label2->SetName("Label2");
    mitk::Label::PixelType value2 = 200;
    label2->SetValue(value2);

    m_LabelSetImage->GetActiveLabelSet()->AddLabel(label1);
    m_LabelSetImage->AddLayer();
    m_LabelSetImage->GetLabelSet(1)->AddLabel(label2);
    CPPUNIT_ASSERT_MESSAGE(
      "Wrong total number of labels",
      m_LabelSetImage->GetTotalNumberOfLabels() == 4); // added 2 labels + 2 exterior default labels
  }

  void TestExistsLabel()
  {
    mitk::Label::Pointer label = mitk::Label::New();
    label->SetName("Label2");
    mitk::Label::PixelType value = 200;
    label->SetValue(value);

    m_LabelSetImage->AddLayer();
    m_LabelSetImage->GetLabelSet(1)->AddLabel(label);
    m_LabelSetImage->SetActiveLayer(0);
    CPPUNIT_ASSERT_MESSAGE("Existing label was not found", m_LabelSetImage->ExistLabel(value) == true);

    CPPUNIT_ASSERT_MESSAGE("Non existing label was found", m_LabelSetImage->ExistLabel(10000) == false);
  }

  void TestExistsLabelSet()
  {
    // Cache active layer
    mitk::LabelSet::ConstPointer activeLayer = m_LabelSetImage->GetActiveLabelSet();

    // Add new layer
    mitk::LabelSet::Pointer newlayer = mitk::LabelSet::New();
    mitk::Label::Pointer label1 = mitk::Label::New();
    label1->SetName("Label1");
    label1->SetValue(1);

    mitk::Label::Pointer label2 = mitk::Label::New();
    label2->SetName("Label2");
    label2->SetValue(200);

    newlayer->AddLabel(label1);
    newlayer->AddLabel(label2);
    newlayer->SetActiveLabel(200);

    m_LabelSetImage->AddLayer(newlayer);

    CPPUNIT_ASSERT_MESSAGE("Check for existing layer failed", m_LabelSetImage->ExistLabelSet(0) == true);
    CPPUNIT_ASSERT_MESSAGE("Check for existing layer failed", m_LabelSetImage->ExistLabelSet(1) == true);
    CPPUNIT_ASSERT_MESSAGE("Check for existing layer failed", m_LabelSetImage->ExistLabelSet(20) == false);
  }

  void TestSetActiveLayer()
  {
    // Cache active layer
    mitk::LabelSet::ConstPointer activeLayer = m_LabelSetImage->GetActiveLabelSet();

    // Add new layer
    mitk::LabelSet::Pointer newlayer = mitk::LabelSet::New();
    mitk::Label::Pointer label1 = mitk::Label::New();
    label1->SetName("Label1");
    label1->SetValue(1);

    mitk::Label::Pointer label2 = mitk::Label::New();
    label2->SetName("Label2");
    label2->SetValue(200);

    newlayer->AddLabel(label1);
    newlayer->AddLabel(label2);
    newlayer->SetActiveLabel(200);

    unsigned int layerID = m_LabelSetImage->AddLayer(newlayer);

    // Set initial layer as active layer
    m_LabelSetImage->SetActiveLayer(0);
    CPPUNIT_ASSERT_MESSAGE("Wrong active labelset returned",
                           mitk::Equal(*activeLayer, *m_LabelSetImage->GetActiveLabelSet(), 0.00001, true));

    // Set previously added layer as active layer
    m_LabelSetImage->SetActiveLayer(layerID);
    CPPUNIT_ASSERT_MESSAGE("Wrong active labelset returned",
                           mitk::Equal(*newlayer, *m_LabelSetImage->GetActiveLabelSet(), 0.00001, true));

    // Set a non existing layer as active layer - nothing should change
    m_LabelSetImage->SetActiveLayer(10000);
    CPPUNIT_ASSERT_MESSAGE("Wrong active labelset returned",
                           mitk::Equal(*newlayer, *m_LabelSetImage->GetActiveLabelSet(), 0.00001, true));
  }

  void TestRemoveLayer()
  {
    // Cache active layer
    mitk::LabelSet::ConstPointer activeLayer = m_LabelSetImage->GetActiveLabelSet();

    // Add new layers
    m_LabelSetImage->AddLayer();

    mitk::LabelSet::Pointer newlayer = mitk::LabelSet::New();
    mitk::Label::Pointer label1 = mitk::Label::New();
    label1->SetName("Label1");
    label1->SetValue(1);

    mitk::Label::Pointer label2 = mitk::Label::New();
    label2->SetName("Label2");
    label2->SetValue(200);

    newlayer->AddLabel(label1);
    newlayer->AddLabel(label2);
    newlayer->SetActiveLabel(200);

    m_LabelSetImage->AddLayer(newlayer);

    CPPUNIT_ASSERT_MESSAGE("Wrong active labelset returned",
                           mitk::Equal(*newlayer, *m_LabelSetImage->GetActiveLabelSet(), 0.00001, true));

    m_LabelSetImage->RemoveLayer();
    CPPUNIT_ASSERT_MESSAGE("Wrong number of layers, after a layer was removed",
                           m_LabelSetImage->GetNumberOfLayers() == 2);
    CPPUNIT_ASSERT_MESSAGE("Check for existing layer failed", m_LabelSetImage->ExistLabelSet(2) == false);
    CPPUNIT_ASSERT_MESSAGE("Check for existing layer failed", m_LabelSetImage->ExistLabelSet(1) == true);
    CPPUNIT_ASSERT_MESSAGE("Check for existing layer failed", m_LabelSetImage->ExistLabelSet(0) == true);

    m_LabelSetImage->RemoveLayer();
    CPPUNIT_ASSERT_MESSAGE("Wrong number of layers, after a layer was removed",
                           m_LabelSetImage->GetNumberOfLayers() == 1);
    CPPUNIT_ASSERT_MESSAGE("Check for existing layer failed", m_LabelSetImage->ExistLabelSet(1) == false);
    CPPUNIT_ASSERT_MESSAGE("Check for existing layer failed", m_LabelSetImage->ExistLabelSet(0) == true);
    CPPUNIT_ASSERT_MESSAGE("Wrong active layer",
                           mitk::Equal(*activeLayer, *m_LabelSetImage->GetActiveLabelSet(), 0.00001, true));

    m_LabelSetImage->RemoveLayer();
    CPPUNIT_ASSERT_MESSAGE("Wrong number of layers, after a layer was removed",
                           m_LabelSetImage->GetNumberOfLayers() == 0);
    CPPUNIT_ASSERT_MESSAGE("Check for existing layer failed", m_LabelSetImage->ExistLabelSet(0) == false);
    CPPUNIT_ASSERT_MESSAGE("Active layers is not nullptr although all layer have been removed",
                           m_LabelSetImage->GetActiveLabelSet() == nullptr);
  }

  void TestRemoveLabels()
  {
    mitk::Image::Pointer image =
      mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("Multilabel/LabelSetTestInitializeImage.nrrd"));
    m_LabelSetImage->InitializeByLabeledImage(image);

    CPPUNIT_ASSERT_MESSAGE("Image - number of labels is not 6", m_LabelSetImage->GetNumberOfLabels() == 6);
    // 2ndMin because of the exterior label = 0
    CPPUNIT_ASSERT_MESSAGE("Labels with value 1 and 3 was not remove from the image",
                           m_LabelSetImage->GetStatistics()->GetScalarValue2ndMin() == 1);
    CPPUNIT_ASSERT_MESSAGE("Label with value 7 was not remove from the image",
                           m_LabelSetImage->GetStatistics()->GetScalarValueMax() == 7);

    CPPUNIT_ASSERT_MESSAGE("Label with ID 3 does not exists after initialization",
                           m_LabelSetImage->ExistLabel(3) == true);
    CPPUNIT_ASSERT_MESSAGE("Label with ID 7 does not exists after initialization",
                           m_LabelSetImage->ExistLabel(7) == true);

    std::vector<mitk::Label::PixelType> labelsToBeRemoved;
    labelsToBeRemoved.push_back(1);
    labelsToBeRemoved.push_back(3);
    labelsToBeRemoved.push_back(7);
    m_LabelSetImage->RemoveLabels(labelsToBeRemoved);

    CPPUNIT_ASSERT_MESSAGE("Wrong number of labels after some have been removed",
                           m_LabelSetImage->GetNumberOfLabels() == 3);
    // Values within the image are 0, 1, 3, 5, 6, 7 - New Min/Max value should be 5 / 6
    // 2ndMin because of the exterior label = 0
    CPPUNIT_ASSERT_MESSAGE("Labels with value 1 and 3 was not remove from the image",
                           m_LabelSetImage->GetStatistics()->GetScalarValue2ndMin() == 5);
    CPPUNIT_ASSERT_MESSAGE("Label with value 7 was not remove from the image",
                           m_LabelSetImage->GetStatistics()->GetScalarValueMax() == 6);
  }

  void TestMergeLabel()
  {
    mitk::Image::Pointer image = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("Multilabel/LabelSetTestInitializeImage.nrrd"));
    m_LabelSetImage = nullptr;
    m_LabelSetImage = mitk::LabelSetImage::New();
    m_LabelSetImage->InitializeByLabeledImage(image);

    CPPUNIT_ASSERT_MESSAGE("Image - number of labels is not 6", m_LabelSetImage->GetNumberOfLabels() == 6);

    // 2ndMin because of the exterior label = 0
    CPPUNIT_ASSERT_MESSAGE("Wrong MIN value", m_LabelSetImage->GetStatistics()->GetScalarValueMin() == 0);
    CPPUNIT_ASSERT_MESSAGE("Wrong MAX value", m_LabelSetImage->GetStatistics()->GetScalarValueMax() == 7);

    m_LabelSetImage->GetActiveLabelSet()->SetActiveLabel(6);
    // Merge label 7 with label 0. Result should be that label 7 is not present any more
    m_LabelSetImage->MergeLabel(6, 7);
    CPPUNIT_ASSERT_MESSAGE("Label with value 7 was not remove from the image", m_LabelSetImage->GetStatistics()->GetScalarValueMax() == 6);
    m_LabelSetImage->GetStatistics()->GetScalarValue2ndMax();

    // Count all pixels with value 7 = 823
    // Count all pixels with value 6 = 507
    // Check if merge label has 507 + 823 = 1330 pixels
    CPPUNIT_ASSERT_MESSAGE("Label with value 7 was not remove from the image", m_LabelSetImage->GetStatistics()->GetCountOfMaxValuedVoxels() == 1330);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkLabelSetImage)
