/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkImageStatisticsTreeModel.h>

#include "QmitkTestQApplication.h"

#include <mitkImageStatisticsConstants.h>
#include <mitkImageStatisticsContainer.h>
#include <mitkImageStatisticsContainerManager.h>
#include <mitkImageStatisticsContainerNodeHelper.h>
#include <mitkProperties.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkStatisticsToImageRelationRule.h>
#include <mitkStatisticsToMaskRelationRule.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <QIcon>

class QmitkImageStatisticsTreeModelTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkImageStatisticsTreeModelTestSuite);
  MITK_TEST(ImageNodeRenamed_ModelIsUpdated);
  MITK_TEST(ImageNodeWithNameOnDataRenamed_ModelIsUpdated);
  MITK_TEST(MaskNodeRenamed_ModelIsUpdated);
  MITK_TEST(LabelRenamed_ModelIsUpdated);
  MITK_TEST(LabelRenamedToAmbiguousName_ModelShowsDisambiguation);
  MITK_TEST(LabelRecolored_ModelIsUpdated);
  MITK_TEST(MultipleLabelsModified_UpdateIsCoalesced);
  MITK_TEST(InputReplaced_FormerInputIsNoLongerObserved);
  CPPUNIT_TEST_SUITE_END();

  mitk::StandaloneDataStorage::Pointer m_DataStorage;
  mitk::Image::Pointer m_Image;
  mitk::DataNode::Pointer m_ImageNode;
  mitk::MultiLabelSegmentation::Pointer m_Mask;
  mitk::DataNode::Pointer m_MaskNode;
  mitk::Label::PixelType m_LabelA;
  mitk::Label::PixelType m_LabelB;

public:

  static mitk::Image::Pointer CreateTestImage()
  {
    auto image = mitk::Image::New();
    unsigned int dimensions[3] = { 10, 10, 10 };
    image->Initialize(mitk::MakeScalarPixelType<char>(), 3, dimensions);

    return image;
  }

  void setUp() override
  {
    // The model creates QPixmaps for the label color decoration, which requires a live
    // QGuiApplication. Use the shared, never-destroyed instance so '-platform minimal'
    // from the driver args is honoured.
    EnsureQApplication();

    m_DataStorage = mitk::StandaloneDataStorage::New();

    m_Image = CreateTestImage();
    m_ImageNode = mitk::DataNode::New();
    m_ImageNode->SetData(m_Image);
    m_ImageNode->SetName("Image");
    m_DataStorage->Add(m_ImageNode);

    m_Mask = mitk::MultiLabelSegmentation::New();
    m_Mask->Initialize(CreateTestImage());

    mitk::Color red;
    red.Set(1.0f, 0.0f, 0.0f);
    mitk::Color green;
    green.Set(0.0f, 1.0f, 0.0f);

    m_LabelA = m_Mask->AddLabel("Label A", red, 0)->GetValue();
    m_LabelB = m_Mask->AddLabel("Label B", green, 0)->GetValue();

    m_MaskNode = mitk::DataNode::New();
    m_MaskNode->SetData(m_Mask);
    m_MaskNode->SetName("Mask");
    m_DataStorage->Add(m_MaskNode);

    // Added last on purpose: ImageStatisticsContainerManager only accepts statistics that
    // are newer than the image and the mask they belong to.
    this->AddStatistics(m_Image, m_Mask);
  }

  void tearDown() override
  {
    m_DataStorage = nullptr;
    m_ImageNode = nullptr;
    m_MaskNode = nullptr;
    m_Image = nullptr;
    m_Mask = nullptr;
  }

  void AddStatistics(const mitk::Image* image, const mitk::MultiLabelSegmentation* mask)
  {
    auto container = mitk::ImageStatisticsContainer::New();
    container->SetTimeGeometry(image->GetTimeGeometry()->Clone());
    container->SetProperty(mitk::STATS_HISTOGRAM_BIN_PROPERTY_NAME.c_str(), mitk::UIntProperty::New(100));
    container->SetProperty(mitk::STATS_IGNORE_ZERO_VOXEL_PROPERTY_NAME.c_str(), mitk::BoolProperty::New(false));

    mitk::ImageStatisticsContainer::ImageStatisticsObject statistics;
    statistics.AddStatistic(mitk::ImageStatisticsConstants::MEAN(), 1.0);

    for (const auto labelValue : mask->GetAllLabelValues())
      container->SetStatistics(labelValue, 0, statistics);

    mitk::StatisticsToImageRelationRule::New()->Connect(container.GetPointer(), image);
    mitk::StatisticsToMaskRelationRule::New()->Connect(container.GetPointer(), mask);

    m_DataStorage->Add(mitk::CreateImageStatisticsNode(container, "statistics"));
  }

  /** Lets the deferred model update run. */
  static void Settle()
  {
    QCoreApplication::processEvents();
  }

  static QModelIndex MaskIndex(const QmitkImageStatisticsTreeModel& model)
  {
    return model.index(0, 0, model.index(0, 0));
  }

  static QModelIndex LabelIndex(const QmitkImageStatisticsTreeModel& model, int row)
  {
    return model.index(row, 0, MaskIndex(model));
  }

  static std::string ImageText(const QmitkImageStatisticsTreeModel& model)
  {
    return model.data(model.index(0, 0), Qt::DisplayRole).toString().toStdString();
  }

  static std::string MaskText(const QmitkImageStatisticsTreeModel& model)
  {
    return model.data(MaskIndex(model), Qt::DisplayRole).toString().toStdString();
  }

  static std::string LabelText(const QmitkImageStatisticsTreeModel& model, int row)
  {
    return model.data(LabelIndex(model, row), Qt::DisplayRole).toString().toStdString();
  }

  static bool StartsWith(const std::string& text, const std::string& prefix)
  {
    return 0 == text.rfind(prefix, 0);
  }

  /** Sets up a model that shows the image and the mask of this fixture. */
  void SetUpModel(QmitkImageStatisticsTreeModel& model)
  {
    model.SetDataStorage(m_DataStorage);
    model.SetImageNodes({ m_ImageNode.GetPointer() });
    model.SetMaskNodes({ m_MaskNode.GetPointer() });

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Precondition failed: the statistics were not picked up.",
      std::string("Image"), ImageText(model));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Precondition failed: the mask was not picked up.",
      std::string("Mask"), MaskText(model));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Precondition failed: unexpected number of label rows.",
      2, model.rowCount(MaskIndex(model)));
  }

  void ImageNodeRenamed_ModelIsUpdated()
  {
    QmitkImageStatisticsTreeModel model;
    this->SetUpModel(model);

    m_ImageNode->SetName("Renamed image");
    Settle();

    CPPUNIT_ASSERT_EQUAL(std::string("Renamed image"), ImageText(model));
  }

  void ImageNodeWithNameOnDataRenamed_ModelIsUpdated()
  {
    // If the data carries the "name" property (e.g. for DICOM images), DataNode::SetName()
    // writes into it in place. That modifies neither the node nor the modification time of
    // the data, so no DataStorage::ChangedNodeEvent is sent at all.
    auto image = CreateTestImage();
    image->SetProperty("name", mitk::StringProperty::New("Image on data"));

    auto imageNode = mitk::DataNode::New();
    imageNode->SetData(image);
    m_DataStorage->Add(imageNode);
    this->AddStatistics(image, m_Mask);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Precondition failed: the name is not owned by the data.",
      std::string("Image on data"), imageNode->GetName());

    QmitkImageStatisticsTreeModel model;
    model.SetDataStorage(m_DataStorage);
    model.SetImageNodes({ imageNode.GetPointer() });
    model.SetMaskNodes({ m_MaskNode.GetPointer() });

    CPPUNIT_ASSERT_EQUAL(std::string("Image on data"), ImageText(model));

    imageNode->SetName("Renamed on data");
    Settle();

    CPPUNIT_ASSERT_EQUAL(std::string("Renamed on data"), ImageText(model));
  }

  void MaskNodeRenamed_ModelIsUpdated()
  {
    QmitkImageStatisticsTreeModel model;
    this->SetUpModel(model);

    m_MaskNode->SetName("Renamed mask");
    Settle();

    CPPUNIT_ASSERT_EQUAL(std::string("Renamed mask"), MaskText(model));
  }

  void LabelRenamed_ModelIsUpdated()
  {
    QmitkImageStatisticsTreeModel model;
    this->SetUpModel(model);

    CPPUNIT_ASSERT_EQUAL(std::string("Label A"), LabelText(model, 0));

    m_Mask->GetLabel(m_LabelA)->SetName("Renamed label");
    Settle();

    CPPUNIT_ASSERT_EQUAL(std::string("Renamed label"), LabelText(model, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The statistics must survive a label rename.",
      2, model.rowCount(MaskIndex(model)));
  }

  void LabelRenamedToAmbiguousName_ModelShowsDisambiguation()
  {
    QmitkImageStatisticsTreeModel model;
    this->SetUpModel(model);

    // Both labels share a name now, so CreateDisplayLabelName() has to disambiguate them.
    m_Mask->GetLabel(m_LabelB)->SetName("Label A");
    Settle();

    CPPUNIT_ASSERT(StartsWith(LabelText(model, 0), "Label A"));
    CPPUNIT_ASSERT(StartsWith(LabelText(model, 1), "Label A"));
    CPPUNIT_ASSERT(LabelText(model, 0) != std::string("Label A"));
    CPPUNIT_ASSERT(LabelText(model, 0) != LabelText(model, 1));
  }

  void LabelRecolored_ModelIsUpdated()
  {
    QmitkImageStatisticsTreeModel model;
    this->SetUpModel(model);

    int modelChangedCount = 0;
    QObject::connect(&model, &QmitkImageStatisticsTreeModel::modelChanged,
      [&modelChangedCount]() { ++modelChangedCount; });

    mitk::Color blue;
    blue.Set(0.0f, 0.0f, 1.0f);
    m_Mask->GetLabel(m_LabelA)->SetColor(blue);
    Settle();

    // The color itself is read live from the label, so the point of the test is that the
    // model asks the view to repaint at all.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A recolored label must trigger a model update.",
      1, modelChangedCount);

    const auto icon = model.data(LabelIndex(model, 0), Qt::DecorationRole).value<QIcon>();
    CPPUNIT_ASSERT_MESSAGE("No color decoration for the label.", !icon.isNull());
    CPPUNIT_ASSERT_EQUAL(QColor(Qt::blue).rgb(), icon.pixmap(QSize(20, 20)).toImage().pixel(0, 0));
  }

  void MultipleLabelsModified_UpdateIsCoalesced()
  {
    QmitkImageStatisticsTreeModel model;
    this->SetUpModel(model);

    int modelChangedCount = 0;
    QObject::connect(&model, &QmitkImageStatisticsTreeModel::modelChanged,
      [&modelChangedCount]() { ++modelChangedCount; });

    m_Mask->GetLabel(m_LabelA)->SetName("First");
    m_Mask->GetLabel(m_LabelB)->SetName("Second");
    Settle();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("A batch of label modifications must result in one update.",
      1, modelChangedCount);
    CPPUNIT_ASSERT_EQUAL(std::string("First"), LabelText(model, 0));
    CPPUNIT_ASSERT_EQUAL(std::string("Second"), LabelText(model, 1));
  }

  void InputReplaced_FormerInputIsNoLongerObserved()
  {
    auto otherImage = CreateTestImage();
    auto otherImageNode = mitk::DataNode::New();
    otherImageNode->SetData(otherImage);
    otherImageNode->SetName("Other image");
    m_DataStorage->Add(otherImageNode);
    this->AddStatistics(otherImage, m_Mask);

    QmitkImageStatisticsTreeModel model;
    this->SetUpModel(model);

    model.SetImageNodes({ otherImageNode.GetPointer() });
    CPPUNIT_ASSERT_EQUAL(std::string("Other image"), ImageText(model));

    // The former input must neither be observed any more nor crash the model.
    m_ImageNode->SetName("Renamed former input");
    Settle();

    CPPUNIT_ASSERT_EQUAL(std::string("Other image"), ImageText(model));

    otherImageNode->SetName("Renamed other image");
    Settle();

    CPPUNIT_ASSERT_EQUAL(std::string("Renamed other image"), ImageText(model));
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkImageStatisticsTreeModel)
