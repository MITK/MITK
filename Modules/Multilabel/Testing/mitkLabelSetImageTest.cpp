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
#include <mitkLabelSetImageConverter.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <mitkAutoCropImageFilter.h>

namespace CppUnit
{
  namespace StringHelper
  {
    template<> inline std::string toString(const mitk::MultiLabelSegmentation::LabelValueVectorType& lvs)
    {
      std::ostringstream stream;
      stream << "[";
      for (mitk::MultiLabelSegmentation::LabelValueVectorType::const_iterator iter = lvs.begin(); iter!=lvs.end(); ++iter)
      {
        stream << *iter;
        if (iter + 1 != lvs.end()) stream << ", ";

      }
      stream << "]";
      return stream.str();
    }

    template<> inline std::string toString(const std::vector<std::string>& strings)
    {
      std::ostringstream stream;
      stream << "[";
      for (std::vector<std::string>::const_iterator iter = strings.begin(); iter != strings.end(); ++iter)
      {
        stream << *iter;
        if (iter + 1 != strings.end()) stream << ", ";

      }
      stream << "]";
      return stream.str();
    }

  }
}

class mitkLabelSetImageTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkLabelSetImageTestSuite);
  MITK_TEST(TestInitialize);
  MITK_TEST(TestClone);
  MITK_TEST(TestAddLayer);
  MITK_TEST(TestGetActiveLabelSet);
  MITK_TEST(TestGetActiveLabel);
  MITK_TEST(TestInitializeByLabeledImage);
  MITK_TEST(TestGetLabel);
  MITK_TEST(TestGetLabelValues);
  MITK_TEST(TestGetLabelClassNames);
  MITK_TEST(TestSetUnlabeledLabelLock);
  MITK_TEST(TestGetTotalNumberOfLabels);
  MITK_TEST(TestGetNumberOfLabels);
  MITK_TEST(TestExistsLabel);
  MITK_TEST(TestExistsGroup);
  MITK_TEST(TestSetActiveLayer);
  MITK_TEST(TestRemoveLayer);
  MITK_TEST(TestRemoveLabels);
  MITK_TEST(TestEraseLabels);
  MITK_TEST(TestMergeLabels);
  MITK_TEST(TestCreateLabelMask);
  CPPUNIT_TEST_SUITE_END();

private:
  mitk::MultiLabelSegmentation::Pointer m_LabelSetImage;
  int m_LabelAddedEventCount;
  int m_LabelModifiedEventCount;
  int m_LabelRemovedEventCount;
  int m_LabelsChangedEventCount;
  int m_GroupAddedEventCount;
  int m_GroupModifiedEventCount;
  int m_GroupRemovedEventCount;

public:
  void setUp() override
  {
    // Create a new labelset image
    m_LabelSetImage = mitk::MultiLabelSegmentation::New();
    mitk::Image::Pointer regularImage = mitk::Image::New();
    unsigned int dimensions[3] = { 96, 128, 52 };
    regularImage->Initialize(mitk::MakeScalarPixelType<char>(), 3, dimensions);
    m_LabelSetImage->Initialize(regularImage);

    this->ResetEvents();
    m_LabelSetImage->AddObserver(mitk::LabelAddedEvent(), [this](const itk::EventObject&) { ++(this->m_LabelAddedEventCount); });
    m_LabelSetImage->AddObserver(mitk::LabelModifiedEvent(), [this](const itk::EventObject&) { ++(this->m_LabelModifiedEventCount); });
    m_LabelSetImage->AddObserver(mitk::LabelRemovedEvent(), [this](const itk::EventObject&) { ++(this->m_LabelRemovedEventCount); });
    m_LabelSetImage->AddObserver(mitk::LabelsChangedEvent(), [this](const itk::EventObject&) { ++(this->m_LabelsChangedEventCount); });
    m_LabelSetImage->AddObserver(mitk::GroupAddedEvent(), [this](const itk::EventObject&) { ++(this->m_GroupAddedEventCount); });
    m_LabelSetImage->AddObserver(mitk::GroupModifiedEvent(), [this](const itk::EventObject&) { ++(this->m_GroupModifiedEventCount); });
    m_LabelSetImage->AddObserver(mitk::GroupRemovedEvent(), [this](const itk::EventObject&) { ++(this->m_GroupRemovedEventCount); });
  }

  void tearDown() override
  {
    // Delete MultiLabelSegmentation
    m_LabelSetImage = nullptr;
  }

  void ResetEvents()
  {
    m_LabelAddedEventCount = 0;
    m_LabelModifiedEventCount = 0;
    m_LabelRemovedEventCount = 0;
    m_LabelsChangedEventCount = 0;
    m_GroupAddedEventCount = 0;
    m_GroupModifiedEventCount = 0;
    m_GroupRemovedEventCount = 0;
  }

  bool CheckEvents(int lAdd, int lMod, int lRem, int lsC, int gAdd, int gMod, int gRem)
  {
    return m_GroupAddedEventCount == gAdd && m_GroupModifiedEventCount == gMod && m_GroupRemovedEventCount == gRem
      && m_LabelAddedEventCount == lAdd && m_LabelModifiedEventCount == lMod && m_LabelRemovedEventCount == lRem
      && m_LabelsChangedEventCount == lsC;
  }

  void InitializeTestSegmentation()
  {
    mitk::Label::Pointer label1 = mitk::Label::New(1, "Label1");
    mitk::Label::Pointer label2 = mitk::Label::New(20, "Label2");
    mitk::Label::Pointer label22 = mitk::Label::New(22, "Label2");
    mitk::Label::Pointer label3 = mitk::Label::New(30, "Label3");

    m_LabelSetImage->AddLabel(label1, 0);
    m_LabelSetImage->AddLayer({ label2, label22, label3 });
    m_LabelSetImage->AddLayer();
    this->ResetEvents();
  }

  void TestInitialize()
  {
    // LabelSet image should always has the pixel type mitk::Label::PixelType
    CPPUNIT_ASSERT_MESSAGE("MultiLabelSegmentation has wrong pixel type",
                           m_LabelSetImage->GetPixelType() == mitk::MakeScalarPixelType<mitk::Label::PixelType>());

    mitk::Image::Pointer regularImage = mitk::Image::New();
    unsigned int dimensions[3] = { 96, 128, 52 };
    regularImage->Initialize(mitk::MakeScalarPixelType<char>(), 3, dimensions);

    mitk::BaseGeometry::Pointer regularImageGeo = regularImage->GetGeometry();
    mitk::BaseGeometry::Pointer labelImageGeo = m_LabelSetImage->GetGeometry();
    MITK_ASSERT_EQUAL(labelImageGeo, regularImageGeo, "MultiLabelSegmentation has wrong geometry");

    // By default one layer should be added
    CPPUNIT_ASSERT_MESSAGE("Image was not correctly initialized - number of layers is not one",
                           m_LabelSetImage->GetNumberOfLayers() == 1);
    CPPUNIT_ASSERT_MESSAGE("Image was not correctly initialized - active layer has wrong ID",
                           m_LabelSetImage->GetActiveLayer() == 0);

    CPPUNIT_ASSERT_MESSAGE("Image was not correctly initialized - no active label should be selected",
                           m_LabelSetImage->GetActiveLabel() == nullptr);
  }

  void TestClone()
  {
    mitk::Label::Pointer label1 = mitk::Label::New();
    label1->SetName("Label1");
    label1->SetValue(1);

    mitk::Label::Pointer label2 = mitk::Label::New();
    label2->SetName("Label2");
    label2->SetValue(200);

    mitk::Label::Pointer label3 = mitk::Label::New();
    label2->SetName("Label3");
    label2->SetValue(300);

    m_LabelSetImage->AddLabel(label1, 0);
    m_LabelSetImage->AddLayer({ label2, label3 });

    auto clone = m_LabelSetImage->Clone();
    MITK_ASSERT_EQUAL(m_LabelSetImage, clone, "MultiLabelSegmentation clone is not equal.");
  }

  void TestAddLayer()
  {
    CPPUNIT_ASSERT_MESSAGE("Number of layers is not zero", m_LabelSetImage->GetNumberOfLayers() == 1);

    m_LabelSetImage->AddLayer();
    CPPUNIT_ASSERT_MESSAGE("Layer was not added correctly to image - number of layers is not one",
                           m_LabelSetImage->GetNumberOfLayers() == 2);
    CPPUNIT_ASSERT_MESSAGE("Layer was not added correctly to image - active layer has wrong ID",
                           m_LabelSetImage->GetActiveLayer() == 0);

    CPPUNIT_ASSERT_MESSAGE("Layer was not added correctly to image - no active label should be selected",
                           m_LabelSetImage->GetActiveLabel() == nullptr);

    CPPUNIT_ASSERT_MESSAGE("Event count incorrect", CheckEvents(0,0,0,0,1,0,0));

    mitk::Label::Pointer label1 = mitk::Label::New();
    label1->SetName("Label1");
    label1->SetValue(1);

    mitk::Label::Pointer label2 = mitk::Label::New();
    label2->SetName("Label2");
    label2->SetValue(200);


    const auto layerID = m_LabelSetImage->AddLayer({ label1, label2 });
    m_LabelSetImage->SetActiveLabel(200);
    CPPUNIT_ASSERT_MESSAGE("Layer was not added correctly to image - number of layers is not two",
                           m_LabelSetImage->GetNumberOfLayers() == 3);
    CPPUNIT_ASSERT_MESSAGE("Layer was not added correctly to image - active layer has wrong ID",
                           m_LabelSetImage->GetActiveLayer() == layerID);
    CPPUNIT_ASSERT_MESSAGE("Layer was not added correctly to image - active label is wrong",
                           m_LabelSetImage->GetActiveLabel()->GetValue() == 200);

    CPPUNIT_ASSERT_MESSAGE("Event count incorrect", CheckEvents(0, 0, 0, 0, 2, 0, 0));
  }

  void TestGetActiveLabelSet()
  {
    mitk::Label::Pointer label1 = mitk::Label::New();
    label1->SetName("Label1");
    label1->SetValue(1);

    mitk::Label::Pointer label2 = mitk::Label::New();
    label2->SetName("Label2");
    label2->SetValue(200);

    mitk::MultiLabelSegmentation::ConstLabelVectorType refLayer = { label1, label2 };
    unsigned int layerID = m_LabelSetImage->AddLayer(refLayer);
    m_LabelSetImage->SetActiveLabel(200);

    auto activeLayer = m_LabelSetImage->GetConstLabelsByValue(m_LabelSetImage->GetLabelValuesByGroup(m_LabelSetImage->GetActiveLayer()));

    CPPUNIT_ASSERT_MESSAGE("Wrong layer ID was returned", layerID == 1);
    CPPUNIT_ASSERT_MESSAGE("Wrong layer ID was returned", layerID == m_LabelSetImage->GetActiveLayer());

    CPPUNIT_ASSERT_MESSAGE("Wrong active labelset returned", mitk::Equal(refLayer, activeLayer, 0.00001, true));
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

    m_LabelSetImage->AddLabel(label1,0);
    m_LabelSetImage->AddLabel(label2,0);
    m_LabelSetImage->SetActiveLabel(1);
    CPPUNIT_ASSERT_MESSAGE("Layer was not added correctly to image - active label is wrong",
                           m_LabelSetImage->GetActiveLabel()->GetValue() == value1);
    m_LabelSetImage->SetActiveLabel(value2);
    CPPUNIT_ASSERT_MESSAGE("Layer was not added correctly to image - active label is wrong",
                           m_LabelSetImage->GetActiveLabel()->GetValue() == value2);

    CPPUNIT_ASSERT_MESSAGE("Active Label was not correctly retrieved with const getter",
      const_cast<const mitk::MultiLabelSegmentation*>(m_LabelSetImage.GetPointer())->GetActiveLabel()->GetValue() == value2);

  }

  void TestInitializeByLabeledImage()
  {
    mitk::Image::Pointer image =
      mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("Multilabel/LabelSetTestInitializeImage.nrrd"));
    m_LabelSetImage->InitializeByLabeledImage(image);
    CPPUNIT_ASSERT_MESSAGE("Image - number of labels is not 5", m_LabelSetImage->GetNumberOfLabels(0) == 5);
  }

  void TestGetLabel()
  {
    mitk::Label::Pointer label1 = mitk::Label::New(1, "Label1");
    mitk::Label::Pointer label2 = mitk::Label::New(20,"Label2");

    m_LabelSetImage->AddLabel(label1,0);
    m_LabelSetImage->AddLayer();
    m_LabelSetImage->AddLabel(label2,1);
    this->ResetEvents();

    CPPUNIT_ASSERT_MESSAGE("Wrong label retrieved for active layer",
                           mitk::Equal(*m_LabelSetImage->GetLabel(1), *label1, 0.0001, true));
    CPPUNIT_ASSERT_MESSAGE("Wrong label retrieved for layer 1",
                           mitk::Equal(*m_LabelSetImage->GetLabel(20), *label2, 0.0001, true));

    // Try to get a non existing label
    mitk::Label *unkownLabel = m_LabelSetImage->GetLabel(1000);
    CPPUNIT_ASSERT_MESSAGE("Non existing label should be nullptr", unkownLabel == nullptr);
    CPPUNIT_ASSERT_MESSAGE("Event count incorrect", CheckEvents(0, 0, 0, 0, 0, 0, 0));
  }

  void TestGetLabelValues()
  {
    InitializeTestSegmentation();

    auto labels = m_LabelSetImage->GetLabelValuesByGroup(0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong label values retrieved for group 0",
      mitk::MultiLabelSegmentation::LabelValueVectorType({ 1 }), labels);

    labels = m_LabelSetImage->GetLabelValuesByGroup(1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong label values retrieved for group 1",
      mitk::MultiLabelSegmentation::LabelValueVectorType({ 20, 22, 30 }), labels);

    labels = m_LabelSetImage->GetLabelValuesByGroup(2);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong label values retrieved for group 2",
      mitk::MultiLabelSegmentation::LabelValueVectorType(), labels);

    CPPUNIT_ASSERT_THROW(m_LabelSetImage->GetLabelValuesByGroup(3), mitk::Exception);

    labels = m_LabelSetImage->GetLabelValuesByName(0, "Label1");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong label values retrieved for \"Label2\" in group 0",
      mitk::MultiLabelSegmentation::LabelValueVectorType({ 1 }), labels);

    labels = m_LabelSetImage->GetLabelValuesByName(1, "Label2");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong label values retrieved for \"Label2\" in group 1",
      mitk::MultiLabelSegmentation::LabelValueVectorType({ 20, 22 }), labels);

    labels = m_LabelSetImage->GetLabelValuesByName(1, "Label3");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong label values retrieved for \"Label3\" in group 1",
      mitk::MultiLabelSegmentation::LabelValueVectorType({ 30 }), labels);

    labels = m_LabelSetImage->GetLabelValuesByName(2, "Label1");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong label values retrieved for group 2",
      mitk::MultiLabelSegmentation::LabelValueVectorType(), labels);

    labels = m_LabelSetImage->GetLabelValuesByName(0, "unkown");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong label values retrieved for unkown name",
      mitk::MultiLabelSegmentation::LabelValueVectorType(), labels);

    CPPUNIT_ASSERT_THROW(m_LabelSetImage->GetLabelValuesByName(3,"invalid"), mitk::Exception);

    labels = m_LabelSetImage->GetAllLabelValues();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong label values retrieved for unkown name",
      mitk::MultiLabelSegmentation::LabelValueVectorType({1,20,22,30}), labels);

    CPPUNIT_ASSERT_MESSAGE("Event count incorrect", CheckEvents(0, 0, 0, 0, 0, 0, 0));
  }


  void TestGetLabelClassNames()
  {
    InitializeTestSegmentation();

    auto names = m_LabelSetImage->GetLabelClassNames();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong names retrieved",
      std::vector<std::string>({ "Label1", "Label2", "Label3"}), names);
    names = m_LabelSetImage->GetLabelClassNamesByGroup(0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong names retrieved for group 0",
      std::vector<std::string>({ "Label1"}), names);
    names = m_LabelSetImage->GetLabelClassNamesByGroup(1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong names retrieved for group 1",
      std::vector<std::string>({"Label2", "Label3" }), names);
    names = m_LabelSetImage->GetLabelClassNamesByGroup(2);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong names retrieved for group 2",
      std::vector<std::string>(), names);

    CPPUNIT_ASSERT_THROW(m_LabelSetImage->GetLabelValuesByGroup(3), mitk::Exception);

    CPPUNIT_ASSERT_MESSAGE("Event count incorrect", CheckEvents(0, 0, 0, 0, 0, 0, 0));
  }

  void TestSetUnlabeledLabelLock()
  {
    auto locked = m_LabelSetImage->GetUnlabeledLabelLock();
    CPPUNIT_ASSERT_MESSAGE("Wrong UnlabeledLabelLock default state",
      locked == false);

    m_LabelSetImage->SetUnlabeledLabelLock(true);
    locked = m_LabelSetImage->GetUnlabeledLabelLock();
    CPPUNIT_ASSERT_MESSAGE("Wrong UnlabeledLabelLock state",
      locked == true);
  }

  void TestGetTotalNumberOfLabels()
  {
    this->InitializeTestSegmentation();
    CPPUNIT_ASSERT_MESSAGE(
      "Wrong total number of labels",
      m_LabelSetImage->GetTotalNumberOfLabels() == 4);
    CPPUNIT_ASSERT_MESSAGE("Event count incorrect", CheckEvents(0, 0, 0, 0, 0, 0, 0));
  }

  void TestGetNumberOfLabels()
  {
    this->InitializeTestSegmentation();
    CPPUNIT_ASSERT_MESSAGE(
      "Wrong number of labels in group 0",
      m_LabelSetImage->GetNumberOfLabels(0) == 1);
    CPPUNIT_ASSERT_MESSAGE(
      "Wrong number of labels in group 1",
      m_LabelSetImage->GetNumberOfLabels(1) == 3);
    CPPUNIT_ASSERT_MESSAGE(
      "Wrong number of labels in group 2",
      m_LabelSetImage->GetNumberOfLabels(2) == 0);

    CPPUNIT_ASSERT_THROW(m_LabelSetImage->GetNumberOfLabels(3), mitk::Exception);

    CPPUNIT_ASSERT_MESSAGE("Event count incorrect", CheckEvents(0, 0, 0, 0, 0, 0, 0));
  }

  void TestExistsLabel()
  {
    mitk::Label::Pointer label = mitk::Label::New();
    label->SetName("Label2");
    mitk::Label::PixelType value = 200;
    label->SetValue(value);

    m_LabelSetImage->AddLayer();
    m_LabelSetImage->AddLabel(label,1);
    m_LabelSetImage->SetActiveLayer(0);
    CPPUNIT_ASSERT_MESSAGE("Existing label was not found", m_LabelSetImage->ExistLabel(value) == true);

    CPPUNIT_ASSERT_MESSAGE("Non existing label was found", m_LabelSetImage->ExistLabel(10000) == false);
  }

  void TestExistsGroup()
  {
    mitk::Label::Pointer label1 = mitk::Label::New();
    label1->SetName("Label1");
    label1->SetValue(1);

    mitk::Label::Pointer label2 = mitk::Label::New();
    label2->SetName("Label2");
    label2->SetValue(200);

    m_LabelSetImage->AddLayer({label1, label2});

    CPPUNIT_ASSERT_MESSAGE("Check for existing layer failed", m_LabelSetImage->ExistGroup(0) == true);
    CPPUNIT_ASSERT_MESSAGE("Check for existing layer failed", m_LabelSetImage->ExistGroup(1) == true);
    CPPUNIT_ASSERT_MESSAGE("Check for existing layer failed", m_LabelSetImage->ExistGroup(20) == false);
  }

  void TestSetActiveLayer()
  {
    // Cache active layer
    auto refActiveLayer = m_LabelSetImage->GetConstLabelsByValue(m_LabelSetImage->GetLabelValuesByGroup(m_LabelSetImage->GetActiveLayer()));

    // Add new layer
    mitk::Label::Pointer label1 = mitk::Label::New();
    label1->SetName("Label1");
    label1->SetValue(1);

    mitk::Label::Pointer label2 = mitk::Label::New();
    label2->SetName("Label2");
    label2->SetValue(200);

    mitk::MultiLabelSegmentation::ConstLabelVectorType newlayer = { label1, label2 };
    unsigned int layerID = m_LabelSetImage->AddLayer(newlayer);

    // Set initial layer as active layer
    m_LabelSetImage->SetActiveLayer(0);
    auto activeLayer = m_LabelSetImage->GetConstLabelsByValue(m_LabelSetImage->GetLabelValuesByGroup(m_LabelSetImage->GetActiveLayer()));
    CPPUNIT_ASSERT_MESSAGE("Wrong active labelset returned",
                           mitk::Equal(refActiveLayer, activeLayer, 0.00001, true));

    // Set previously added layer as active layer
    m_LabelSetImage->SetActiveLayer(layerID);
    activeLayer = m_LabelSetImage->GetConstLabelsByValue(m_LabelSetImage->GetLabelValuesByGroup(m_LabelSetImage->GetActiveLayer()));
    CPPUNIT_ASSERT_MESSAGE("Wrong active labelset returned",
                           mitk::Equal(newlayer, activeLayer, 0.00001, true));

    // Set a non existing layer as active layer - nothing should change
    m_LabelSetImage->SetActiveLayer(10000);
    CPPUNIT_ASSERT_MESSAGE("Wrong active labelset returned",
                           mitk::Equal(newlayer, activeLayer, 0.00001, true));
  }

  void TestRemoveLayer()
  {
    // Cache active layer
    auto refActiveLayer = m_LabelSetImage->GetConstLabelsByValue(m_LabelSetImage->GetLabelValuesByGroup(m_LabelSetImage->GetActiveLayer()));

    // Add new layers
    m_LabelSetImage->AddLayer();

    mitk::Label::Pointer label1 = mitk::Label::New();
    label1->SetName("Label1");
    label1->SetValue(1);

    mitk::Label::Pointer label2 = mitk::Label::New();
    label2->SetName("Label2");
    label2->SetValue(200);

    mitk::MultiLabelSegmentation::ConstLabelVectorType newlayer = { label1, label2 };
    unsigned int layerID = m_LabelSetImage->AddLayer(newlayer);
    m_LabelSetImage->SetActiveLayer(layerID);

    auto activeLayer = m_LabelSetImage->GetConstLabelsByValue(m_LabelSetImage->GetLabelValuesByGroup(m_LabelSetImage->GetActiveLayer()));
    CPPUNIT_ASSERT_MESSAGE("Wrong active labelset returned",
                           mitk::Equal(newlayer, activeLayer, 0.00001, true));

    m_LabelSetImage->RemoveGroup(1);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of layers, after a layer was removed",
                           m_LabelSetImage->GetNumberOfLayers() == 2);
    CPPUNIT_ASSERT_MESSAGE("Check for existing layer failed", m_LabelSetImage->ExistGroup(2) == false);
    CPPUNIT_ASSERT_MESSAGE("Check for existing layer failed", m_LabelSetImage->ExistGroup(1) == true);
    CPPUNIT_ASSERT_MESSAGE("Check for existing layer failed", m_LabelSetImage->ExistGroup(0) == true);

    m_LabelSetImage->RemoveGroup(1);
    activeLayer = m_LabelSetImage->GetConstLabelsByValue(m_LabelSetImage->GetLabelValuesByGroup(m_LabelSetImage->GetActiveLayer()));
    CPPUNIT_ASSERT_MESSAGE("Wrong number of layers, after a layer was removed",
                           m_LabelSetImage->GetNumberOfLayers() == 1);
    CPPUNIT_ASSERT_MESSAGE("Check for existing layer failed", m_LabelSetImage->ExistGroup(1) == false);
    CPPUNIT_ASSERT_MESSAGE("Check for existing layer failed", m_LabelSetImage->ExistGroup(0) == true);
    CPPUNIT_ASSERT_MESSAGE("Wrong active layer",
                           mitk::Equal(refActiveLayer, activeLayer, 0.00001, true));

    m_LabelSetImage->RemoveGroup(0);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of layers, after a layer was removed",
                           m_LabelSetImage->GetNumberOfLayers() == 0);
    CPPUNIT_ASSERT_MESSAGE("Check for existing layer failed", m_LabelSetImage->ExistGroup(0) == false);
    CPPUNIT_ASSERT_THROW_MESSAGE("GetActiveLayers does not fail although all layer have been removed",
                           m_LabelSetImage->GetActiveLayer(), mitk::Exception);
  }

  void TestRemoveLabels()
  {
    mitk::Image::Pointer image =
      mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("Multilabel/LabelSetTestInitializeImage.nrrd"));

    m_LabelSetImage = nullptr;
    m_LabelSetImage = mitk::MultiLabelSegmentation::New();
    m_LabelSetImage->InitializeByLabeledImage(image);

    CPPUNIT_ASSERT_MESSAGE("Image - number of labels is not 6", m_LabelSetImage->GetNumberOfLabels(0) == 5);
    // 2ndMin because of unlabeled pixels = 0
    CPPUNIT_ASSERT_MESSAGE("Wrong MIN value", m_LabelSetImage->GetStatistics()->GetScalarValue2ndMin() == 1);
    CPPUNIT_ASSERT_MESSAGE("Wrong MAX value", m_LabelSetImage->GetStatistics()->GetScalarValueMax() == 7);

    CPPUNIT_ASSERT_MESSAGE("Label with ID 3 does not exist after initialization",
                           m_LabelSetImage->ExistLabel(3) == true);

    m_LabelSetImage->RemoveLabel(1);
    std::vector<mitk::Label::PixelType> labelsToBeRemoved;
    labelsToBeRemoved.push_back(3);
    labelsToBeRemoved.push_back(7);
    m_LabelSetImage->RemoveLabels(labelsToBeRemoved);

    CPPUNIT_ASSERT_MESSAGE("Wrong number of labels after some have been removed",
                           m_LabelSetImage->GetNumberOfLabels(0) == 2);
    // Values within the image are 0, 1, 3, 5, 6, 7 - New Min / Max value should be 5 / 6
    // 2ndMin because of unlabeled pixels = 0
    CPPUNIT_ASSERT_MESSAGE("Labels with value 1 and 3 were not removed from the image",
                           m_LabelSetImage->GetStatistics()->GetScalarValue2ndMin() == 5);
    CPPUNIT_ASSERT_MESSAGE("Label with value 7 was not removed from the image",
                           m_LabelSetImage->GetStatistics()->GetScalarValueMax() == 6);
  }

  void TestEraseLabels()
  {
    mitk::Image::Pointer image =
      mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("Multilabel/LabelSetTestInitializeImage.nrrd"));

    m_LabelSetImage = nullptr;
    m_LabelSetImage = mitk::MultiLabelSegmentation::New();
    m_LabelSetImage->InitializeByLabeledImage(image);

    CPPUNIT_ASSERT_MESSAGE("Image - number of labels is not 6", m_LabelSetImage->GetNumberOfLabels(0) == 5);
    // 2ndMin because of unlabeled pixels = 0
    CPPUNIT_ASSERT_MESSAGE("Wrong MIN value", m_LabelSetImage->GetStatistics()->GetScalarValue2ndMin() == 1);
    CPPUNIT_ASSERT_MESSAGE("Wrong MAX value", m_LabelSetImage->GetStatistics()->GetScalarValueMax() == 7);

    CPPUNIT_ASSERT_MESSAGE("Label with ID 3 does not exist after initialization",
      m_LabelSetImage->ExistLabel(3) == true);

    m_LabelSetImage->EraseLabel(1);
    std::vector<mitk::Label::PixelType> labelsToBeErased;
    labelsToBeErased.push_back(3);
    labelsToBeErased.push_back(7);
    m_LabelSetImage->EraseLabels(labelsToBeErased);

    CPPUNIT_ASSERT_MESSAGE("Wrong number of labels since none have been removed",
      m_LabelSetImage->GetNumberOfLabels(0) == 5);
    // Values within the image are 0, 1, 3, 5, 6, 7 - New Min / Max value should be 5 / 6
    // 2ndMin because of unlabeled pixels = 0
    CPPUNIT_ASSERT_MESSAGE("Labels with value 1 and 3 were not erased from the image",
      m_LabelSetImage->GetStatistics()->GetScalarValue2ndMin() == 5);
    CPPUNIT_ASSERT_MESSAGE("Label with value 7 was not erased from the image",
      m_LabelSetImage->GetStatistics()->GetScalarValueMax() == 6);
  }

  void TestMergeLabels()
  {
    mitk::Image::Pointer image =
      mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("Multilabel/LabelSetTestInitializeImage.nrrd"));

    m_LabelSetImage = nullptr;
    m_LabelSetImage = mitk::MultiLabelSegmentation::New();
    m_LabelSetImage->InitializeByLabeledImage(image);

    CPPUNIT_ASSERT_MESSAGE("Image - number of labels is not 6", m_LabelSetImage->GetNumberOfLabels(0) == 5);
    // 2ndMin because of unlabeled pixels = 0
    CPPUNIT_ASSERT_MESSAGE("Wrong MIN value", m_LabelSetImage->GetStatistics()->GetScalarValue2ndMin() == 1);
    CPPUNIT_ASSERT_MESSAGE("Wrong MAX value", m_LabelSetImage->GetStatistics()->GetScalarValueMax() == 7);

    CPPUNIT_ASSERT_MESSAGE("Label with ID 6 does not exist after initialization",
      m_LabelSetImage->ExistLabel(6) == true);

    // Merge label 7 with label 6. Result should be that label 7 is not present anymore.
    m_LabelSetImage->MergeLabel(6, 7);
    CPPUNIT_ASSERT_MESSAGE("Label with value 7 was not removed from the image", m_LabelSetImage->GetStatistics()->GetScalarValueMax() == 6);

    // Count all pixels with value 6 = 507
    // Count all pixels with value 7 = 823
    // Check if merged label has 507 + 823 = 1330 pixels
    CPPUNIT_ASSERT_MESSAGE("Labels were not correctly merged", m_LabelSetImage->GetStatistics()->GetCountOfMaxValuedVoxels() == 1330);

    CPPUNIT_ASSERT_MESSAGE("Label with ID 3 does not exist after initialization",
      m_LabelSetImage->ExistLabel(3) == true);
    CPPUNIT_ASSERT_MESSAGE("Label with ID 5 does not exist after initialization",
      m_LabelSetImage->ExistLabel(5) == true);

    // Merge labels 5 and 6 with 3. Result should be that labels 5 and 6 are not present anymore.
    std::vector<mitk::Label::PixelType> vectorOfSourcePixelValues{ 5, 6 };
    m_LabelSetImage->MergeLabels(3, vectorOfSourcePixelValues);
    // Values within the image are 0, 1, 3, 5, 6, 7 - New Max value should be 3
    CPPUNIT_ASSERT_MESSAGE("Labels with value 5 and 6 were not removed from the image", m_LabelSetImage->GetStatistics()->GetScalarValueMax() == 3);

    // Count all pixels with value 3 = 1893
    // Count all pixels with value 5 = 2143
    // Count all pixels with value 6 = 1330
    // Check if merged label has 1893 + 2143 + 1330 = 5366 pixels
    CPPUNIT_ASSERT_MESSAGE("Labels were not correctly merged", m_LabelSetImage->GetStatistics()->GetCountOfMaxValuedVoxels() == 5366);
  }

  void TestCreateLabelMask()
  {
    mitk::Image::Pointer image =
      mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("Multilabel/LabelSetTestInitializeImage.nrrd"));

    m_LabelSetImage = nullptr;
    m_LabelSetImage = mitk::MultiLabelSegmentation::New();
    m_LabelSetImage->InitializeByLabeledImage(image);

    auto labelMask = mitk::CreateLabelMask(m_LabelSetImage,6);

    mitk::AutoCropImageFilter::Pointer cropFilter = mitk::AutoCropImageFilter::New();
    cropFilter->SetInput(labelMask);
    cropFilter->SetBackgroundValue(0);
    cropFilter->SetMarginFactor(1.15);
    cropFilter->Update();
    auto maskImage = cropFilter->GetOutput();

    // Count all pixels with value 6 = 507
    CPPUNIT_ASSERT_MESSAGE("Label mask not correctly created", maskImage->GetStatistics()->GetCountOfMaxValuedVoxels() == 507);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkLabelSetImage)
