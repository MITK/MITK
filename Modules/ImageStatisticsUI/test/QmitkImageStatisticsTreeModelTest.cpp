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
#include <QLocale>

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
  MITK_TEST(LabelsAddedOutOfAlphabeticalOrder_ModelOrdersByName);
  MITK_TEST(MultipleGroups_ModelShowsGroupRowsInSegmentationViewOrder);
  MITK_TEST(GroupRenamed_ModelIsUpdated);
  MITK_TEST(LabelsCheckable_OnlyLabelRowsAreCheckable);
  MITK_TEST(LabelsCheckable_FirstLabelIsCheckedByDefault);
  MITK_TEST(LabelChecked_StateSurvivesLabelRename);
  MITK_TEST(MaskNodesSet_CheckStateIsReset);
  MITK_TEST(SingleLabel_NoCheckBoxesAndLabelCountsAsChecked);
  MITK_TEST(CheckedLabelRemoved_FirstRemainingLabelIsChecked);
  MITK_TEST(CheckedLabelRemoved_OtherCheckStatesAreKept);
  MITK_TEST(LabelsRecolored_InputDisplayChangedIsEmittedOnce);
  MITK_TEST(Headers_ShowReadableNamesAndToolTips);
  MITK_TEST(Headers_AreExportedByTheirKeys);
  MITK_TEST(Values_AreFormattedForReadingAndAvailableUnformatted);
  MITK_TEST(Values_ShareTheDecimalPlacesOfTheirColumn);
  MITK_TEST(RoundedValue_ToolTipShowsItUnrounded);
  CPPUNIT_TEST_SUITE_END();

  QLocale m_DefaultLocale;
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

    // Statistics are formatted in the locale of the user. Pin it, so that the expected
    // strings of the formatting tests hold on any machine.
    m_DefaultLocale = QLocale();
    QLocale::setDefault(QLocale::c());

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
    QLocale::setDefault(m_DefaultLocale);

    m_DataStorage = nullptr;
    m_ImageNode = nullptr;
    m_MaskNode = nullptr;
    m_Image = nullptr;
    m_Mask = nullptr;
  }

  void AddStatistics(const mitk::Image* image, const mitk::MultiLabelSegmentation* mask, double mean = 1.0)
  {
    auto container = mitk::ImageStatisticsContainer::New();
    container->SetTimeGeometry(image->GetTimeGeometry()->Clone());
    container->SetProperty(mitk::STATS_HISTOGRAM_BIN_PROPERTY_NAME.c_str(), mitk::UIntProperty::New(100));
    container->SetProperty(mitk::STATS_IGNORE_ZERO_VOXEL_PROPERTY_NAME.c_str(), mitk::BoolProperty::New(false));

    mitk::ImageStatisticsContainer::IndexType minPosition(3);
    minPosition[0] = 1;
    minPosition[1] = 2;
    minPosition[2] = 3;

    mitk::ImageStatisticsContainer::ImageStatisticsObject statistics;
    statistics.AddStatistic(mitk::ImageStatisticsConstants::MEAN(), mean);
    statistics.AddStatistic(mitk::ImageStatisticsConstants::NUMBEROFVOXELS(),
      static_cast<mitk::ImageStatisticsContainer::VoxelCountType>(999));
    statistics.AddStatistic(mitk::ImageStatisticsConstants::MINIMUMPOSITION(), minPosition);
    // Stands for the statistics other modules contribute, which the model knows nothing about.
    statistics.AddStatistic("CustomStat", 0.5);

    for (const auto labelValue : mask->GetAllLabelValues())
      container->SetStatistics(labelValue, 0, statistics);

    mitk::StatisticsToImageRelationRule::New()->Connect(container.GetPointer(), image);
    mitk::StatisticsToMaskRelationRule::New()->Connect(container.GetPointer(), mask);

    m_DataStorage->Add(mitk::CreateImageStatisticsNode(container, "statistics"));
  }

  /** Adds a node for the passed segmentation, together with statistics of the fixture image
  for all of its labels. Labels have to be added before, see AddStatistics(). */
  mitk::DataNode::Pointer AddMaskNode(mitk::MultiLabelSegmentation* mask, double mean = 1.0)
  {
    auto maskNode = mitk::DataNode::New();
    maskNode->SetData(mask);
    maskNode->SetName("Other mask");
    m_DataStorage->Add(maskNode);
    this->AddStatistics(m_Image, mask, mean);

    return maskNode;
  }

  static mitk::Label::PixelType AddLabel(mitk::MultiLabelSegmentation* mask, const std::string& name, mitk::MultiLabelSegmentation::GroupIndexType groupID)
  {
    mitk::Color white;
    white.Set(1.0f, 1.0f, 1.0f);

    return mask->AddLabel(name, white, groupID)->GetValue();
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

  static std::string Text(const QmitkImageStatisticsTreeModel& model, const QModelIndex& index)
  {
    return model.data(index, Qt::DisplayRole).toString().toStdString();
  }

  static std::string FirstColumnHeader(const QmitkImageStatisticsTreeModel& model)
  {
    return model.headerData(0, Qt::Horizontal, Qt::DisplayRole).toString().toStdString();
  }

  /** Returns the column headed by the passed name, or -1 if there is none. */
  static int ColumnOf(const QmitkImageStatisticsTreeModel& model, const QString& header)
  {
    for (int column = 1; column < model.columnCount(); ++column)
    {
      if (header == model.headerData(column, Qt::Horizontal, Qt::DisplayRole).toString())
        return column;
    }

    return -1;
  }

  static std::string ToolTip(const QmitkImageStatisticsTreeModel& model, int column)
  {
    return model.headerData(column, Qt::Horizontal, Qt::ToolTipRole).toString().toStdString();
  }

  /** Returns the technical key a column is exported by. */
  static std::string ExportedHeader(const QmitkImageStatisticsTreeModel& model, int column)
  {
    return model.headerData(column, Qt::Horizontal, Qt::EditRole).toString().toStdString();
  }

  /** Returns the cell of the passed statistic in the first label row. */
  static QModelIndex ValueIndex(const QmitkImageStatisticsTreeModel& model, const QString& header)
  {
    return model.index(0, ColumnOf(model, header), MaskIndex(model));
  }

  /** Returns the displayed value of the passed statistic in the first label row. */
  static std::string ValueText(const QmitkImageStatisticsTreeModel& model, const QString& header)
  {
    return model.data(ValueIndex(model, header), Qt::DisplayRole).toString().toStdString();
  }

  /** Returns the exported value of the passed statistic in the first label row. */
  static std::string ExportedValueText(const QmitkImageStatisticsTreeModel& model, const QString& header)
  {
    return model.data(ValueIndex(model, header), Qt::EditRole).toString().toStdString();
  }

  static bool IsCheckable(const QmitkImageStatisticsTreeModel& model, const QModelIndex& index)
  {
    return model.flags(index).testFlag(Qt::ItemIsUserCheckable);
  }

  static int CheckState(const QmitkImageStatisticsTreeModel& model, const QModelIndex& index)
  {
    return model.data(index, Qt::CheckStateRole).toInt();
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

    // Labels are ordered by name, so the renamed label moves behind "Label B".
    CPPUNIT_ASSERT_EQUAL(std::string("Label B"), LabelText(model, 0));
    CPPUNIT_ASSERT_EQUAL(std::string("Renamed label"), LabelText(model, 1));
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

  void LabelsAddedOutOfAlphabeticalOrder_ModelOrdersByName()
  {
    auto mask = mitk::MultiLabelSegmentation::New();
    mask->Initialize(CreateTestImage());
    AddLabel(mask, "Zeta", 0);
    AddLabel(mask, "Alpha", 0);
    auto maskNode = this->AddMaskNode(mask);

    QmitkImageStatisticsTreeModel model;
    model.SetDataStorage(m_DataStorage);
    model.SetImageNodes({ m_ImageNode.GetPointer() });
    model.SetMaskNodes({ maskNode.GetPointer() });

    // Ordered by label value, "Zeta" would come first.
    CPPUNIT_ASSERT_EQUAL(2, model.rowCount(MaskIndex(model)));
    CPPUNIT_ASSERT_EQUAL(std::string("Alpha"), LabelText(model, 0));
    CPPUNIT_ASSERT_EQUAL(std::string("Zeta"), LabelText(model, 1));
    CPPUNIT_ASSERT_MESSAGE("A single group must not add a group level.",
      !model.hasChildren(LabelIndex(model, 0)));
  }

  void MultipleGroups_ModelShowsGroupRowsInSegmentationViewOrder()
  {
    // Ordered by label value, this would read Label 1, Label 2, Label 3.
    auto mask = mitk::MultiLabelSegmentation::New();
    mask->Initialize(CreateTestImage());
    AddLabel(mask, "Label 1", 0);
    mask->AddGroup();
    AddLabel(mask, "Label 2", 1);
    AddLabel(mask, "Label 3", 0);
    auto maskNode = this->AddMaskNode(mask);

    QmitkImageStatisticsTreeModel model;
    model.SetDataStorage(m_DataStorage);
    model.SetImageNodes({ m_ImageNode.GetPointer() });
    model.SetMaskNodes({ maskNode.GetPointer() });

    const auto firstGroup = LabelIndex(model, 0);
    const auto secondGroup = LabelIndex(model, 1);

    CPPUNIT_ASSERT_EQUAL(2, model.rowCount(MaskIndex(model)));
    CPPUNIT_ASSERT_EQUAL(std::string("Group 1"), Text(model, firstGroup));
    CPPUNIT_ASSERT_EQUAL(std::string("Group 2"), Text(model, secondGroup));

    CPPUNIT_ASSERT_EQUAL(2, model.rowCount(firstGroup));
    CPPUNIT_ASSERT_EQUAL(std::string("Label 1"), Text(model, model.index(0, 0, firstGroup)));
    CPPUNIT_ASSERT_EQUAL(std::string("Label 3"), Text(model, model.index(1, 0, firstGroup)));

    CPPUNIT_ASSERT_EQUAL(1, model.rowCount(secondGroup));
    CPPUNIT_ASSERT_EQUAL(std::string("Label 2"), Text(model, model.index(0, 0, secondGroup)));
  }

  void GroupRenamed_ModelIsUpdated()
  {
    auto mask = mitk::MultiLabelSegmentation::New();
    mask->Initialize(CreateTestImage());
    AddLabel(mask, "Label 1", 0);
    mask->AddGroup();
    AddLabel(mask, "Label 2", 1);
    auto maskNode = this->AddMaskNode(mask);

    QmitkImageStatisticsTreeModel model;
    model.SetDataStorage(m_DataStorage);
    model.SetImageNodes({ m_ImageNode.GetPointer() });
    model.SetMaskNodes({ maskNode.GetPointer() });

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Precondition failed: no group rows.",
      std::string("Group 1"), LabelText(model, 0));

    mask->SetGroupName(0, "Organs");
    Settle();

    CPPUNIT_ASSERT_EQUAL(std::string("Organs"), LabelText(model, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The statistics must survive a group rename.",
      1, model.rowCount(LabelIndex(model, 0)));
  }

  void LabelsCheckable_OnlyLabelRowsAreCheckable()
  {
    QmitkImageStatisticsTreeModel model;
    this->SetUpModel(model);

    CPPUNIT_ASSERT_MESSAGE("Check boxes must be off by default.", !IsCheckable(model, LabelIndex(model, 0)));
    CPPUNIT_ASSERT(!model.data(LabelIndex(model, 0), Qt::CheckStateRole).isValid());

    model.SetLabelsCheckable(true);

    CPPUNIT_ASSERT(IsCheckable(model, LabelIndex(model, 0)));
    CPPUNIT_ASSERT(IsCheckable(model, LabelIndex(model, 1)));
    CPPUNIT_ASSERT_MESSAGE("Only the first column carries the check box.",
      !IsCheckable(model, model.index(0, 1, MaskIndex(model))));
    CPPUNIT_ASSERT(!IsCheckable(model, MaskIndex(model)));
    CPPUNIT_ASSERT(!IsCheckable(model, model.index(0, 0)));
    CPPUNIT_ASSERT(!model.data(MaskIndex(model), Qt::CheckStateRole).isValid());
  }

  void LabelsCheckable_FirstLabelIsCheckedByDefault()
  {
    QmitkImageStatisticsTreeModel model;
    this->SetUpModel(model);
    model.SetLabelsCheckable(true);

    CPPUNIT_ASSERT_EQUAL(static_cast<int>(Qt::Checked), CheckState(model, LabelIndex(model, 0)));
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(Qt::Unchecked), CheckState(model, LabelIndex(model, 1)));
    CPPUNIT_ASSERT(model.IsLabelChecked(m_LabelA));
    CPPUNIT_ASSERT(!model.IsLabelChecked(m_LabelB));
  }

  void LabelChecked_StateSurvivesLabelRename()
  {
    QmitkImageStatisticsTreeModel model;
    this->SetUpModel(model);
    model.SetLabelsCheckable(true);

    int checkStateChangedCount = 0;
    QObject::connect(&model, &QmitkImageStatisticsTreeModel::labelCheckStateChanged,
      [&checkStateChangedCount]() { ++checkStateChangedCount; });

    CPPUNIT_ASSERT(model.setData(LabelIndex(model, 1), Qt::Checked, Qt::CheckStateRole));
    CPPUNIT_ASSERT_EQUAL(1, checkStateChangedCount);
    CPPUNIT_ASSERT(model.IsLabelChecked(m_LabelB));

    CPPUNIT_ASSERT_MESSAGE("Setting the same state again must be a no-op.",
      !model.setData(LabelIndex(model, 1), Qt::Checked, Qt::CheckStateRole));
    CPPUNIT_ASSERT_EQUAL(1, checkStateChangedCount);

    // The rename moves "Label A" behind "Label B" and rebuilds the tree.
    m_Mask->GetLabel(m_LabelA)->SetName("Renamed label");
    Settle();

    CPPUNIT_ASSERT_EQUAL(std::string("Label B"), LabelText(model, 0));
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(Qt::Checked), CheckState(model, LabelIndex(model, 0)));
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(Qt::Checked), CheckState(model, LabelIndex(model, 1)));
    CPPUNIT_ASSERT(model.IsLabelChecked(m_LabelA));
    CPPUNIT_ASSERT(model.IsLabelChecked(m_LabelB));
  }

  void MaskNodesSet_CheckStateIsReset()
  {
    QmitkImageStatisticsTreeModel model;
    this->SetUpModel(model);
    model.SetLabelsCheckable(true);

    model.setData(LabelIndex(model, 1), Qt::Checked, Qt::CheckStateRole);
    model.setData(LabelIndex(model, 0), Qt::Unchecked, Qt::CheckStateRole);

    CPPUNIT_ASSERT_MESSAGE("Precondition failed: the check state was not changed.",
      !model.IsLabelChecked(m_LabelA) && model.IsLabelChecked(m_LabelB));

    model.SetMaskNodes({ m_MaskNode.GetPointer() });

    CPPUNIT_ASSERT(model.IsLabelChecked(m_LabelA));
    CPPUNIT_ASSERT(!model.IsLabelChecked(m_LabelB));
  }

  void SingleLabel_NoCheckBoxesAndLabelCountsAsChecked()
  {
    auto mask = mitk::MultiLabelSegmentation::New();
    mask->Initialize(CreateTestImage());
    const auto onlyLabel = AddLabel(mask, "Only label", 0);
    auto maskNode = this->AddMaskNode(mask);

    QmitkImageStatisticsTreeModel model;
    model.SetDataStorage(m_DataStorage);
    model.SetImageNodes({ m_ImageNode.GetPointer() });
    model.SetMaskNodes({ maskNode.GetPointer() });
    model.SetLabelsCheckable(true);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("A segmentation shows its label even if it is the only one.",
      1, model.rowCount(MaskIndex(model)));
    CPPUNIT_ASSERT_EQUAL(std::string("Only label"), LabelText(model, 0));
    CPPUNIT_ASSERT_MESSAGE("A single label row must not offer a check box.",
      !IsCheckable(model, LabelIndex(model, 0)));
    CPPUNIT_ASSERT(model.IsLabelChecked(onlyLabel));
  }

  /** Sets up a model on the fixture mask with a third label "Label C" and check boxes. */
  mitk::Label::PixelType SetUpModelWithThreeLabels(QmitkImageStatisticsTreeModel& model)
  {
    const auto labelC = AddLabel(m_Mask, "Label C", 0);
    this->AddStatistics(m_Image, m_Mask);

    model.SetDataStorage(m_DataStorage);
    model.SetImageNodes({ m_ImageNode.GetPointer() });
    model.SetMaskNodes({ m_MaskNode.GetPointer() });
    model.SetLabelsCheckable(true);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Precondition failed: unexpected number of label rows.",
      3, model.rowCount(MaskIndex(model)));
    CPPUNIT_ASSERT_MESSAGE("Precondition failed: the first label is not checked.",
      model.IsLabelChecked(m_LabelA));

    return labelC;
  }

  /** Removes the label from the fixture mask and adds recomputed statistics, as the
  data generator would after the removal outdated the former ones. */
  void RemoveLabelAndRecompute(mitk::Label::PixelType labelValue)
  {
    m_Mask->RemoveLabel(labelValue);
    this->AddStatistics(m_Image, m_Mask);
    Settle();
  }

  void CheckedLabelRemoved_FirstRemainingLabelIsChecked()
  {
    QmitkImageStatisticsTreeModel model;
    const auto labelC = this->SetUpModelWithThreeLabels(model);

    this->RemoveLabelAndRecompute(m_LabelA);

    CPPUNIT_ASSERT_EQUAL(2, model.rowCount(MaskIndex(model)));
    CPPUNIT_ASSERT_EQUAL(std::string("Label B"), LabelText(model, 0));
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(Qt::Checked), CheckState(model, LabelIndex(model, 0)));
    CPPUNIT_ASSERT(model.IsLabelChecked(m_LabelB));
    CPPUNIT_ASSERT(!model.IsLabelChecked(labelC));
  }

  void CheckedLabelRemoved_OtherCheckStatesAreKept()
  {
    QmitkImageStatisticsTreeModel model;
    const auto labelC = this->SetUpModelWithThreeLabels(model);
    model.setData(LabelIndex(model, 2), Qt::Checked, Qt::CheckStateRole);

    this->RemoveLabelAndRecompute(m_LabelA);

    CPPUNIT_ASSERT(!model.IsLabelChecked(m_LabelB));
    CPPUNIT_ASSERT(model.IsLabelChecked(labelC));
  }

  void LabelsRecolored_InputDisplayChangedIsEmittedOnce()
  {
    QmitkImageStatisticsTreeModel model;
    this->SetUpModel(model);

    int inputDisplayChangedCount = 0;
    QObject::connect(&model, &QmitkImageStatisticsTreeModel::inputDisplayChanged,
      [&inputDisplayChangedCount]() { ++inputDisplayChangedCount; });

    mitk::Color blue;
    blue.Set(0.0f, 0.0f, 1.0f);
    m_Mask->GetLabel(m_LabelA)->SetColor(blue);
    m_Mask->GetLabel(m_LabelB)->SetColor(blue);
    Settle();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("A batch of label modifications must result in one signal.",
      1, inputDisplayChangedCount);

    // A rebuild for any other reason must not pretend that an input display changed.
    model.SetHistogramNBins(50);
    CPPUNIT_ASSERT_EQUAL(1, inputDisplayChangedCount);
  }

  void Headers_ShowReadableNamesAndToolTips()
  {
    QmitkImageStatisticsTreeModel model;
    this->SetUpModel(model);

    CPPUNIT_ASSERT_EQUAL(std::string("Images / Masks"), FirstColumnHeader(model));

    const auto standardDeviation = ColumnOf(model, QStringLiteral("Std. dev."));
    CPPUNIT_ASSERT_MESSAGE("The standard deviation must not be headed by its key.", standardDeviation > 0);
    CPPUNIT_ASSERT_EQUAL(std::string("Standard deviation"), ToolTip(model, standardDeviation));

    CPPUNIT_ASSERT_EQUAL(std::string("Mean of positive pixels"), ToolTip(model, ColumnOf(model, QStringLiteral("MPP"))));

    CPPUNIT_ASSERT_MESSAGE("The superscript of the volume must be a single character.",
      ColumnOf(model, QStringLiteral("Volume [mm\u00B3]")) > 0);

    const auto custom = ColumnOf(model, QStringLiteral("CustomStat"));
    CPPUNIT_ASSERT_MESSAGE("An unknown statistic must be headed by its key.", custom > 0);
    CPPUNIT_ASSERT_MESSAGE("An unknown statistic must not offer an empty tooltip.",
      model.headerData(custom, Qt::Horizontal, Qt::ToolTipRole).isNull());
  }

  void Headers_AreExportedByTheirKeys()
  {
    QmitkImageStatisticsTreeModel model;
    this->SetUpModel(model);

    CPPUNIT_ASSERT_EQUAL(std::string("Images / Masks"), ExportedHeader(model, 0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("The export has to stay machine readable.",
      mitk::ImageStatisticsConstants::STANDARDDEVIATION(),
      ExportedHeader(model, ColumnOf(model, QStringLiteral("Std. dev."))));

    CPPUNIT_ASSERT_EQUAL(mitk::ImageStatisticsConstants::VOLUME(),
      ExportedHeader(model, ColumnOf(model, QStringLiteral("Volume [mm\u00B3]"))));
  }

  void Values_AreFormattedForReadingAndAvailableUnformatted()
  {
    QmitkImageStatisticsTreeModel model;
    this->SetUpModel(model);

    const auto mean = model.index(0, ColumnOf(model, QStringLiteral("Mean")), MaskIndex(model));

    CPPUNIT_ASSERT_EQUAL(std::string("1.0000"), model.data(mean, Qt::DisplayRole).toString().toStdString());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The raw value is what the clipboard export builds on.",
      static_cast<int>(QMetaType::Double), model.data(mean, Qt::EditRole).typeId());
    CPPUNIT_ASSERT_EQUAL(1.0, model.data(mean, Qt::EditRole).toDouble());

    CPPUNIT_ASSERT_EQUAL(std::string("999"), ValueText(model, QStringLiteral("Voxels")));

    CPPUNIT_ASSERT_EQUAL(std::string("[1, 2, 3]"), ValueText(model, QStringLiteral("Min position")));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The export keeps the technical rendering of a position.",
      std::string("1 2 3"), ExportedValueText(model, QStringLiteral("Min position")));

    CPPUNIT_ASSERT_EQUAL(static_cast<int>(Qt::AlignRight | Qt::AlignVCenter),
      model.data(mean, Qt::TextAlignmentRole).toInt());
  }

  void Values_ShareTheDecimalPlacesOfTheirColumn()
  {
    auto otherMask = mitk::MultiLabelSegmentation::New();
    otherMask->Initialize(CreateTestImage());
    AddLabel(otherMask, "Label C", 0);
    auto otherMaskNode = this->AddMaskNode(otherMask, 999.5);

    QmitkImageStatisticsTreeModel model;
    model.SetDataStorage(m_DataStorage);
    model.SetImageNodes({ m_ImageNode.GetPointer() });
    model.SetMaskNodes({ m_MaskNode.GetPointer(), otherMaskNode.GetPointer() });

    const auto column = ColumnOf(model, QStringLiteral("Mean"));
    const auto imageIndex = model.index(0, 0);
    const auto otherMaskIndex = model.index(1, 0, imageIndex);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Precondition failed: the second mask was not picked up.",
      std::string("Other mask"), Text(model, otherMaskIndex));

    // Two decimal places for both, because the largest value of the column has three digits.
    CPPUNIT_ASSERT_EQUAL(std::string("1.00"),
      Text(model, model.index(0, column, MaskIndex(model))));
    CPPUNIT_ASSERT_EQUAL(std::string("999.50"),
      Text(model, model.index(0, column, otherMaskIndex)));
  }

  void RoundedValue_ToolTipShowsItUnrounded()
  {
    auto mask = mitk::MultiLabelSegmentation::New();
    mask->Initialize(CreateTestImage());
    AddLabel(mask, "Label C", 0);
    auto maskNode = this->AddMaskNode(mask, 1234.56789);

    QmitkImageStatisticsTreeModel model;
    model.SetDataStorage(m_DataStorage);
    model.SetImageNodes({ m_ImageNode.GetPointer() });
    model.SetMaskNodes({ maskNode.GetPointer() });

    const auto mean = model.index(0, ColumnOf(model, QStringLiteral("Mean")), MaskIndex(model));

    CPPUNIT_ASSERT_EQUAL(std::string("1234.57"), Text(model, mean));
    CPPUNIT_ASSERT_EQUAL(std::string("1234.56789"),
      model.data(mean, Qt::ToolTipRole).toString().toStdString());
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkImageStatisticsTreeModel)
