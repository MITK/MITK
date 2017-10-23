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

// semantic relations UI module
#include "QmitkLesionInfoWidget.h"

#include "QmitkCustomVariants.h"

// semantic relations module
#include <mitkNodePredicates.h>
#include <mitkUIDGeneratorBoost.h>

// multi label module
#include <mitkLabelSetImage.h>

// mitk core
#include <mitkProperties.h>

// qt
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QString>

const QBrush QmitkLesionInfoWidget::DEFAULT_BACKGROUND_COLOR = QBrush(Qt::white);
const QBrush QmitkLesionInfoWidget::SELECTED_BACKGROUND_COLOR = QBrush(Qt::green);
const QBrush QmitkLesionInfoWidget::CONNECTED_BACKGROUND_COLOR = QBrush(Qt::darkGreen);

QmitkLesionInfoWidget::QmitkLesionInfoWidget(mitk::DataStorage* dataStorage, QWidget* parent /*= nullptr*/)
  : QWidget(parent)
  , m_DataStorage(dataStorage)
  , m_SemanticRelations(std::make_unique<mitk::SemanticRelations>(dataStorage))
  , m_SimpleDatamanagerWidget(new QmitkSimpleDatamanagerWidget(dataStorage))
  , m_SimpleDatamanagerNodeDialog(new QmitkSelectNodeDialog(this))
{
  Init();
}

QmitkLesionInfoWidget::~QmitkLesionInfoWidget()
{
  m_SemanticRelations->RemoveObserver(this);
}

void QmitkLesionInfoWidget::Init()
{
  // create GUI from the Qt Designer's .ui file
  m_Controls.setupUi(this);

  m_Controls.lesionListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
  m_Controls.segmentationListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
  m_Controls.imageListWidget->setContextMenuPolicy(Qt::CustomContextMenu);

  m_SimpleDatamanagerNodeDialog->setWindowTitle("Select node");
  m_SimpleDatamanagerNodeDialog->SetSelectionWidget(m_SimpleDatamanagerWidget);

  SetUpConnections();

  m_SemanticRelations->AddObserver(this);
}

void QmitkLesionInfoWidget::SetUpConnections()
{
  // connect each list widget with a custom slots
  connect(m_Controls.lesionListWidget, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), SLOT(OnCurrentLesionItemChanged(QListWidgetItem*, QListWidgetItem*)));
  connect(m_Controls.lesionListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), SLOT(OnLesionItemDoubleClicked(QListWidgetItem*)));
  connect(m_Controls.segmentationListWidget, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), SLOT(OnCurrentSegmentationItemChanged(QListWidgetItem*, QListWidgetItem*)));
  connect(m_Controls.segmentationListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), SLOT(OnSegmentationItemDoubleClicked(QListWidgetItem*)));
  connect(m_Controls.imageListWidget, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), SLOT(OnCurrentImageItemChanged(QListWidgetItem*, QListWidgetItem*)));
  connect(m_Controls.imageListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), SLOT(OnImageItemDoubleClicked(QListWidgetItem*)));

  // connect context menu entries
  connect(m_Controls.lesionListWidget, SIGNAL(customContextMenuRequested(const QPoint&)), SLOT(OnLesionListContextMenuRequested(const QPoint&)));
  connect(m_Controls.segmentationListWidget, SIGNAL(customContextMenuRequested(const QPoint&)), SLOT(OnSegmentationListContextMenuRequested(const QPoint&)));
  connect(m_Controls.imageListWidget, SIGNAL(customContextMenuRequested(const QPoint&)), SLOT(OnImageListContextMenuRequested(const QPoint&)));
}

void QmitkLesionInfoWidget::SetCurrentCaseID(const mitk::SemanticTypes::CaseID& caseID)
{
  m_CaseID = caseID;
  Update(m_CaseID);
}

void QmitkLesionInfoWidget::Update(const mitk::SemanticTypes::CaseID& caseID)
{
  if (nullptr == m_SemanticRelations)
  {
    return;
  }

  // if the case ID of updated instance is equal to the currently active caseID
  if (caseID == m_CaseID)
  {
    ResetLesionListWidget();
    ResetSegmentationListWidget();
    ResetImageListWidget();
  }
}

void QmitkLesionInfoWidget::OnCurrentLesionItemChanged(QListWidgetItem* current, QListWidgetItem* /*previous*/)
{
  if (nullptr == m_SemanticRelations
   || nullptr == current
   || nullptr == m_DataStorage)
  {
    return;
  }

  // only the UID is needed to identify a representing lesion
  m_CurrentLesion.UID = current->text().toStdString();
  if (false == m_SemanticRelations->InstanceExists(m_CaseID, m_CurrentLesion))
  {
    // no UID found; cannot create a lesion
    return;
  }

  if (SELECTED_BACKGROUND_COLOR == current->background()
    || CONNECTED_BACKGROUND_COLOR == current->background())
  {
    DarkenBackgroundColors();
  }
  else
  {
    ResetBackgroundColors();
  }

  current->setBackground(SELECTED_BACKGROUND_COLOR);
  try
  {
    // set background color for all corresponding segmentations of the currently selected lesion
    mitk::SemanticRelations::DataNodeVector allSegmentationsOfLesion = m_SemanticRelations->GetAllSegmentationsOfLesion(m_CaseID, m_CurrentLesion);
    for (const auto& segmentation : allSegmentationsOfLesion)
    {
      QList<QListWidgetItem*> items = m_Controls.segmentationListWidget->findItems(QString::fromStdString(segmentation->GetName()), Qt::MatchExactly);
      for (auto& item : items)
      {
        item->setBackground(SELECTED_BACKGROUND_COLOR);
      }

      // set background color for all corresponding images of the current segmentation
      // get parent node of the current segmentation node with the node predicate
      mitk::DataStorage::SetOfObjects::ConstPointer parentNodes = m_DataStorage->GetSources(segmentation, mitk::NodePredicates::GetImagePredicate(), false);
      for (auto it = parentNodes->Begin(); it != parentNodes->End(); ++it)
      {
        QList<QListWidgetItem*> items = m_Controls.imageListWidget->findItems(QString::fromStdString(it->Value()->GetName()), Qt::MatchExactly);
        for (auto& item : items)
        {
          item->setBackground(SELECTED_BACKGROUND_COLOR);
        }
      }
    }
  }
  catch (mitk::Exception& e)
  {
    std::stringstream exceptionMessage; exceptionMessage << e;
    QMessageBox msgBox;
    msgBox.setWindowTitle("No segmentations retrieved");
    msgBox.setText("Could not automatically retrieve all segmentations of the selected lesion.\n"
      "Reason:\n" + QString::fromStdString(exceptionMessage.str()));
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.exec();
    return;
  }
}

void QmitkLesionInfoWidget::OnLesionItemDoubleClicked(QListWidgetItem* clickedItem)
{
  if (nullptr == m_SemanticRelations)
  {
    return;
  }

  if (nullptr == clickedItem)
  {
    // no item clicked; cannot retrieve the current lesion
    return;
  }
}

void QmitkLesionInfoWidget::OnCurrentSegmentationItemChanged(QListWidgetItem* current, QListWidgetItem* /*previous*/)
{
  if (nullptr == m_SemanticRelations
    || nullptr == current
    || nullptr == m_DataStorage)
  {
    return;
  }

  QString segmentationName = current->text();
  m_CurrentSegmentation = m_DataStorage->GetNamedNode(segmentationName.toStdString());
  if (nullptr == m_CurrentSegmentation)
  {
    return;
  }

  if (SELECTED_BACKGROUND_COLOR == current->background()
    || CONNECTED_BACKGROUND_COLOR == current->background())
  {
    DarkenBackgroundColors();
  }
  else
  {
    ResetBackgroundColors();
  }

  current->setBackground(SELECTED_BACKGROUND_COLOR);
  // set background color for all corresponding images of the current segmentation
  // get parent node of the current segmentation node with the node predicate
  mitk::DataStorage::SetOfObjects::ConstPointer parentNodes = m_DataStorage->GetSources(m_CurrentSegmentation, mitk::NodePredicates::GetImagePredicate(), false);
  for (auto it = parentNodes->Begin(); it != parentNodes->End(); ++it)
  {
    QList<QListWidgetItem*> items = m_Controls.imageListWidget->findItems(QString::fromStdString(it->Value()->GetName()), Qt::MatchExactly);
    for (auto& item : items)
    {
      item->setBackground(SELECTED_BACKGROUND_COLOR);
    }
  }

  if (m_SemanticRelations->IsRepresentingALesion(m_CurrentSegmentation))
  {
    try
    {
      // set background color for the represented lesion of the currently selected segmentation
      mitk::SemanticTypes::Lesion representedLesion = m_SemanticRelations->GetRepresentedLesion(m_CurrentSegmentation);
      QList<QListWidgetItem*> items = m_Controls.lesionListWidget->findItems(QString::fromStdString(representedLesion.UID), Qt::MatchExactly);
      for (auto& item : items)
      {
        item->setBackground(SELECTED_BACKGROUND_COLOR);
      }
    }
    catch (mitk::Exception& e)
    {
      std::stringstream exceptionMessage; exceptionMessage << e;
      QMessageBox msgBox;
      msgBox.setWindowTitle("No lesion retrieved");
      msgBox.setText("Could not automatically retrieve the represented lesion of the selected segmentation.\n"
        "Reason:\n" + QString::fromStdString(exceptionMessage.str()));
      msgBox.setIcon(QMessageBox::Warning);
      msgBox.exec();
      return;
    }
  }
}

void QmitkLesionInfoWidget::OnSegmentationItemDoubleClicked(QListWidgetItem* clickedItem)
{
  if (nullptr == clickedItem
   || nullptr == m_DataStorage)
  {
    // no item clicked; cannot retrieve the current segmentation
    return;
  }

  QString segmentationName = clickedItem->text();
  m_CurrentSegmentation = m_DataStorage->GetNamedNode(segmentationName.toStdString());
  if (nullptr == m_CurrentSegmentation)
  {
    return;
  }

  mitk::LabelSetImage* labelSetImage = dynamic_cast<mitk::LabelSetImage*>(m_CurrentSegmentation->GetData());
  if (nullptr == labelSetImage)
  {
    return;
  }

  int activeLayer = labelSetImage->GetActiveLayer();
  mitk::Label* activeLabel = labelSetImage->GetActiveLabel(activeLayer);
  labelSetImage->UpdateCenterOfMass(activeLabel->GetValue(), activeLayer);
  const mitk::Point3D& centerPosition = activeLabel->GetCenterOfMassCoordinates();
  if (centerPosition.GetVnlVector().max_value() > 0.0)
  {
    emit JumpToPosition(centerPosition);
  }
}

void QmitkLesionInfoWidget::OnCurrentImageItemChanged(QListWidgetItem* current, QListWidgetItem* /*previous*/)
{
  if (nullptr == m_SemanticRelations
   || nullptr == current
   || nullptr == m_DataStorage)
  {
    return;
  }

  QString imageName = current->text();
  m_CurrentImage = m_DataStorage->GetNamedNode(imageName.toStdString());
  if (nullptr == m_CurrentImage)
  {
    return;
  }

  if (SELECTED_BACKGROUND_COLOR == current->background()
   || CONNECTED_BACKGROUND_COLOR == current->background())
  {
    DarkenBackgroundColors();
  }
  else
  {
    ResetBackgroundColors();
  }

  current->setBackground(SELECTED_BACKGROUND_COLOR);

  // set background color for all corresponding segmentations of the current image
  // get child nodes of the current image node with the segmentation predicate
  mitk::DataStorage::SetOfObjects::ConstPointer segmentationNodes = m_DataStorage->GetDerivations(m_CurrentImage, mitk::NodePredicates::GetSegmentationPredicate(), false);
  for (auto it = segmentationNodes->Begin(); it != segmentationNodes->End(); ++it)
  {
    QList<QListWidgetItem*> items = m_Controls.segmentationListWidget->findItems(QString::fromStdString(it->Value()->GetName()), Qt::MatchExactly);
    for (auto& item : items)
    {
      item->setBackground(SELECTED_BACKGROUND_COLOR);
    }

    try
    {
      if (m_SemanticRelations->IsRepresentingALesion(it->Value()))
      {
        // set background color for the represented lesion of the current segmentation
        mitk::SemanticTypes::Lesion representedLesion = m_SemanticRelations->GetRepresentedLesion(it->Value());
        QList<QListWidgetItem*> items = m_Controls.lesionListWidget->findItems(QString::fromStdString(representedLesion.UID), Qt::MatchExactly);
        for (auto& item : items)
        {
          item->setBackground(SELECTED_BACKGROUND_COLOR);
        }
      }
    }
    catch (mitk::Exception& e)
    {
      std::stringstream exceptionMessage; exceptionMessage << e;
      QMessageBox msgBox;
      msgBox.setWindowTitle("No lesion retrieved");
      msgBox.setText("Could not automatically retrieve the represented lesion of the selected segmentation.\n"
        "Reason:\n" + QString::fromStdString(exceptionMessage.str()));
      msgBox.setIcon(QMessageBox::Warning);
      msgBox.exec();
      return;
    }
  }
}

void QmitkLesionInfoWidget::OnImageItemDoubleClicked(QListWidgetItem* clickedItem)
{
  if (nullptr == m_SemanticRelations)
  {
    return;
  }

  if (nullptr == clickedItem)
  {
    // no item clicked; cannot retrieve the current image
    return;
  }
}

void QmitkLesionInfoWidget::OnLesionListContextMenuRequested(const QPoint& pos)
{
  QListWidgetItem* currentItem = m_Controls.lesionListWidget->itemAt(pos);
  if (nullptr == currentItem)
  {
    // no item clicked; cannot retrieve the current lesion
    return;
  }

  mitk::SemanticTypes::Lesion selectedLesion;
  selectedLesion.UID = currentItem->text().toStdString();
  if (selectedLesion.UID.empty())
  {
    // no UID found; cannot create a lesion
    return;
  }

  QMenu* menu = new QMenu(m_Controls.lesionListWidget);

  QAction* linkToSegmentation = new QAction("Link to segmentation", this);
  linkToSegmentation->setEnabled(true);
  connect(linkToSegmentation, &QAction::triggered, [this, selectedLesion] { OnLinkToSegmentation(selectedLesion); });
  menu->addAction(linkToSegmentation);

  QAction* setLesionClass = new QAction("Set lesion class", this);
  setLesionClass->setEnabled(true);
  connect(setLesionClass, &QAction::triggered, [this, selectedLesion] { OnSetLesionClass(selectedLesion); });
  menu->addAction(setLesionClass);

  QAction* removeLesion = new QAction("Remove lesion", this);
  removeLesion->setEnabled(true);
  connect(removeLesion, &QAction::triggered, [this, selectedLesion] { OnRemoveLesion(selectedLesion); });
  menu->addAction(removeLesion);

  menu->popup(QCursor::pos());
}

void QmitkLesionInfoWidget::OnSegmentationListContextMenuRequested(const QPoint& pos)
{
  QListWidgetItem* currentItem = m_Controls.segmentationListWidget->itemAt(pos);
  if (nullptr == currentItem)
  {
    // no item clicked; cannot retrieve the current segmentation
    return;
  }

  const mitk::DataNode* selectedSegmentation = m_DataStorage->GetNamedNode(currentItem->text().toStdString());
  if (nullptr == selectedSegmentation)
  {
    return;
  }

  if (false == mitk::NodePredicates::GetSegmentationPredicate()->CheckNode(selectedSegmentation))
  {
    return;
  }

  QMenu* menu = new QMenu(m_Controls.segmentationListWidget);
  QAction* unlinkFromLesion = new QAction("Unlink from lesion", this);
  unlinkFromLesion->setEnabled(true);
  connect(unlinkFromLesion, &QAction::triggered, [this, selectedSegmentation] { OnUnlinkFromLesion(selectedSegmentation); });
  menu->addAction(unlinkFromLesion);

  QAction* removeSegmentation = new QAction("Remove segmentation", this);
  removeSegmentation->setEnabled(true);
  connect(removeSegmentation, &QAction::triggered, [this, selectedSegmentation] { OnRemoveSegmentation(selectedSegmentation); });
  menu->addAction(removeSegmentation);

  menu->popup(QCursor::pos());
}

void QmitkLesionInfoWidget::OnImageListContextMenuRequested(const QPoint& pos)
{
  QListWidgetItem* currentItem = m_Controls.imageListWidget->itemAt(pos);
  if (nullptr == currentItem)
  {
    // no item clicked; cannot retrieve the current image
    return;
  }

  const mitk::DataNode* selectedImage = m_DataStorage->GetNamedNode(currentItem->text().toStdString());
  if (nullptr == selectedImage)
  {
    return;
  }

  if (false == mitk::NodePredicates::GetImagePredicate()->CheckNode(selectedImage))
  {
    return;
  }

  QMenu* menu = new QMenu(m_Controls.imageListWidget);
  QAction* removeImage = new QAction("Remove image", this);
  removeImage->setEnabled(true);
  connect(removeImage, &QAction::triggered, [this, selectedImage] { OnRemoveImage(selectedImage); });
  menu->addAction(removeImage);

  menu->popup(QCursor::pos());
}

void QmitkLesionInfoWidget::OnLinkToSegmentation(const mitk::SemanticTypes::Lesion& selectedLesion)
{
  int dialogReturnValue = m_SimpleDatamanagerNodeDialog->exec();
  if (QDialog::Rejected == dialogReturnValue)
  {
    return;
  }

  mitk::DataNode* selectedDataNode = m_SimpleDatamanagerNodeDialog->GetSelectedDataNode();
  if (nullptr == selectedDataNode)
  {
    QMessageBox msgBox;
    msgBox.setWindowTitle("No valid segmentation node selected.");
    msgBox.setText("In order to link the selected lesion to a segmentation, please specify a valid segmentation node.");
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.exec();
    return;
  }

  if (false == mitk::NodePredicates::GetSegmentationPredicate()->CheckNode(selectedDataNode))
  {
    QMessageBox msgBox;
    msgBox.setWindowTitle("No segmentation selected");
    msgBox.setText("In order to link the selected lesion to a segmentation, please specify a valid segmentation node.");
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.exec();
    return;
  }

  mitk::BaseData* baseData = selectedDataNode->GetData();
  if (nullptr == baseData)
  {
    QMessageBox msgBox;
    msgBox.setWindowTitle("No valid base data.");
    msgBox.setText("In order to link the selected lesion to a segmentation, please specify a valid segmentation node.");
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.exec();
    return;
  }

  try
  {
    m_SemanticRelations->LinkSegmentationToLesion(selectedDataNode, selectedLesion);
  }
  catch (mitk::Exception& e)
  {
    std::stringstream exceptionMessage; exceptionMessage << e;
    QMessageBox msgBox;
    msgBox.setWindowTitle("Could not link the selected lesion.");
    msgBox.setText("The program wasn't able to correctly link the selected lesion with the selected segmentation.\n"
      "Reason:\n" + QString::fromStdString(exceptionMessage.str()));
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.exec();
  }
}

void QmitkLesionInfoWidget::OnRemoveLesion(const mitk::SemanticTypes::Lesion& selectedLesion)
{
  if (m_CaseID.empty())
  {
    QMessageBox msgBox;
    msgBox.setWindowTitle("No case ID set.");
    msgBox.setText("In order to remove a lesion, please specify the current case / patient.");
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.exec();
    return;
  }

  try
  {
    m_SemanticRelations->RemoveLesion(m_CaseID, selectedLesion);
  }
  catch (mitk::Exception& e)
  {
    std::stringstream exceptionMessage; exceptionMessage << e;
    QMessageBox msgBox;
    msgBox.setWindowTitle("Could not remove the selected lesion.");
    msgBox.setText("The program wasn't able to correctly remove the selected lesion from the semantic relations model.\n"
                   "Reason:\n" + QString::fromStdString(exceptionMessage.str()));
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.exec();
  }
}

void QmitkLesionInfoWidget::OnSetLesionClass(const mitk::SemanticTypes::Lesion& selectedLesion)
{
  // get lesion class from user input and generate UID for lesion class
  //m_SemanticRelations->OverwriteLesion()
}

void QmitkLesionInfoWidget::OnUnlinkFromLesion(const mitk::DataNode* selectedSegmentation)
{
  try
  {
    m_SemanticRelations->UnlinkSegmentationFromLesion(selectedSegmentation);
  }
  catch (mitk::Exception& e)
  {
    std::stringstream exceptionMessage; exceptionMessage << e;
    QMessageBox msgBox;
    msgBox.setWindowTitle("Could not unlink a lesion from the selected segmentation.");
    msgBox.setText("The program wasn't able to correctly unlink the lesion from the selected segmentation.\n"
      "Reason:\n" + QString::fromStdString(exceptionMessage.str()));
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.exec();
  }
}

void QmitkLesionInfoWidget::OnRemoveSegmentation(const mitk::DataNode* selectedSegmentation)
{
  try
  {
    m_SemanticRelations->RemoveSegmentation(selectedSegmentation);
  }
  catch (mitk::Exception& e)
  {
    std::stringstream exceptionMessage; exceptionMessage << e;
    QMessageBox msgBox;
    msgBox.setWindowTitle("Could not remove the selected segmentation.");
    msgBox.setText("The program wasn't able to correctly remove the selected segmentation from the semantic relations model.\n"
      "Reason:\n" + QString::fromStdString(exceptionMessage.str()));
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.exec();
  }
}

void QmitkLesionInfoWidget::OnRemoveImage(const mitk::DataNode* selectedImage)
{
  try
  {
    m_SemanticRelations->RemoveImage(selectedImage);
    emit ImageRemoved(selectedImage);
  }
  catch (mitk::Exception& e)
  {
    std::stringstream exceptionMessage; exceptionMessage << e;
    QMessageBox msgBox;
    msgBox.setWindowTitle("Could not remove the selected image.");
    msgBox.setText("The program wasn't able to correctly remove the selected image from the semantic relations model.\n"
      "Reason:\n" + QString::fromStdString(exceptionMessage.str()));
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.exec();
  }
}

void QmitkLesionInfoWidget::ResetLesionListWidget()
{
  m_Controls.lesionListWidget->clear();
  m_CurrentLesion.UID = "";

  // create lesion list widget entries with the current lesions
  mitk::SemanticRelations::LesionVector allLesionsOfCase = m_SemanticRelations->GetAllLesionsOfCase(m_CaseID);
  if (allLesionsOfCase.empty())
  {
    m_Controls.lesionListWidget->addItem("No lesions found");
  }
  for (const auto& lesion : allLesionsOfCase)
  {
    m_Controls.lesionListWidget->addItem(QString::fromStdString(lesion.UID));
  }
}

void QmitkLesionInfoWidget::ResetSegmentationListWidget()
{
  m_Controls.segmentationListWidget->clear();
  m_CurrentSegmentation = nullptr;

  try
  {
    // create segmentation list widget entries with the current segmentations
    mitk::SemanticRelations::DataNodeVector allSegmentationsOfCase = m_SemanticRelations->GetAllSegmentationsOfCase(m_CaseID);
    if (allSegmentationsOfCase.empty())
    {
      m_Controls.segmentationListWidget->addItem("No segmentations found");
    }
    for (const auto& segmentation : allSegmentationsOfCase)
    {
      m_Controls.segmentationListWidget->addItem(QString::fromStdString(segmentation->GetName()));
    }
  }
  catch (mitk::Exception& e)
  {
    std::stringstream exceptionMessage; exceptionMessage << e;
    QMessageBox msgBox;
    msgBox.setWindowTitle("No segmentations retrieved");
    msgBox.setText("Could not automatically retrieve all segmentations of the current case.\n"
      "Reason:\n" + QString::fromStdString(exceptionMessage.str()));
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.exec();
    return;
  }
}

void QmitkLesionInfoWidget::ResetImageListWidget()
{
  m_Controls.imageListWidget->clear();
  m_CurrentImage = nullptr;

  try
  {
    // create image list widget entries with the current images
    mitk::SemanticRelations::DataNodeVector allImagesOfCase = m_SemanticRelations->GetAllImagesOfCase(m_CaseID);
    if (allImagesOfCase.empty())
    {
      m_Controls.imageListWidget->addItem("No images found");
    }
    for (const auto& image : allImagesOfCase)
    {
      m_Controls.imageListWidget->addItem(QString::fromStdString(image->GetName()));
    }
  }
  catch (mitk::Exception& e)
  {
    std::stringstream exceptionMessage; exceptionMessage << e;
    QMessageBox msgBox;
    msgBox.setWindowTitle("No images retrieved");
    msgBox.setText("Could not automatically retrieve all images of the current case.\n"
      "Reason:\n" + QString::fromStdString(exceptionMessage.str()));
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.exec();
    return;
  }
}

void QmitkLesionInfoWidget::ResetBackgroundColors()
{
  // reset all lesion list widget items to original background color
  for (int i = 0; i < m_Controls.lesionListWidget->count(); ++i)
  {
    QListWidgetItem* item = m_Controls.lesionListWidget->item(i);
    item->setBackground(DEFAULT_BACKGROUND_COLOR);
  }

  // reset all segmentation list widget items to original background color
  for (int i = 0; i < m_Controls.segmentationListWidget->count(); ++i)
  {
    QListWidgetItem* item = m_Controls.segmentationListWidget->item(i);
    item->setBackground(DEFAULT_BACKGROUND_COLOR);
  }

  // reset all image list widget items to original background color
  for (int i = 0; i < m_Controls.imageListWidget->count(); ++i)
  {
    QListWidgetItem* item = m_Controls.imageListWidget->item(i);
    item->setBackground(DEFAULT_BACKGROUND_COLOR);
  }
}

void QmitkLesionInfoWidget::DarkenBackgroundColors()
{
  // reset all lesion list widget items to original background color
  for (int i = 0; i < m_Controls.lesionListWidget->count(); ++i)
  {
    QListWidgetItem* item = m_Controls.lesionListWidget->item(i);
    if (DEFAULT_BACKGROUND_COLOR != item->background())
    {
      item->setBackground(CONNECTED_BACKGROUND_COLOR);
    }
  }

  // reset all segmentation list widget items to original background color
  for (int i = 0; i < m_Controls.segmentationListWidget->count(); ++i)
  {
    QListWidgetItem* item = m_Controls.segmentationListWidget->item(i);
    if (DEFAULT_BACKGROUND_COLOR != item->background())
    {
      item->setBackground(CONNECTED_BACKGROUND_COLOR);
    }
  }

  // reset all image list widget items to original background color
  for (int i = 0; i < m_Controls.imageListWidget->count(); ++i)
  {
    QListWidgetItem* item = m_Controls.imageListWidget->item(i);
    if (DEFAULT_BACKGROUND_COLOR != item->background())
    {
      item->setBackground(CONNECTED_BACKGROUND_COLOR);
    }
  }
}
