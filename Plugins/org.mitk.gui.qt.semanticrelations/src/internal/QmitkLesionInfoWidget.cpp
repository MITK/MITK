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
{
  Init();
}

QmitkLesionInfoWidget::~QmitkLesionInfoWidget()
{
  if (nullptr != m_SemanticRelations)
  {
    m_SemanticRelations->RemoveObserver(this);
  }
}

void QmitkLesionInfoWidget::Init()
{
  // create GUI from the Qt Designer's .ui file
  m_Controls.setupUi(this);

  m_Controls.lesionListWidget->setContextMenuPolicy(Qt::CustomContextMenu);

  SetUpConnections();

  m_SemanticRelations->AddObserver(this);
}

void QmitkLesionInfoWidget::SetUpConnections()
{
  // connect buttons to modify semantic relations
  connect(m_Controls.addLesionPushButton, &QPushButton::clicked, this, &QmitkLesionInfoWidget::OnAddLesionButtonClicked);

  // connect each list widget with a custom slots
  connect(m_Controls.lesionListWidget, &QListWidget::currentItemChanged, this, &QmitkLesionInfoWidget::OnCurrentLesionItemChanged);
  connect(m_Controls.lesionListWidget, &QListWidget::itemDoubleClicked, this, &QmitkLesionInfoWidget::OnLesionItemDoubleClicked);

  // connect context menu entries
  connect(m_Controls.lesionListWidget, &QListWidget::customContextMenuRequested, this, &QmitkLesionInfoWidget::OnLesionListContextMenuRequested);
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

void QmitkLesionInfoWidget::OnCurrentLesionItemChanged(QListWidgetItem* current, QListWidgetItem* /*previous*/)
{
  if (nullptr == current
   || nullptr == m_SemanticRelations
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

  emit LesionChanged(m_CurrentLesion);
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

void QmitkLesionInfoWidget::OnLinkToSegmentation(const mitk::SemanticTypes::ID& selectedLesionUID)
{
  if (m_CaseID.empty())
  {
    QMessageBox msgBox;
    msgBox.setWindowTitle("No case ID set.");
    msgBox.setText("In order to link a lesion to a segmentation, please specify the current case / patient.");
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

  QmitkSemanticRelationsNodeSelectionDialog* dialog = new QmitkSemanticRelationsNodeSelectionDialog(this, "Select segmentation to link to the selected lesion.", "");
  dialog->SetDataStorage(m_DataStorage);
  dialog->setWindowTitle("Select segmentation node");
  dialog->SetNodePredicate(mitk::NodePredicates::GetSegmentationPredicate());
  dialog->SetSelectOnlyVisibleNodes(true);
  dialog->SetSelectionMode(QAbstractItemView::SingleSelection);
  dialog->SetCaseID(m_CaseID);

  int dialogReturnValue = dialog->exec();
  if (QDialog::Rejected == dialogReturnValue)
  {
    return;
  }

  auto nodes = dialog->GetSelectedNodes();
  mitk::DataNode::Pointer selectedDataNode = nullptr;
  if (!nodes.isEmpty())
  {
    // only single selection allowed
    selectedDataNode = nodes.front();
  }

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
  catch (const mitk::SemanticRelationException& e)
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
  catch (const mitk::SemanticRelationException& e)
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

void QmitkLesionInfoWidget::ResetBackgroundColors()
{
  // reset all lesion list widget items to original background color
  for (int i = 0; i < m_Controls.lesionListWidget->count(); ++i)
  {
    QListWidgetItem* item = m_Controls.lesionListWidget->item(i);
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
}
