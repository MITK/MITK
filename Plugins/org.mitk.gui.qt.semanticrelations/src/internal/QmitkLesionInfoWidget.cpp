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

// semantic relations plugin
#include "QmitkLesionInfoWidget.h"
#include "QmitkSemanticRelationsNodeSelectionDialog.h"

// semantic relations UI module
#include <QmitkLesionTextDialog.h>

// semantic relations module
#include <mitkLesionManager.h>
#include <mitkNodePredicates.h>
#include <mitkSemanticRelationException.h>
#include <mitkUIDGeneratorBoost.h>

#include "QmitkCustomVariants.h"

// multi label module
#include <mitkLabelSetImage.h>

// mitk core
#include <mitkProperties.h>

// qt
#include <QCompleter>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QString>

const QBrush QmitkLesionInfoWidget::DEFAULT_BACKGROUND_COLOR = QBrush(Qt::transparent);
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
  // connect buttons to modify semantic relations
  connect(m_Controls.addLesionPushButton, SIGNAL(clicked()), SLOT(OnAddLesionButtonClicked()));
  connect(m_Controls.addSegmentationPushButton, SIGNAL(clicked()), SLOT(OnAddSegmentationButtonClicked()));
  connect(m_Controls.addImagePushButton, SIGNAL(clicked()), SLOT(OnAddImageButtonClicked()));

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

//////////////////////////////////////////////////////////////////////////
// Implementation of the QT_SLOTS
//////////////////////////////////////////////////////////////////////////
void QmitkLesionInfoWidget::OnAddLesionButtonClicked()
{
  if (m_CaseID.empty())
  {
    QMessageBox msgBox;
    msgBox.setWindowTitle("No case ID set.");
    msgBox.setText("In order to add a lesion, please specify the current case / patient.");
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.exec();
    return;
  }

  mitk::SemanticTypes::Lesion newLesion = mitk::GenerateNewLesion();
  try
  {
    m_SemanticRelations->AddLesion(m_CaseID, newLesion);
  }
  catch (mitk::SemanticRelationException& e)
  {
    MITK_INFO << "Could not add a new lesion. " << e;
  }
}

void QmitkLesionInfoWidget::OnAddSegmentationButtonClicked()
{
  if (m_CaseID.empty())
  {
    QMessageBox msgBox;
    msgBox.setWindowTitle("No case ID set.");
    msgBox.setText("In order to add a segmentation, please specify the current case / patient.");
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.exec();
    return;
  }

  QmitkSemanticRelationsNodeSelectionDialog* dialog = new QmitkSemanticRelationsNodeSelectionDialog(this, "Select segmentations to add to the semantic relations storage", "");
  dialog->SetDataStorage(m_DataStorage);
  dialog->setWindowTitle("Select segmentation node");
  dialog->SetNodePredicate(mitk::NodePredicates::GetSegmentationPredicate());
  dialog->SetSelectOnlyVisibleNodes(true);
  dialog->SetSelectionMode(QAbstractItemView::MultiSelection);
  dialog->SetCaseID(m_CaseID);

  if (QDialog::Accepted == dialog->exec())
  {
    auto nodes = dialog->GetSelectedNodes();
    for (mitk::DataNode* dataNode : nodes)
    {
      if (nullptr == dataNode)
      {
        continue;
      }

      mitk::BaseData* baseData = dataNode->GetData();
      if (nullptr == baseData)
      {
        continue;
      }

      // continue with valid segmentation data
      // get parent node of the current segmentation node with the node predicate
      mitk::DataStorage::SetOfObjects::ConstPointer parentNodes = m_DataStorage->GetSources(dataNode, mitk::NodePredicates::GetImagePredicate(), false);

      // check for already existing, identifying base properties
      mitk::BaseProperty* caseIDProperty = baseData->GetProperty("DICOM.0010.0010");
      mitk::BaseProperty* nodeIDProperty = baseData->GetProperty("DICOM.0020.000E");
      if (nullptr == caseIDProperty || nullptr == nodeIDProperty)
      {
        MITK_INFO << "No DICOM tags for case and node identification found. Transferring DICOM tags from the parent node to the selected segmentation node.";

        mitk::SemanticTypes::CaseID caseID = mitk::GetCaseIDFromDataNode(parentNodes->front());
        mitk::SemanticTypes::ID nodeID = mitk::GetIDFromDataNode(parentNodes->front());
        // transfer DICOM tags to the segmentation node
        mitk::StringProperty::Pointer caseIDTag = mitk::StringProperty::New(caseID);
        baseData->SetProperty("DICOM.0010.0010", caseIDTag); // DICOM tag is "PatientName"

                                                             // add UID to distinguish between different segmentations of the same parent node
        mitk::StringProperty::Pointer nodeIDTag = mitk::StringProperty::New(nodeID + mitk::UIDGeneratorBoost::GenerateUID());
        baseData->SetProperty("DICOM.0020.000E", nodeIDTag); // DICOM tag is "SeriesInstanceUID"
      }

      try
      {
        m_SemanticRelations->AddSegmentation(dataNode, parentNodes->front());
      }
      catch (mitk::Exception& e)
      {
        std::stringstream exceptionMessage; exceptionMessage << e;
        QMessageBox msgBox;
        msgBox.setWindowTitle("Could not add the selected segmentation.");
        msgBox.setText("The program wasn't able to correctly add the selected segmentation.\n"
          "Reason:\n" + QString::fromStdString(exceptionMessage.str()));
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
      }
    }
  }
}

void QmitkLesionInfoWidget::OnAddImageButtonClicked()
{
  QmitkSemanticRelationsNodeSelectionDialog* dialog = new QmitkSemanticRelationsNodeSelectionDialog(this, "Select images to add to the semantic relations storage", "");
  dialog->SetDataStorage(m_DataStorage);
  dialog->setWindowTitle("Select image node");
  dialog->SetNodePredicate(mitk::NodePredicates::GetImagePredicate());
  dialog->SetSelectOnlyVisibleNodes(true);
  dialog->SetSelectionMode(QAbstractItemView::MultiSelection);
  dialog->SetCaseID(m_CaseID);

  if (QDialog::Accepted == dialog->exec())
  {
    auto nodes = dialog->GetSelectedNodes();
    for (mitk::DataNode* dataNode : nodes)
    {
      if (nullptr != dataNode)
      {
        try
        {
          // add the image to the semantic relations storage
          m_SemanticRelations->AddImage(dataNode);
          mitk::SemanticTypes::CaseID caseID = mitk::GetCaseIDFromDataNode(dataNode);
          emit ImageAdded(caseID);
        }
        catch (mitk::Exception& e)
        {
          std::stringstream exceptionMessage; exceptionMessage << e;
          QMessageBox msgBox;
          msgBox.setWindowTitle("Could not add the selected image.");
          msgBox.setText("The program wasn't able to correctly add the selected images.\n"
            "Reason:\n" + QString::fromStdString(exceptionMessage.str()));
          msgBox.setIcon(QMessageBox::Warning);
          msgBox.exec();
        }
      }
    }
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
  m_CurrentLesion.UID = current->data(Qt::UserRole).toString().toStdString();
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
  if (nullptr == clickedItem
   || nullptr == m_SemanticRelations)
  {
    return;
  }

  // only the UID is needed to identify a representing lesion
  m_CurrentLesion.UID = clickedItem->data(Qt::UserRole).toString().toStdString();
  m_CurrentLesion.name = clickedItem->text().toStdString();
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
      for (int i = 0; i < m_Controls.lesionListWidget->count(); ++i)
      {
        QListWidgetItem* item = m_Controls.lesionListWidget->item(i);
        std::string currentLesionUID = item->data(Qt::UserRole).toString().toStdString();
        if (currentLesionUID == representedLesion.UID)
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

void QmitkLesionInfoWidget::OnSegmentationItemDoubleClicked(QListWidgetItem* clickedItem)
{
  if (nullptr == clickedItem
   || nullptr == m_DataStorage)
  {
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
        // set background color for the represented lesion of the currently selected segmentation
        mitk::SemanticTypes::Lesion representedLesion = m_SemanticRelations->GetRepresentedLesion(it->Value());
        for (int i = 0; i < m_Controls.lesionListWidget->count(); ++i)
        {
          QListWidgetItem* item = m_Controls.lesionListWidget->item(i);
          std::string currentLesionUID = item->data(Qt::UserRole).toString().toStdString();
          if (currentLesionUID == representedLesion.UID)
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
  if (nullptr == clickedItem
    || nullptr == m_SemanticRelations)
  {
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

  mitk::SemanticTypes::ID selectedLesionUID;
  selectedLesionUID = currentItem->data(Qt::UserRole).toString().toStdString();
  if (selectedLesionUID.empty())
  {
    // no UID found; cannot create a lesion
    return;
  }

  QMenu* menu = new QMenu(m_Controls.lesionListWidget);

  QAction* linkToSegmentation = new QAction("Link to segmentation", this);
  linkToSegmentation->setEnabled(true);
  connect(linkToSegmentation, &QAction::triggered, [this, selectedLesionUID] { OnLinkToSegmentation(selectedLesionUID); });
  menu->addAction(linkToSegmentation);

  QAction* setLesionName = new QAction("Set lesion name", this);
  setLesionName->setEnabled(true);
  connect(setLesionName, &QAction::triggered, [this, selectedLesionUID] { OnSetLesionName(selectedLesionUID); });
  menu->addAction(setLesionName);

  QAction* setLesionClass = new QAction("Set lesion class", this);
  setLesionClass->setEnabled(true);
  connect(setLesionClass, &QAction::triggered, [this, selectedLesionUID] { OnSetLesionClass(selectedLesionUID); });
  menu->addAction(setLesionClass);

  QAction* removeLesion = new QAction("Remove lesion", this);
  removeLesion->setEnabled(true);
  connect(removeLesion, &QAction::triggered, [this, selectedLesionUID] { OnRemoveLesion(selectedLesionUID); });
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

void QmitkLesionInfoWidget::OnLinkToSegmentation(const mitk::SemanticTypes::ID& selectedLesionUID)
{
  // retrieve the full lesion with its lesion class using the given lesion UID
  mitk::SemanticTypes::Lesion selectedLesion = mitk::GetLesionByUID(selectedLesionUID, m_SemanticRelations->GetAllLesionsOfCase(m_CaseID));
  if (selectedLesion.UID.empty())
  {
    // could not find lesion information for the selected lesion
    return;
  }

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

void QmitkLesionInfoWidget::OnSetLesionName(const mitk::SemanticTypes::ID& selectedLesionUID)
{
  // retrieve the full lesion with its lesion class using the given lesion UID
  mitk::SemanticTypes::Lesion selectedLesion = mitk::GetLesionByUID(selectedLesionUID, m_SemanticRelations->GetAllLesionsOfCase(m_CaseID));
  if (selectedLesion.UID.empty())
  {
    // could not find lesion information for the selected lesion
    return;
  }

  // use the lesion information to set the input text for the dialog
  QmitkLesionTextDialog* inputDialog = new QmitkLesionTextDialog(this);
  inputDialog->setWindowTitle("Set lesion name");
  inputDialog->SetLineEditText(selectedLesion.name);

  int dialogReturnValue = inputDialog->exec();
  if (QDialog::Rejected == dialogReturnValue)
  {
    return;
  }

  std::string newLesionName = inputDialog->GetLineEditText().toStdString();

  selectedLesion.name = newLesionName;
  m_SemanticRelations->OverwriteLesion(m_CaseID, selectedLesion);
}

void QmitkLesionInfoWidget::OnSetLesionClass(const mitk::SemanticTypes::ID& selectedLesionUID)
{
  // retrieve the full lesion with its lesion class using the given lesion UID
  mitk::SemanticTypes::Lesion selectedLesion = mitk::GetLesionByUID(selectedLesionUID, m_SemanticRelations->GetAllLesionsOfCase(m_CaseID));
  if (selectedLesion.UID.empty())
  {
    // could not find lesion information for the selected lesion
    return;
  }

  // use the lesion information to set the input text for the dialog
  QmitkLesionTextDialog* inputDialog = new QmitkLesionTextDialog(this);
  inputDialog->setWindowTitle("Set lesion class");
  inputDialog->SetLineEditText(selectedLesion.lesionClass.classType);

  // prepare the completer for the dialogs input text field
  mitk::LesionClassVector allLesionClasses = m_SemanticRelations->GetAllLesionClassesOfCase(m_CaseID);

  QStringList wordList;
  for (const auto& lesionClass : allLesionClasses)
  {
    wordList << QString::fromStdString(lesionClass.classType);
  }
  QCompleter* completer = new QCompleter(wordList, this);
  completer->setCaseSensitivity(Qt::CaseInsensitive);
  inputDialog->GetLineEdit()->setCompleter(completer);

  int dialogReturnValue = inputDialog->exec();
  if (QDialog::Rejected == dialogReturnValue)
  {
    return;
  }

  // retrieve the new input lesion class type and check for an already existing lesion class types
  std::string newLesionClassType = inputDialog->GetLineEditText().toStdString();
  mitk::SemanticTypes::LesionClass existingLesionClass = mitk::FindExistingLesionClass(newLesionClassType, allLesionClasses);
  if (existingLesionClass.UID.empty())
  {
    // could not find lesion class information for the new lesion class type
    // create a new lesion class for the selected lesion
    existingLesionClass = mitk::GenerateNewLesionClass(newLesionClassType);
  }

  selectedLesion.lesionClass = existingLesionClass;
  m_SemanticRelations->OverwriteLesion(m_CaseID, selectedLesion);
}

void QmitkLesionInfoWidget::OnRemoveLesion(const mitk::SemanticTypes::ID& selectedLesionUID)
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

  // retrieve the full lesion with its lesion class using the given lesion UID
  mitk::SemanticTypes::Lesion selectedLesion = mitk::GetLesionByUID(selectedLesionUID, m_SemanticRelations->GetAllLesionsOfCase(m_CaseID));
  if (selectedLesion.UID.empty())
  {
    // could not find lesion information for the selected lesion
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
  m_CurrentLesion.name = "";

  // create lesion list widget entries with the current lesions
  mitk::SemanticRelations::LesionVector allLesionsOfCase = m_SemanticRelations->GetAllLesionsOfCase(m_CaseID);
  if (allLesionsOfCase.empty())
  {
    m_Controls.lesionListWidget->addItem("No lesions found");
  }
  for (const auto& lesion : allLesionsOfCase)
  {
    // store the UID as 'UserRole' data in the widget item
    QListWidgetItem* lesionItem = new QListWidgetItem;
    lesionItem->setData(Qt::UserRole, QString::fromStdString(lesion.UID));

    // use the lesion UID for the item text, if the lesion name is non-existent
    if (lesion.name.empty())
    {
      lesionItem->setText(QString::fromStdString(lesion.UID));
    }
    else
    {
      lesionItem->setText(QString::fromStdString(lesion.name));
    }

    m_Controls.lesionListWidget->addItem(lesionItem);
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
