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
  Initialize();
}

QmitkLesionInfoWidget::~QmitkLesionInfoWidget()
{
  // nothing here
}

void QmitkLesionInfoWidget::Initialize()
{
  m_Controls.setupUi(this);

  m_Controls.lesionTreeView->setAlternatingRowColors(true);
  m_Controls.lesionTreeView->setSelectionMode(QAbstractItemView::SingleSelection);
  m_Controls.lesionTreeView->setSelectionBehavior(QAbstractItemView::SelectItems);
  m_Controls.lesionTreeView->setContextMenuPolicy(Qt::CustomContextMenu);

  m_StorageModel = new QmitkLesionTreeModel(this);
  if (m_DataStorage.IsExpired())
  {
    return;
  }

  auto dataStorage = m_DataStorage.Lock();
  m_StorageModel->SetDataStorage(dataStorage);
  m_Controls.lesionTreeView->setModel(m_StorageModel);

  SetUpConnections();
}

void QmitkLesionInfoWidget::SetUpConnections()
{
  connect(m_StorageModel, &QmitkLesionTreeModel::ModelUpdated, this, &QmitkLesionInfoWidget::OnModelUpdated);

  // connect buttons to modify semantic relations
  connect(m_Controls.addLesionPushButton, &QPushButton::clicked, this, &QmitkLesionInfoWidget::OnAddLesionButtonClicked);

  // connect each list widget with a custom slots
  connect(m_Controls.lesionTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QmitkLesionInfoWidget::OnSelectionChanged);

  // connect context menu entries
  connect(m_Controls.lesionTreeView, &QTreeView::customContextMenuRequested, this, &QmitkLesionInfoWidget::OnLesionListContextMenuRequested);
}

void QmitkLesionInfoWidget::SetCaseID(const mitk::SemanticTypes::CaseID& caseID)
{
  m_CaseID = caseID;
  m_StorageModel->SetCaseID(caseID);
}

//////////////////////////////////////////////////////////////////////////
// Implementation of the QT_SLOTS
//////////////////////////////////////////////////////////////////////////
void QmitkLesionInfoWidget::OnModelUpdated()
{
  m_Controls.lesionTreeView->expandAll();
  int columns = m_Controls.lesionTreeView->model()->columnCount();
  for (int i = 0; i < columns; ++i)
  {
    m_Controls.lesionTreeView->resizeColumnToContents(i);
  }
}

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

void QmitkLesionInfoWidget::OnSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
  if (nullptr == m_SemanticRelations)
  {
    return;
  }

  for (const auto& index : selected.indexes())
  {
    // only the UID is needed to identify a representing lesion
    QVariant data = m_StorageModel->data(index, Qt::UserRole);
    if (data.canConvert<QmitkLesionTreeItem*>())
    {
      m_CurrentLesion = data.value<QmitkLesionTreeItem*>()->GetData().GetLesion();
    }
    else
    {
      return;
    }

    if (false == m_SemanticRelations->InstanceExists(m_CaseID, m_CurrentLesion))
    {
      // no UID found; cannot create a lesion
      continue;
    }

    emit LesionChanged(m_CurrentLesion);
  }
}

void QmitkLesionInfoWidget::OnLesionListContextMenuRequested(const QPoint& pos)
{
  QModelIndex index = m_Controls.lesionTreeView->indexAt(pos);
  if (!index.isValid())
  {
    // no item clicked; cannot retrieve the current lesion
    return;
  }

  QVariant data = m_StorageModel->data(index, Qt::UserRole);
  mitk::SemanticTypes::Lesion selectedLesion;
  if (data.canConvert<QmitkLesionTreeItem*>())
  {
    selectedLesion = data.value<QmitkLesionTreeItem*>()->GetData().GetLesion();
  }
  else
  {
    return;
  }

  QMenu* menu = new QMenu(m_Controls.lesionTreeView);

  QAction* linkToSegmentation = new QAction("Link to segmentation", this);
  linkToSegmentation->setEnabled(true);
  connect(linkToSegmentation, &QAction::triggered, [this, selectedLesion] { OnLinkToSegmentation(selectedLesion); });
  menu->addAction(linkToSegmentation);

  QAction* setLesionName = new QAction("Set lesion name", this);
  setLesionName->setEnabled(true);
  connect(setLesionName, &QAction::triggered, [this, selectedLesion] { OnSetLesionName(selectedLesion); });
  menu->addAction(setLesionName);

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

void QmitkLesionInfoWidget::OnLinkToSegmentation(mitk::SemanticTypes::Lesion selectedLesion)
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

  if (m_DataStorage.IsExpired())
  {
    return;
  }

  auto dataStorage = m_DataStorage.Lock();

  QmitkSemanticRelationsNodeSelectionDialog* dialog = new QmitkSemanticRelationsNodeSelectionDialog(this, "Select segmentation to link to the selected lesion.", "");
  dialog->SetDataStorage(dataStorage);
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

void QmitkLesionInfoWidget::OnSetLesionName(mitk::SemanticTypes::Lesion selectedLesion)
{
  if (m_CaseID.empty())
  {
    QMessageBox msgBox;
    msgBox.setWindowTitle("No case ID set.");
    msgBox.setText("In order to set a lesion name, please specify the current case / patient.");
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.exec();
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

void QmitkLesionInfoWidget::OnSetLesionClass(mitk::SemanticTypes::Lesion selectedLesion)
{
  if (m_CaseID.empty())
  {
    QMessageBox msgBox;
    msgBox.setWindowTitle("No case ID set.");
    msgBox.setText("In order to set a lesion class, please specify the current case / patient.");
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.exec();
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

void QmitkLesionInfoWidget::OnRemoveLesion(mitk::SemanticTypes::Lesion selectedLesion)
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
