/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkMultiLabelInspector.h>

// mitk
#include <mitkRenderingManager.h>
#include <mitkLabelSetImageHelper.h>
#include <mitkDICOMSegmentationPropertyHelper.h>

// Qmitk
#include <QmitkMultiLabelTreeModel.h>
#include <QmitkLabelColorItemDelegate.h>
#include <QmitkLabelToggleItemDelegate.h>
#include <QmitkStyleManager.h>

// Qt
#include <QMenu>
#include <QLabel>
#include <QWidgetAction>
#include <QMessageBox>

#include <ui_QmitkMultiLabelInspectorControls.h>

QmitkMultiLabelInspector::QmitkMultiLabelInspector(QWidget* parent/* = nullptr*/)
  : QWidget(parent), m_Controls(new Ui::QmitkMultiLabelInspector)
{
  m_Controls->setupUi(this);

  m_Model = new QmitkMultiLabelTreeModel(this);

  m_Controls->view->setModel(m_Model);

  m_ColorItemDelegate = new QmitkLabelColorItemDelegate(this);

  auto visibleIcon = QmitkStyleManager::ThemeIcon(QLatin1String(":/Qmitk/visible.svg"));
  auto invisibleIcon = QmitkStyleManager::ThemeIcon(QLatin1String(":/Qmitk/invisible.svg"));
  m_VisibilityItemDelegate = new QmitkLabelToggleItemDelegate(visibleIcon, invisibleIcon, this);

  auto lockIcon = QmitkStyleManager::ThemeIcon(QLatin1String(":/Qmitk/lock.svg"));
  auto unlockIcon = QmitkStyleManager::ThemeIcon(QLatin1String(":/Qmitk/unlock.svg"));
  m_LockItemDelegate = new QmitkLabelToggleItemDelegate(lockIcon, unlockIcon, this);

  auto* view = this->m_Controls->view;
  view->setItemDelegateForColumn(1, m_LockItemDelegate);
  view->setItemDelegateForColumn(2, m_ColorItemDelegate);
  view->setItemDelegateForColumn(3, m_VisibilityItemDelegate);

  auto* header = view->header();
  header->setSectionResizeMode(0,QHeaderView::Stretch);
  header->setSectionResizeMode(1, QHeaderView::ResizeToContents);
  header->setSectionResizeMode(2, QHeaderView::ResizeToContents);
  header->setSectionResizeMode(3, QHeaderView::ResizeToContents);
  view->setContextMenuPolicy(Qt::CustomContextMenu);

  connect(m_Model, &QAbstractItemModel::modelReset, this, &QmitkMultiLabelInspector::OnModelReset);
  connect(view->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), SLOT(OnChangeModelSelection(const QItemSelection&, const QItemSelection&)));
  connect(view, &QAbstractItemView::customContextMenuRequested, this, &QmitkMultiLabelInspector::OnContextMenuRequested);
  connect(view, &QAbstractItemView::doubleClicked, this, &QmitkMultiLabelInspector::OnItemDoubleClicked);
}

QmitkMultiLabelInspector::~QmitkMultiLabelInspector()
{
  delete m_Controls;
}

void QmitkMultiLabelInspector::Initialize()
{
  m_LastValidSelectedLabels = {};
  m_ModelManipulationOngoing = false;
  m_Model->SetSegmentation(m_Segmentation);
  m_Controls->view->expandAll();

  m_LastValidSelectedLabels = {};

  //in singel selection mode, if at least one label exist select the first label of the mode.
  if (m_Segmentation.IsNotNull() && !this->GetMultiSelectionMode() && m_Segmentation->GetTotalNumberOfLabels() > 0)
  {
    auto firstIndex = m_Model->FirstLabelInstanceIndex(QModelIndex());
    auto labelVariant = firstIndex.data(QmitkMultiLabelTreeModel::ItemModelRole::LabelInstanceValueRole);

    if (labelVariant.isValid())
    {
      this->SetSelectedLabel(labelVariant.value<LabelValueType>());
      m_Controls->view->selectionModel()->setCurrentIndex(firstIndex, QItemSelectionModel::NoUpdate);
    }
  }
}

void QmitkMultiLabelInspector::SetMultiSelectionMode(bool multiMode)
{
  m_Controls->view->setSelectionMode(multiMode
     ? QAbstractItemView::SelectionMode::MultiSelection
     : QAbstractItemView::SelectionMode::SingleSelection);
}

bool QmitkMultiLabelInspector::GetMultiSelectionMode() const
{
  return QAbstractItemView::SelectionMode::MultiSelection == m_Controls->view->selectionMode();
}

void QmitkMultiLabelInspector::SetAllowVisibilityModification(bool visibilityMod)
{
  m_AllowVisibilityModification = visibilityMod;
  this->m_Model->SetAllowVisibilityModification(visibilityMod);
}

void QmitkMultiLabelInspector::SetAllowLabelModification(bool labelMod)
{
  m_AllowLabelModification = labelMod;
}

bool QmitkMultiLabelInspector::GetAllowVisibilityModification() const
{
  return m_AllowVisibilityModification;
}

void QmitkMultiLabelInspector::SetAllowLockModification(bool lockMod)
{
  m_AllowLockModification = lockMod;
  this->m_Model->SetAllowLockModification(lockMod);
}

bool QmitkMultiLabelInspector::GetAllowLockModification() const
{
  return m_AllowLockModification;
}

bool QmitkMultiLabelInspector::GetAllowLabelModification() const
{
  return m_AllowLabelModification;
}

void QmitkMultiLabelInspector::SetDefaultLabelNaming(bool defaultLabelNaming)
{
  m_DefaultLabelNaming = defaultLabelNaming;
}

void QmitkMultiLabelInspector::SetMultiLabelSegmentation(mitk::LabelSetImage* segmentation)
{
  if (segmentation != m_Segmentation)
  {
    m_Segmentation = segmentation;
    this->Initialize();
  }
}

void QmitkMultiLabelInspector::OnModelReset()
{
  m_LastValidSelectedLabels = {};
  m_ModelManipulationOngoing = false;
}

bool EqualLabelSelections(const QmitkMultiLabelInspector::LabelValueVectorType& selection1, const QmitkMultiLabelInspector::LabelValueVectorType& selection2)
{
  if (selection1.size() == selection2.size())
  {
    // lambda to compare node pointer inside both lists
    return std::is_permutation(selection1.begin(), selection1.end(), selection2.begin());
  }

  return false;
}

void QmitkMultiLabelInspector::SetSelectedLabels(const LabelValueVectorType& selectedLabels)
{
  if (EqualLabelSelections(this->GetSelectedLabels(), selectedLabels))
  {
    return;
  }

  this->UpdateSelectionModel(selectedLabels);
  m_LastValidSelectedLabels = selectedLabels;
}

void QmitkMultiLabelInspector::UpdateSelectionModel(const LabelValueVectorType& selectedLabels)
{
  // create new selection by retrieving the corresponding indices of the labels
  QItemSelection newCurrentSelection;
  for (const auto& labelID : selectedLabels)
  {
    QModelIndexList matched = m_Model->match(m_Model->index(0, 0), QmitkMultiLabelTreeModel::ItemModelRole::LabelInstanceValueRole, QVariant(labelID), 1, Qt::MatchRecursive);
    if (!matched.empty())
    {
      newCurrentSelection.select(matched.front(), matched.front());
    }
  }

  m_Controls->view->selectionModel()->select(newCurrentSelection, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Current);
}

void QmitkMultiLabelInspector::SetSelectedLabel(mitk::LabelSetImage::LabelValueType selectedLabel)
{
  this->SetSelectedLabels({ selectedLabel });
}

QmitkMultiLabelInspector::LabelValueVectorType QmitkMultiLabelInspector::GetSelectedLabelsFromSelectionModel() const
{
  LabelValueVectorType result;
  QModelIndexList selectedIndexes = m_Controls->view->selectionModel()->selectedIndexes();
  for (const auto& index : qAsConst(selectedIndexes))
  {
    QVariant qvariantDataNode = m_Model->data(index, QmitkMultiLabelTreeModel::ItemModelRole::LabelInstanceValueRole);
    if (qvariantDataNode.canConvert<mitk::LabelSetImage::LabelValueType>())
    {
      result.push_back(qvariantDataNode.value<mitk::LabelSetImage::LabelValueType>());
    }
  }
  return result;
}

QmitkMultiLabelInspector::LabelValueVectorType QmitkMultiLabelInspector::GetSelectedLabels() const
{
  return m_LastValidSelectedLabels;
}

mitk::Label* QmitkMultiLabelInspector::GetFirstSelectedLabelObject() const
{
  if (m_LastValidSelectedLabels.empty() || m_Segmentation.IsNull())
    return nullptr;

  return m_Segmentation->GetLabel(m_LastValidSelectedLabels.front());
}

void QmitkMultiLabelInspector::OnChangeModelSelection(const QItemSelection& /*selected*/, const QItemSelection& /*deselected*/)
{
  if (!m_ModelManipulationOngoing)
  {
    auto internalSelection = GetSelectedLabelsFromSelectionModel();
    if (internalSelection.empty())
    {
      //empty selections are not allowed by UI interactions, there should always be at least on label selected.
      //but selections are e.g. also cleared if the model is updated (e.g. due to addition of labels)
      UpdateSelectionModel(m_LastValidSelectedLabels);
    }
    else
    {
      m_LastValidSelectedLabels = internalSelection;
      emit CurrentSelectionChanged(GetSelectedLabels());
    }
  }
}

void QmitkMultiLabelInspector::WaitCursorOn() const
{
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
}

void QmitkMultiLabelInspector::WaitCursorOff() const
{
  this->RestoreOverrideCursor();
}

void QmitkMultiLabelInspector::RestoreOverrideCursor() const
{
  QApplication::restoreOverrideCursor();
}

mitk::Label* QmitkMultiLabelInspector::GetCurrentLabel() const
{
  auto currentIndex = this->m_Controls->view->currentIndex();
  auto labelVariant = currentIndex.data(QmitkMultiLabelTreeModel::ItemModelRole::LabelDataRole);
  mitk::Label::Pointer currentIndexLabel = nullptr;

  if (labelVariant.isValid())
  {
    auto uncastedLabel = labelVariant.value<void*>();
    currentIndexLabel = static_cast<mitk::Label*>(uncastedLabel);
  }
  return currentIndexLabel;
}

QmitkMultiLabelInspector::IndexLevelType QmitkMultiLabelInspector::GetCurrentLevelType() const
{
  auto currentIndex = this->m_Controls->view->currentIndex();
  auto labelInstanceVariant = currentIndex.data(QmitkMultiLabelTreeModel::ItemModelRole::LabelInstanceDataRole);
  auto labelVariant = currentIndex.data(QmitkMultiLabelTreeModel::ItemModelRole::LabelDataRole);

  if (labelInstanceVariant.isValid() )
  {
    return IndexLevelType::LabelInstance;
  }
  else if (labelVariant.isValid())
  {
    return IndexLevelType::LabelClass;
  }

  return IndexLevelType::Group;
}

QmitkMultiLabelInspector::LabelValueVectorType QmitkMultiLabelInspector::GetCurrentlyAffactedLabelInstances() const
{
  auto currentIndex = m_Controls->view->currentIndex();
  return m_Model->GetLabelsInSubTree(currentIndex);
}

mitk::Label* QmitkMultiLabelInspector::AddNewLabelInstanceInternal(mitk::Label* templateLabel)
{
  if (!m_AllowLabelModification)
    mitkThrow() << "QmitkMultiLabelInspector is configured incorrectly. Set AllowLabelModification to true to allow the usage of AddNewLabelInstance.";

  if (nullptr == templateLabel)
    mitkThrow() << "QmitkMultiLabelInspector is in an invalid state. AddNewLabelInstanceInternal was called with a non existing label as template";

  auto groupID = m_Segmentation->GetGroupIndexOfLabel(templateLabel->GetValue());
  auto group = m_Segmentation->GetLabelSet(groupID);
  m_ModelManipulationOngoing = true;
  auto newLabel = group->AddLabel(templateLabel, true);
  m_ModelManipulationOngoing = false;
  this->SetSelectedLabel(newLabel->GetValue());

  auto index = m_Model->indexOfLabel(newLabel->GetValue());
  if (index.isValid())
  {
    m_Controls->view->expand(index.parent());
  }
  else
  {
    mitkThrow() << "Segmentation or QmitkMultiLabelTreeModel is in an invalid state. Label is not present in the model after adding it to the segmentation. Label value: " << newLabel->GetValue();
  }

  return newLabel;
}

mitk::Label* QmitkMultiLabelInspector::AddNewLabelInstance()
{
  auto currentLabel = this->GetFirstSelectedLabelObject();
  if (nullptr == currentLabel)
    return nullptr;

  return this->AddNewLabelInstanceInternal(currentLabel);
}

mitk::Label* QmitkMultiLabelInspector::AddNewLabelInternal(const mitk::LabelSetImage::GroupIndexType& containingGroup)
{
  auto newLabel = mitk::LabelSetImageHelper::CreateNewLabel(m_Segmentation);

  if (!m_DefaultLabelNaming)
    emit LabelRenameRequested(newLabel, false);

  auto group = m_Segmentation->GetLabelSet(containingGroup);

  m_ModelManipulationOngoing = true;
  group->AddLabel(newLabel, false);
  m_ModelManipulationOngoing = false;

  this->SetSelectedLabel(newLabel->GetValue());

  auto index = m_Model->indexOfLabel(newLabel->GetValue());

  if (!index.isValid())
    mitkThrow() << "Segmentation or QmitkMultiLabelTreeModel is in an invalid state. Label is not present in the "
                   "model after adding it to the segmentation. Label value: " << newLabel->GetValue();

  m_Controls->view->expand(index.parent());

  return newLabel;
}

mitk::Label* QmitkMultiLabelInspector::AddNewLabel()
{
  if (!m_AllowLabelModification)
    mitkThrow() << "QmitkMultiLabelInspector is configured incorrectly. Set AllowLabelModification to true to allow the usage of AddNewLabel.";

  if (m_Segmentation.IsNull())
  {
    return nullptr;
  }

  auto currentLabel = this->GetFirstSelectedLabelObject();
  mitk::LabelSetImage::GroupIndexType groupID = nullptr != currentLabel
    ? m_Segmentation->GetGroupIndexOfLabel(currentLabel->GetValue())
    : 0;

  return AddNewLabelInternal(groupID);
}

void QmitkMultiLabelInspector::DeleteLabel()
{
  if (!m_AllowLabelModification)
    mitkThrow() << "QmitkMultiLabelInspector is configured incorrectly. Set AllowLabelModification to true to allow the usage of RemoveLabel.";

  if (m_Segmentation.IsNull())
  {
    return;
  }

  auto currentLabel = this->GetFirstSelectedLabelObject();
  auto question = "Do you really want to remove label \"" + QString::fromStdString(currentLabel->GetName()) + "\"?";

  auto answerButton = QMessageBox::question(this, QString("Remove label"), question, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

  if (answerButton == QMessageBox::Yes)
  {
    this->DeleteLabelInternal({ currentLabel->GetValue() });
  }
}

void QmitkMultiLabelInspector::DeleteLabelInternal(const LabelValueVectorType& labelValues)
{
  if (!m_AllowLabelModification)
    mitkThrow() << "QmitkMultiLabelInspector is configured incorrectly. Set AllowLabelModification to true to allow the usage of DeleteLabelInternal.";

  if (m_Segmentation.IsNull())
  {
    return;
  }

  QVariant nextLabelVariant;

  this->WaitCursorOn();
  m_ModelManipulationOngoing = true;
  for (auto labelValue : labelValues)
  {
    if (labelValue == labelValues.back())
    {
      auto currentIndex = m_Model->indexOfLabel(labelValue);
      auto nextIndex = m_Model->ClosestLabelInstanceIndex(currentIndex);
      nextLabelVariant = nextIndex.data(QmitkMultiLabelTreeModel::ItemModelRole::LabelInstanceValueRole);
    }

    m_Segmentation->RemoveLabel(labelValue);
  }
  m_ModelManipulationOngoing = false;
  this->WaitCursorOff();

  if (nextLabelVariant.isValid())
  {
    auto newLabelValue = nextLabelVariant.value<LabelValueType>();
    this->SetSelectedLabel(newLabelValue);

    auto index = m_Model->indexOfLabel(newLabelValue); //we have to get index again, because it could have changed due to remove operation.
    if (index.isValid())
    {
      m_Controls->view->expand(index.parent());
    }
    else
    {
      mitkThrow() << "Segmentation or QmitkMultiLabelTreeModel is in an invalid state. Label is not present in the model after adding it to the segmentation. Label value: " << newLabelValue;
    }
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

mitk::Label* QmitkMultiLabelInspector::AddNewGroup()
{
  if (!m_AllowLabelModification)
    mitkThrow() << "QmitkMultiLabelInspector is configured incorrectly. Set AllowLabelModification to true to allow the usage of AddNewLabel.";

  if (m_Segmentation.IsNull())
  {
    return nullptr;
  }

  mitk::LabelSetImage::GroupIndexType groupID = 0;
  mitk::Label* newLabel = nullptr;
  m_ModelManipulationOngoing = true;
  try
  {
    this->WaitCursorOn();
    groupID = m_Segmentation->AddLayer();
    this->WaitCursorOff();
    newLabel =  this->AddNewLabelInternal(groupID);
  }
  catch (mitk::Exception& e)
  {
    this->WaitCursorOff();
    m_ModelManipulationOngoing = false;
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    QMessageBox::information(this, "Add group", "Could not add a new group. See error log for details.");
  }
  m_ModelManipulationOngoing = false;

  return newLabel;
}

void QmitkMultiLabelInspector::RemoveGroupInternal(const mitk::LabelSetImage::GroupIndexType& groupID)
{
  if (!m_AllowLabelModification)
    mitkThrow() << "QmitkMultiLabelInspector is configured incorrectly. Set AllowLabelModification to true to allow the usage of RemoveLabel.";

  if (m_Segmentation.IsNull())
    return;

  auto currentIndex = m_Model->indexOfGroup(groupID);
  auto nextIndex = m_Model->ClosestLabelInstanceIndex(currentIndex);

  if (!nextIndex.isValid())
  {
    QMessageBox::information(this, "Delete group", "Cannot delete last remaining group. A segmentation must contain at least a single group.");
    return;
  }

  auto labelVariant = nextIndex.data(QmitkMultiLabelTreeModel::ItemModelRole::LabelInstanceValueRole);

  try
  {
    this->WaitCursorOn();
    m_ModelManipulationOngoing = true;
    m_Segmentation->RemoveGroup(groupID);
    m_ModelManipulationOngoing = false;
    this->WaitCursorOff();
  }
  catch (mitk::Exception& e)
  {
    m_ModelManipulationOngoing = false;
    this->WaitCursorOff();
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    QMessageBox::information(this, "Delete group", "Could not delete the currently active group. See error log for details.");
    return;
  }

  if (labelVariant.isValid())
  {
    auto newLabelValue = labelVariant.value<LabelValueType>();
    this->SetSelectedLabel(newLabelValue);

    auto index = m_Model->indexOfLabel(newLabelValue); //we have to get index again, because it could have changed due to remove operation.
    if (index.isValid())
    {
      m_Controls->view->expand(index.parent());
    }
    else
    {
      mitkThrow() << "Segmentation or QmitkMultiLabelTreeModel is in an invalid state. Label is not present in the model after adding it to the segmentation. Label value: " << newLabelValue;
    }
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkMultiLabelInspector::RemoveGroup()
{
  if (!m_AllowLabelModification)
    mitkThrow() << "QmitkMultiLabelInspector is configured incorrectly. Set AllowLabelModification to true to allow the usage of RemoveLabel.";

  if (m_Segmentation.IsNull())
  {
    return;
  }

  QString question = "Do you really want to delete the group of the selected label with all labels?";
  QMessageBox::StandardButton answerButton = QMessageBox::question(
    this, "Delete group", question, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

  if (answerButton != QMessageBox::Yes)
  {
    return;
  }

  auto currentLabel = GetFirstSelectedLabelObject();
  const auto currentGroup = m_Segmentation->GetGroupIndexOfLabel(currentLabel->GetValue());

  this->RemoveGroupInternal(currentGroup);
}

void QmitkMultiLabelInspector::OnDeleteGroup()
{
  if (!m_AllowLabelModification)
    mitkThrow() << "QmitkMultiLabelInspector is configured incorrectly. Set AllowLabelModification to true to allow the usage of RemoveLabel.";

  if (m_Segmentation.IsNull())
  {
    return;
  }

  auto currentIndex = this->m_Controls->view->currentIndex();
  auto groupIDVariant = currentIndex.data(QmitkMultiLabelTreeModel::ItemModelRole::GroupIDRole);

  if (groupIDVariant.isValid())
  {
    auto groupID = groupIDVariant.value<mitk::LabelSetImage::GroupIndexType>();

    QString question = "Do you really want to delete the current group with all labels?";
    QMessageBox::StandardButton answerButton = QMessageBox::question(
      this, QString("Delete group ") + QString::number(groupID), question, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

    if (answerButton != QMessageBox::Yes)
    {
      return;
    }

    this->RemoveGroupInternal(groupID);
  }
};


void QmitkMultiLabelInspector::OnContextMenuRequested(const QPoint& /*pos*/)
{
  if (m_Segmentation.IsNull() || !this->isEnabled())
    return;

  const auto indexLevel = this->GetCurrentLevelType();

  if (IndexLevelType::Group == indexLevel)
  {
    QMenu* menu = new QMenu(this);

    if (m_AllowLabelModification)
    {
      QAction* addInstanceAction = new QAction(QIcon(":/Qmitk/RenameLabel.png"), "&Add label", this);
      QObject::connect(addInstanceAction, &QAction::triggered, this, &QmitkMultiLabelInspector::OnAddLabel);
      menu->addAction(addInstanceAction);

      auto nextIndex = m_Model->ClosestLabelInstanceIndex(m_Controls->view->currentIndex());

      if (nextIndex.isValid())
      {
        QAction* removeAction = new QAction(QIcon(":/Qmitk/RemoveLabel.png"), "Delete group", this);
        QObject::connect(removeAction, &QAction::triggered, this, &QmitkMultiLabelInspector::OnDeleteGroup);
        menu->addAction(removeAction);
      }
    }

    if (m_AllowLockModification)
    {
      menu->addSeparator();
      QAction* lockAllAction = new QAction(QIcon(":/Qmitk/lock.png"), "Lock group", this);
      QObject::connect(lockAllAction, &QAction::triggered, this, &QmitkMultiLabelInspector::OnLockAffectedLabels);
      menu->addAction(lockAllAction);

      QAction* unlockAllAction = new QAction(QIcon(":/Qmitk/unlock.png"), "Unlock group", this);
      QObject::connect(unlockAllAction, &QAction::triggered, this, &QmitkMultiLabelInspector::OnUnlockAffectedLabels);
      menu->addAction(unlockAllAction);
    }

    if (m_AllowVisibilityModification)
    {
      menu->addSeparator();

      QAction* viewAllAction = new QAction(QIcon(":/Qmitk/visible.png"), "View group", this);
      QObject::connect(viewAllAction, &QAction::triggered, this, &QmitkMultiLabelInspector::OnSetAffectedLabelsVisible);
      menu->addAction(viewAllAction);

      QAction* hideAllAction = new QAction(QIcon(":/Qmitk/invisible.png"), "Hide group", this);
      QObject::connect(hideAllAction, &QAction::triggered, this, &QmitkMultiLabelInspector::OnSetAffectedLabelsInvisible);
      menu->addAction(hideAllAction);

      menu->addSeparator();

      auto opacityAction = this->CreateOpacityAction();
      if (nullptr != opacityAction)
        menu->addAction(opacityAction);
    }
    menu->popup(QCursor::pos());
  }
  else if (IndexLevelType::LabelClass == indexLevel)
  {
    QMenu* menu = new QMenu(this);

    if (m_AllowLabelModification)
    {
      QAction* addInstanceAction = new QAction(QIcon(":/Qmitk/RenameLabel.png"), "Add label instance", this);
      QObject::connect(addInstanceAction, &QAction::triggered, this, &QmitkMultiLabelInspector::OnAddLabelInstance);
      menu->addAction(addInstanceAction);

      QAction* renameAction = new QAction(QIcon(":/Qmitk/RenameLabel.png"), "&Rename label class", this);
      QObject::connect(renameAction, SIGNAL(triggered(bool)), this, SLOT(OnRenameLabel(bool)));
      menu->addAction(renameAction);

      QAction* removeAction = new QAction(QIcon(":/Qmitk/RemoveLabel.png"), "&Delete label class", this);
      QObject::connect(removeAction, &QAction::triggered, this, &QmitkMultiLabelInspector::OnDeleteAffectedLabel);
      menu->addAction(removeAction);
    }

    if (m_AllowLockModification)
    {
      menu->addSeparator();
      QAction* lockAllAction = new QAction(QIcon(":/Qmitk/lock.png"), "Lock label instances", this);
      QObject::connect(lockAllAction, &QAction::triggered, this, &QmitkMultiLabelInspector::OnLockAffectedLabels);
      menu->addAction(lockAllAction);

      QAction* unlockAllAction = new QAction(QIcon(":/Qmitk/unlock.png"), "Unlock label instances", this);
      QObject::connect(unlockAllAction, &QAction::triggered, this, &QmitkMultiLabelInspector::OnUnlockAffectedLabels);
      menu->addAction(unlockAllAction);
    }

    if (m_AllowVisibilityModification)
    {
      menu->addSeparator();

      QAction* viewAllAction = new QAction(QIcon(":/Qmitk/visible.png"), "View label instances", this);
      QObject::connect(viewAllAction, &QAction::triggered, this, &QmitkMultiLabelInspector::OnSetAffectedLabelsVisible);
      menu->addAction(viewAllAction);

      QAction* hideAllAction = new QAction(QIcon(":/Qmitk/invisible.png"), "Hide label instances", this);
      QObject::connect(hideAllAction, &QAction::triggered, this, &QmitkMultiLabelInspector::OnSetAffectedLabelsInvisible);
      menu->addAction(hideAllAction);

      QAction* viewOnlyAction = new QAction(QIcon(":/Qmitk/visible.png"), "View only", this);
      QObject::connect(viewOnlyAction, SIGNAL(triggered(bool)), this, SLOT(OnSetOnlyActiveLabelVisible(bool)));
      menu->addAction(viewOnlyAction);

      menu->addSeparator();

      auto opacityAction = this->CreateOpacityAction();
      if (nullptr!=opacityAction)
        menu->addAction(opacityAction);
    }
    menu->popup(QCursor::pos());
  }
  else
  {
    auto selectedLabelValues = this->GetSelectedLabels();
    if (selectedLabelValues.empty())
      return;

    QMenu* menu = new QMenu(this);

    if (this->GetMultiSelectionMode() && selectedLabelValues.size() > 1)
    {
      QAction* mergeAction = new QAction(QIcon(":/Qmitk/MergeLabels.png"), "Merge selection on current label", this);
      QObject::connect(mergeAction, SIGNAL(triggered(bool)), this, SLOT(OnMergeLabels(bool)));
      menu->addAction(mergeAction);

      QAction* removeLabelsAction = new QAction(QIcon(":/Qmitk/RemoveLabel.png"), "&Delete selected labels", this);
      QObject::connect(removeLabelsAction, SIGNAL(triggered(bool)), this, SLOT(OnDeleteLabels(bool)));
      menu->addAction(removeLabelsAction);

      QAction* clearLabelsAction = new QAction(QIcon(":/Qmitk/EraseLabel.png"), "&Clear selected labels", this);
      QObject::connect(clearLabelsAction, SIGNAL(triggered(bool)), this, SLOT(OnClearLabels(bool)));
      menu->addAction(clearLabelsAction);
    }
    else
    {
      if (m_AllowLabelModification)
      {
        QAction* addInstanceAction = new QAction(QIcon(":/Qmitk/RenameLabel.png"), "&Add label instance...", this);
        QObject::connect(addInstanceAction, &QAction::triggered, this, &QmitkMultiLabelInspector::OnAddLabelInstance);
        menu->addAction(addInstanceAction);

        QAction* renameAction = new QAction(QIcon(":/Qmitk/RenameLabel.png"), "&Rename", this);
        QObject::connect(renameAction, SIGNAL(triggered(bool)), this, SLOT(OnRenameLabel(bool)));
        menu->addAction(renameAction);

        QAction* removeAction = new QAction(QIcon(":/Qmitk/RemoveLabel.png"), "&Delete", this);
        QObject::connect(removeAction, &QAction::triggered, this, &QmitkMultiLabelInspector::DeleteLabel);
        menu->addAction(removeAction);

        QAction* clearAction = new QAction(QIcon(":/Qmitk/EraseLabel.png"), "&Clear content", this);
        QObject::connect(clearAction, SIGNAL(triggered(bool)), this, SLOT(OnClearLabel(bool)));
        menu->addAction(clearAction);
      }

      if (m_AllowVisibilityModification)
      {
        menu->addSeparator();
        QAction* viewOnlyAction = new QAction(QIcon(":/Qmitk/visible.png"), "View only", this);
        QObject::connect(viewOnlyAction, SIGNAL(triggered(bool)), this, SLOT(OnSetOnlyActiveLabelVisible(bool)));
        menu->addAction(viewOnlyAction);

        menu->addSeparator();

        auto opacityAction = this->CreateOpacityAction();
        if (nullptr != opacityAction)
          menu->addAction(opacityAction);
      }
    }
    menu->popup(QCursor::pos());
  }
}

QWidgetAction* QmitkMultiLabelInspector::CreateOpacityAction()
{
  auto relevantLabelValues = this->GetCurrentlyAffactedLabelInstances();
  std::vector<mitk::Label*> relevantLabels;

  if (!relevantLabelValues.empty())
  {
    //we assume here that all affacted label belong to one group.
    auto groupID = m_Segmentation->GetGroupIndexOfLabel(relevantLabelValues.front());
    auto group = m_Segmentation->GetLabelSet(groupID);

    for (auto value : relevantLabelValues)
    {
      auto label = this->m_Segmentation->GetLabel(value);
      if (nullptr == label)
        mitkThrow() << "Invalid state. Internal model returned a label value that does not exist in segmentation. Invalid value:" << value;
      relevantLabels.emplace_back(label);
    }

    auto* opacitySlider = new QSlider;
    opacitySlider->setMinimum(0);
    opacitySlider->setMaximum(100);
    opacitySlider->setOrientation(Qt::Horizontal);

    auto opacity = relevantLabels.front()->GetOpacity();
    opacitySlider->setValue(static_cast<int>(opacity * 100));
    auto segmentation = m_Segmentation;

    QObject::connect(opacitySlider, &QSlider::valueChanged, this, [segmentation, relevantLabels, group](const int value)
    {
      auto opacity = static_cast<float>(value) / 100.0f;
      for (auto label : relevantLabels)
      {
        label->SetOpacity(opacity);
        group->UpdateLookupTable(label->GetValue());
      }
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
    );

    QLabel* opacityLabel = new QLabel("Opacity: ");
    QVBoxLayout* opacityWidgetLayout = new QVBoxLayout;
    opacityWidgetLayout->setContentsMargins(4, 4, 4, 4);
    opacityWidgetLayout->addWidget(opacityLabel);
    opacityWidgetLayout->addWidget(opacitySlider);
    QWidget* opacityWidget = new QWidget;
    opacityWidget->setLayout(opacityWidgetLayout);
    QWidgetAction* opacityAction = new QWidgetAction(this);
    opacityAction->setDefaultWidget(opacityWidget);

    return opacityAction;
  }

  return nullptr;
}

void QmitkMultiLabelInspector::OnClearLabels(bool /*value*/)
{
  QString question = "Do you really want to clear the selected labels?";

  QMessageBox::StandardButton answerButton = QMessageBox::question(
    this, "Clear selected labels", question, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

  if (answerButton == QMessageBox::Yes)
  {
    this->WaitCursorOn();
    m_Segmentation->EraseLabels(this->GetSelectedLabels());
    this->WaitCursorOff();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkMultiLabelInspector::OnDeleteAffectedLabel()
{
  if (!m_AllowLabelModification)
    mitkThrow() << "QmitkMultiLabelInspector is configured incorrectly. Set AllowLabelModification to true to allow the usage of RemoveLabel.";

  if (m_Segmentation.IsNull())
  {
    return;
  }

  auto affectedLabels = GetCurrentlyAffactedLabelInstances();
  auto currentLabel = m_Segmentation->GetLabel(affectedLabels.front());
  QString question = "Do you really want to delete all label instances of class \"" + QString::fromStdString(currentLabel->GetName()) + "\"?";

  QMessageBox::StandardButton answerButton =
    QMessageBox::question(this, "Delete label class", question, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

  if (answerButton == QMessageBox::Yes)
  {
    this->DeleteLabelInternal(affectedLabels);
  }
}

void QmitkMultiLabelInspector::OnDeleteLabels(bool /*value*/)
{
  QString question = "Do you really want to remove the selected labels?";
  QMessageBox::StandardButton answerButton = QMessageBox::question(
    this, "Remove selected labels", question, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

  if (answerButton == QMessageBox::Yes)
  {
    this->WaitCursorOn();
    m_Segmentation->RemoveLabels(this->GetSelectedLabels());
    this->WaitCursorOff();
  }
}

void QmitkMultiLabelInspector::OnMergeLabels(bool /*value*/)
{
  auto currentLabel = GetCurrentLabel();
  QString question = "Do you really want to merge selected labels into \"" + QString::fromStdString(currentLabel->GetName())+"\"?";

  QMessageBox::StandardButton answerButton = QMessageBox::question(
    this, "Merge selected label", question, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

  if (answerButton == QMessageBox::Yes)
  {
    this->WaitCursorOn();
    m_Segmentation->MergeLabels(currentLabel->GetValue(), this->GetSelectedLabels(), m_Segmentation->GetActiveLayer());
    this->WaitCursorOff();

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkMultiLabelInspector::OnAddLabel()
{
  auto currentIndex = this->m_Controls->view->currentIndex();
  auto groupIDVariant = currentIndex.data(QmitkMultiLabelTreeModel::ItemModelRole::GroupIDRole);

  if (groupIDVariant.isValid())
  {
    auto groupID = groupIDVariant.value<mitk::LabelSetImage::GroupIndexType>();
    this->AddNewLabelInternal(groupID);
  }
}

void QmitkMultiLabelInspector::OnAddLabelInstance()
{
  auto currentLabel = this->GetCurrentLabel();
  if (nullptr == currentLabel)
    return;

  this->AddNewLabelInstanceInternal(currentLabel);
}

void QmitkMultiLabelInspector::OnClearLabel(bool /*value*/)
{
  auto currentLabel = GetFirstSelectedLabelObject();
  QString question = "Do you really want to clear the contents of label \"" + QString::fromStdString(currentLabel->GetName())+"\"?";

  QMessageBox::StandardButton answerButton =
    QMessageBox::question(this, "Clear label", question, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

  if (answerButton == QMessageBox::Yes)
  {
    this->WaitCursorOn();
    m_Segmentation->EraseLabel(currentLabel->GetValue());
    this->WaitCursorOff();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkMultiLabelInspector::OnRenameLabel(bool /*value*/)
{
  auto relevantLabelValues = this->GetCurrentlyAffactedLabelInstances();
  auto currentLabel = this->GetCurrentLabel();

  emit LabelRenameRequested(currentLabel, true);

  //we assume here that all affacted label belong to one group.
  auto groupID = m_Segmentation->GetGroupIndexOfLabel(currentLabel->GetValue());
  auto group = m_Segmentation->GetLabelSet(groupID);

  for (auto value : relevantLabelValues)
  {
    if (value != currentLabel->GetValue())
    {
      auto label = this->m_Segmentation->GetLabel(value);
      if (nullptr == label)
        mitkThrow() << "Invalid state. Internal model returned a label value that does not exist in segmentation. Invalid value:" << value;

      label->SetName(currentLabel->GetName());
      label->SetColor(currentLabel->GetColor());
      group->UpdateLookupTable(label->GetValue());
      mitk::DICOMSegmentationPropertyHelper::SetDICOMSegmentProperties(label);
    }
  }
}


void QmitkMultiLabelInspector::SetLockOfAffectedLabels(bool locked) const
{
  auto relevantLabelValues = this->GetCurrentlyAffactedLabelInstances();

  if (!relevantLabelValues.empty())
  {
    for (auto value : relevantLabelValues)
    {
      auto label = this->m_Segmentation->GetLabel(value);
      if (nullptr == label)
        mitkThrow() << "Invalid state. Internal model returned a label value that does not exist in segmentation. Invalid value:" << value;
      label->SetLocked(locked);
    }
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkMultiLabelInspector::OnUnlockAffectedLabels()
{
  this->SetLockOfAffectedLabels(false);
}

void QmitkMultiLabelInspector::OnLockAffectedLabels()
{
  this->SetLockOfAffectedLabels(true);
}

void QmitkMultiLabelInspector::SetVisibilityOfAffectedLabels(bool visible) const
{
  auto relevantLabelValues = this->GetCurrentlyAffactedLabelInstances();

  if (!relevantLabelValues.empty())
  {
    //we assume here that all affacted label belong to one group.
    auto groupID = m_Segmentation->GetGroupIndexOfLabel(relevantLabelValues.front());
    auto group = m_Segmentation->GetLabelSet(groupID);

    for (auto value : relevantLabelValues)
    {
      auto label = this->m_Segmentation->GetLabel(value);
      if (nullptr == label)
        mitkThrow() << "Invalid state. Internal model returned a label value that does not exist in segmentation. Invalid value:" << value;
      label->SetVisible(visible);
      group->UpdateLookupTable(label->GetValue());
    }
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkMultiLabelInspector::OnSetAffectedLabelsVisible()
{
  this->SetVisibilityOfAffectedLabels(true);
}

void QmitkMultiLabelInspector::OnSetAffectedLabelsInvisible()
{
  this->SetVisibilityOfAffectedLabels(false);
}

void QmitkMultiLabelInspector::OnSetOnlyActiveLabelVisible(bool /*value*/)
{
  auto currentLabel = GetFirstSelectedLabelObject();
  const auto labelID = currentLabel->GetValue();
  auto groupID = m_Segmentation->GetGroupIndexOfLabel(currentLabel->GetValue());
  auto group = m_Segmentation->GetLabelSet(groupID);
  group->SetAllLabelsVisible(false);

  currentLabel->SetVisible(true);
  group->UpdateLookupTable(labelID);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  this->PrepareGoToLabel(labelID);
}

void QmitkMultiLabelInspector::OnItemDoubleClicked(const QModelIndex& index)
{
  if (!index.isValid()) return;
  if (index.column() > 0) return;

  auto labelVariant = index.data(QmitkMultiLabelTreeModel::ItemModelRole::LabelInstanceValueRole);

  if (!labelVariant.isValid()) return;

  const auto labelID = labelVariant.value<mitk::Label::PixelType>();

  if (QApplication::queryKeyboardModifiers().testFlag(Qt::AltModifier))
  {
    this->OnRenameLabel(false);
    return;
  }

  this->PrepareGoToLabel(labelID);
}

void QmitkMultiLabelInspector::PrepareGoToLabel(mitk::Label::PixelType labelID) const
{
  this->WaitCursorOn();
  m_Segmentation->UpdateCenterOfMass(labelID);
  const auto currentLabel = m_Segmentation->GetLabel(labelID);
  const mitk::Point3D& pos = currentLabel->GetCenterOfMassCoordinates();
  this->WaitCursorOff();

  if (pos.GetVnlVector().max_value() > 0.0)
  {
    emit GoToLabel(currentLabel->GetValue(), pos);
  }
}

