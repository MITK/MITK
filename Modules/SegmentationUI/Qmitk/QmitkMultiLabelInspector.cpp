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

#include "ui_QmitkMultiLabelInspectorControls.h"

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

  this->m_Controls->view->setItemDelegateForColumn(1, m_LockItemDelegate);
  this->m_Controls->view->setItemDelegateForColumn(2, m_ColorItemDelegate);
  this->m_Controls->view->setItemDelegateForColumn(3, m_VisibilityItemDelegate);

  this->m_Controls->view->header()->setSectionResizeMode(0,QHeaderView::Stretch);
  this->m_Controls->view->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
  this->m_Controls->view->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
  this->m_Controls->view->header()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
  this->m_Controls->view->setContextMenuPolicy(Qt::CustomContextMenu);

  connect(m_Model, &QAbstractItemModel::modelReset, this, &QmitkMultiLabelInspector::OnModelReset);
  connect(m_Controls->view->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), SLOT(OnChangeModelSelection(const QItemSelection&, const QItemSelection&)));
  connect(m_Controls->view, &QAbstractItemView::customContextMenuRequested, this, &QmitkMultiLabelInspector::OnContextMenuRequested);
  connect(m_Controls->view, &QAbstractItemView::doubleClicked, this, &QmitkMultiLabelInspector::OnItemDoubleClicked);
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
  if (m_Segmentation->GetTotalNumberOfLabels() > 0 && !this->GetMultiSelectionMode())
  { //in singel selection mode, if at least one label exist select the first label of the mode.
    auto firstIndex = m_Model->FirstLabelInstanceIndex(QModelIndex());
    auto labelVariant = firstIndex.data(QmitkMultiLabelTreeModel::ItemModelRole::LabelInstanceValueRole);

    if (labelVariant.isValid())
    {
      this->SetSelectedLabel(labelVariant.value<LabelValueType>());
    }
  }
}

void QmitkMultiLabelInspector::SetMultiSelectionMode(bool multiMode)
{
  if (multiMode)
  {
    m_Controls->view->setSelectionMode(QAbstractItemView::SelectionMode::MultiSelection);
  }
  else
  {
    m_Controls->view->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
  }
}

bool QmitkMultiLabelInspector::GetMultiSelectionMode() const
{
  return QAbstractItemView::SelectionMode::MultiSelection == m_Controls->view->selectionMode();
}

void QmitkMultiLabelInspector::SetAllowVisibilityModification(bool vmod)
{
  m_AllowVisibilityModification = vmod;
  this->m_Model->SetAllowVisibilityModification(vmod);
}

void QmitkMultiLabelInspector::SetAllowLabelModification(bool labelMod)
{
  m_AllowLabelModification = labelMod;
}

bool QmitkMultiLabelInspector::GetAllowVisibilityModification() const
{
  return m_AllowVisibilityModification;
}

void QmitkMultiLabelInspector::SetAllowLockModification(bool lmod)
{
  m_AllowLockModification = lmod;
  this->m_Model->SetAllowLockModification(lmod);
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
    auto lambda = [](mitk::LabelSetImage::LabelValueType lhs, mitk::LabelSetImage::LabelValueType rhs) { return lhs == rhs; };
    return std::is_permutation(selection1.begin(), selection1.end(), selection2.begin(), selection2.end(), lambda);
  }

  return false;
}

void QmitkMultiLabelInspector::SetSelectedLabels(const LabelValueVectorType& selectedLabels)
{
  bool equal = EqualLabelSelections(this->GetSelectedLabels(), selectedLabels);
  if (equal)
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
      auto f = matched.front();
      auto d = f.data(QmitkMultiLabelTreeModel::ItemModelRole::LabelDataRole);
      newCurrentSelection.select(matched.front(), matched.front());
    }
  }

  m_Controls->view->selectionModel()->select(newCurrentSelection, QItemSelectionModel::ClearAndSelect);
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
  if (m_LastValidSelectedLabels.empty() || m_Segmentation.IsNull()) return nullptr;

  return m_Segmentation->GetLabel(m_LastValidSelectedLabels.front());
}

void QmitkMultiLabelInspector::OnChangeModelSelection(const QItemSelection& selected, const QItemSelection& deselected)
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

mitk::Label* QmitkMultiLabelInspector::AddNewLabelInstance()
{
  if (!m_AllowLabelModification)
    mitkThrow() << "QmitkMultiLabelInspector is configured incorrectly. Set AllowLabelModification to true to allow the usage of AddNewLabelInstance.";

  auto currentLabel = this->GetFirstSelectedLabelObject();
  if (nullptr == currentLabel)
    return nullptr;

  auto groupID = m_Segmentation->GetGroupIndexOfLabel(currentLabel->GetValue());
  auto group = m_Segmentation->GetLabelSet(groupID);
  m_ModelManipulationOngoing = true;
  auto newLabel = group->AddLabel(currentLabel, true);
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

mitk::Label* QmitkMultiLabelInspector::AddNewLabelInternal(const mitk::LabelSetImage::SpatialGroupIndexType& containingGroup)
{
  mitk::Label::Pointer newLabel = mitk::LabelSetImageHelper::CreateNewLabel(m_Segmentation);

  if (!m_DefaultLabelNaming)
  {
    emit LabelRenameRequested(newLabel, false);
  }

  auto group = m_Segmentation->GetLabelSet(containingGroup);
  m_ModelManipulationOngoing = true;
  group->AddLabel(newLabel, false);
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

mitk::Label* QmitkMultiLabelInspector::AddNewLabel()
{
  if (!m_AllowLabelModification)
    mitkThrow() << "QmitkMultiLabelInspector is configured incorrectly. Set AllowLabelModification to true to allow the usage of AddNewLabel.";

  if (m_Segmentation.IsNull())
  {
    return nullptr;
  }

  auto currentLabel = this->GetFirstSelectedLabelObject();
  mitk::LabelSetImage::SpatialGroupIndexType groupID = nullptr == currentLabel ? 0 : m_Segmentation->GetGroupIndexOfLabel(currentLabel->GetValue());

  return AddNewLabelInternal(groupID);
}

void QmitkMultiLabelInspector::RemoveLabel()
{
  if (!m_AllowLabelModification)
    mitkThrow() << "QmitkMultiLabelInspector is configured incorrectly. Set AllowLabelModification to true to allow the usage of RemoveLabel.";

  if (m_Segmentation.IsNull())
  {
    return;
  }

  auto currentLabel = GetFirstSelectedLabelObject();
  QString question = "Do you really want to remove label \"";
  question.append(
    QString::fromStdString(currentLabel->GetName()));
  question.append("\"?");

  QMessageBox::StandardButton answerButton =
    QMessageBox::question(this, "Remove label", question, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

  if (answerButton == QMessageBox::Yes)
  {
    auto currentIndex = m_Model->indexOfLabel(currentLabel->GetValue());
    auto nextIndex = m_Model->ClosestLabelInstanceIndex(currentIndex);
    auto labelVariant = nextIndex.data(QmitkMultiLabelTreeModel::ItemModelRole::LabelInstanceValueRole);

    this->WaitCursorOn();
    m_ModelManipulationOngoing = true;
    m_Segmentation->RemoveLabel(currentLabel->GetValue());
    m_ModelManipulationOngoing = false;
    this->WaitCursorOff();

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
}

mitk::Label* QmitkMultiLabelInspector::AddNewGroup()
{
  if (!m_AllowLabelModification)
    mitkThrow() << "QmitkMultiLabelInspector is configured incorrectly. Set AllowLabelModification to true to allow the usage of AddNewLabel.";

  if (m_Segmentation.IsNull())
  {
    return nullptr;
  }

  mitk::LabelSetImage::SpatialGroupIndexType groupID = 0;
  mitk::Label* newLabel = nullptr;
  m_ModelManipulationOngoing = true;
  try
  {
    this->WaitCursorOn();
    groupID = m_Segmentation->AddLayer();
    this->WaitCursorOff();
    newLabel =  AddNewLabelInternal(groupID);
  }
  catch (mitk::Exception& e)
  {
    this->WaitCursorOff();
    m_ModelManipulationOngoing = false;
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    QMessageBox::information(
      this, "Add layer", "Could not add a new layer. See error log for details.\n");
  }
  m_ModelManipulationOngoing = false;

  return newLabel;
}

void QmitkMultiLabelInspector::RemoveGroup()
{
  if (!m_AllowLabelModification)
    mitkThrow() << "QmitkMultiLabelInspector is configured incorrectly. Set AllowLabelModification to true to allow the usage of RemoveLabel.";

  if (m_Segmentation.IsNull())
  {
    return;
  }

  QString question = "Do you really want to delete the current layer with all labels?";
  QMessageBox::StandardButton answerButton = QMessageBox::question(
    this, "Delete layer", question, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

  if (answerButton != QMessageBox::Yes)
  {
    return;
  }

  auto currentLabel = GetFirstSelectedLabelObject();
  const auto currentGroup = m_Segmentation->GetGroupIndexOfLabel(currentLabel->GetValue());

  auto currentIndex = m_Model->indexOfGroup(currentGroup);
  auto nextIndex = m_Model->ClosestLabelInstanceIndex(currentIndex);
  auto labelVariant = nextIndex.data(QmitkMultiLabelTreeModel::ItemModelRole::LabelInstanceValueRole);

  try
  {
    this->WaitCursorOn();
    m_ModelManipulationOngoing = true;
    m_Segmentation->RemoveSpatialGroup(currentGroup);
    m_ModelManipulationOngoing = false;
    this->WaitCursorOff();
  }
  catch (mitk::Exception& e)
  {
    m_ModelManipulationOngoing = false;
    this->WaitCursorOff();
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    QMessageBox::information(
      this, "Delete layer", "Could not delete the currently active layer. See error log for details.\n");
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

void QmitkMultiLabelInspector::OnContextMenuRequested(const QPoint& pos)
{
  auto selectedLabelValues = this->GetSelectedLabels();
  if (m_Segmentation.IsNull() || !this->isEnabled() || selectedLabelValues.empty())
    return;

  if (nullptr == this->GetCurrentLabel())
    return; //for now we only have context menues if we realy click on an instance
            //context menues for group level and label class level will be added later.

  QMenu* menu = new QMenu(this);


  if (this->GetMultiSelectionMode() && selectedLabelValues.size()>1)
  {
    QAction* mergeAction = new QAction(QIcon(":/Qmitk/MergeLabels.png"), "Merge selection on current label", this);
    mergeAction->setEnabled(true);
    QObject::connect(mergeAction, SIGNAL(triggered(bool)), this, SLOT(OnMergeLabels(bool)));
    menu->addAction(mergeAction);

    QAction* removeLabelsAction = new QAction(QIcon(":/Qmitk/RemoveLabel.png"), "Remove selected labels", this);
    removeLabelsAction->setEnabled(true);
    QObject::connect(removeLabelsAction, SIGNAL(triggered(bool)), this, SLOT(OnRemoveLabels(bool)));
    menu->addAction(removeLabelsAction);

    QAction* eraseLabelsAction = new QAction(QIcon(":/Qmitk/EraseLabel.png"), "Erase selected labels", this);
    eraseLabelsAction->setEnabled(true);
    QObject::connect(eraseLabelsAction, SIGNAL(triggered(bool)), this, SLOT(OnEraseLabels(bool)));
    menu->addAction(eraseLabelsAction);
  }
  else
  {
    if (m_AllowLabelModification)
    {
      QAction* addInstanceAction = new QAction(QIcon(":/Qmitk/RenameLabel.png"), "Add label instance...", this);
      addInstanceAction->setEnabled(true);
      QObject::connect(addInstanceAction, &QAction::triggered, this, &QmitkMultiLabelInspector::AddNewLabelInstance);
      menu->addAction(addInstanceAction);

      QAction* renameAction = new QAction(QIcon(":/Qmitk/RenameLabel.png"), "Rename...", this);
      renameAction->setEnabled(true);
      QObject::connect(renameAction, SIGNAL(triggered(bool)), this, SLOT(OnRenameLabel(bool)));
      menu->addAction(renameAction);

      QAction* removeAction = new QAction(QIcon(":/Qmitk/RemoveLabel.png"), "Remove...", this);
      removeAction->setEnabled(true);
      QObject::connect(removeAction, &QAction::triggered, this, &QmitkMultiLabelInspector::RemoveLabel);
      menu->addAction(removeAction);

      QAction* eraseAction = new QAction(QIcon(":/Qmitk/EraseLabel.png"), "Erase...", this);
      eraseAction->setEnabled(true);
      QObject::connect(eraseAction, SIGNAL(triggered(bool)), this, SLOT(OnEraseLabel(bool)));
      menu->addAction(eraseAction);
    }

    if (m_AllowLockModification)
    {
      QAction* lockAllAction = new QAction(QIcon(":/Qmitk/lock.png"), "Lock group", this);
      lockAllAction->setEnabled(true);
      QObject::connect(lockAllAction, SIGNAL(triggered(bool)), this, SLOT(OnLockAllLabels(bool)));
      menu->addAction(lockAllAction);

      QAction* unlockAllAction = new QAction(QIcon(":/Qmitk/unlock.png"), "Unlock group", this);
      unlockAllAction->setEnabled(true);
      QObject::connect(unlockAllAction, SIGNAL(triggered(bool)), this, SLOT(OnUnlockAllLabels(bool)));
      menu->addAction(unlockAllAction);
    }

    if (m_AllowVisibilityModification)
    {
      QAction* viewOnlyAction = new QAction(QIcon(":/Qmitk/visible.png"), "View only", this);
      viewOnlyAction->setEnabled(true);
      QObject::connect(viewOnlyAction, SIGNAL(triggered(bool)), this, SLOT(OnSetOnlyActiveLabelVisible(bool)));
      menu->addAction(viewOnlyAction);

      QAction* viewAllAction = new QAction(QIcon(":/Qmitk/visible.png"), "View group", this);
      viewAllAction->setEnabled(true);
      QObject::connect(viewAllAction, SIGNAL(triggered(bool)), this, SLOT(OnSetAllLabelsVisible(bool)));
      menu->addAction(viewAllAction);

      QAction* hideAllAction = new QAction(QIcon(":/Qmitk/invisible.png"), "Hide group", this);
      hideAllAction->setEnabled(true);
      QObject::connect(hideAllAction, SIGNAL(triggered(bool)), this, SLOT(OnSetAllLabelsInvisible(bool)));
      menu->addAction(hideAllAction);

      QSlider* opacitySlider = new QSlider;
      opacitySlider->setMinimum(0);
      opacitySlider->setMaximum(100);
      opacitySlider->setOrientation(Qt::Horizontal);
      auto currentLabel = this->GetFirstSelectedLabelObject();
      auto opacity = currentLabel->GetOpacity();
      opacitySlider->setValue(static_cast<int>(opacity * 100));
      auto segmentation = m_Segmentation;
      QObject::connect(opacitySlider, &QSlider::valueChanged, this, [segmentation, currentLabel](const int value)
      {
        float opacity = static_cast<float>(value) / 100.0f;
        currentLabel->SetOpacity(opacity);
        auto groupID = segmentation->GetGroupIndexOfLabel(currentLabel->GetValue());
        auto group = segmentation->GetLabelSet(groupID);
        group->UpdateLookupTable(currentLabel->GetValue());
      }
      );
      QLabel* _OpacityLabel = new QLabel("Opacity: ");
      QVBoxLayout* _OpacityWidgetLayout = new QVBoxLayout;
      _OpacityWidgetLayout->setContentsMargins(4, 4, 4, 4);
      _OpacityWidgetLayout->addWidget(_OpacityLabel);
      _OpacityWidgetLayout->addWidget(opacitySlider);
      QWidget* _OpacityWidget = new QWidget;
      _OpacityWidget->setLayout(_OpacityWidgetLayout);
      QWidgetAction* OpacityAction = new QWidgetAction(this);
      OpacityAction->setDefaultWidget(_OpacityWidget);
      menu->addAction(OpacityAction);
    }


  }
  menu->popup(QCursor::pos());
}

void QmitkMultiLabelInspector::OnEraseLabels(bool /*value*/)
{
  QString question = "Do you really want to erase the selected labels?";

  QMessageBox::StandardButton answerButton = QMessageBox::question(
    this, "Erase selected labels", question, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

  if (answerButton == QMessageBox::Yes)
  {
    this->WaitCursorOn();
    m_Segmentation->EraseLabels(this->GetSelectedLabels());
    this->WaitCursorOff();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkMultiLabelInspector::OnRemoveLabels(bool /*value*/)
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
  QString question = "Do you really want to merge selected labels into \"";
  question.append(
    QString::fromStdString(currentLabel->GetName()));
  question.append("\"?");

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

void QmitkMultiLabelInspector::OnAddLabelInstance(bool /*value*/)
{
  this->AddNewLabelInstance();
}

void QmitkMultiLabelInspector::OnEraseLabel(bool /*value*/)
{
  auto currentLabel = GetFirstSelectedLabelObject();
  QString question = "Do you really want to erase the contents of label \"";
  question.append(
    QString::fromStdString(currentLabel->GetName()));
  question.append("\"?");

  QMessageBox::StandardButton answerButton =
    QMessageBox::question(this, "Erase label", question, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

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
  auto currentLabel = GetFirstSelectedLabelObject();
  emit LabelRenameRequested(currentLabel, true);
}

void QmitkMultiLabelInspector::OnUnlockAllLabels(bool /*value*/)
{
  auto currentLabel = GetFirstSelectedLabelObject();

  auto groupID = m_Segmentation->GetGroupIndexOfLabel(currentLabel->GetValue());
  auto group = m_Segmentation->GetLabelSet(groupID);
  group->SetAllLabelsLocked(false);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkMultiLabelInspector::OnLockAllLabels(bool /*value*/)
{
  auto currentLabel = GetFirstSelectedLabelObject();

  auto groupID = m_Segmentation->GetGroupIndexOfLabel(currentLabel->GetValue());
  auto group = m_Segmentation->GetLabelSet(groupID);
  group->SetAllLabelsLocked(true);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkMultiLabelInspector::OnSetAllLabelsVisible(bool /*value*/)
{
  auto currentLabel = GetFirstSelectedLabelObject();

  auto groupID = m_Segmentation->GetGroupIndexOfLabel(currentLabel->GetValue());
  auto group = m_Segmentation->GetLabelSet(groupID);
  group->SetAllLabelsVisible(true);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkMultiLabelInspector::OnSetAllLabelsInvisible(bool /*value*/)
{
  auto currentLabel = GetFirstSelectedLabelObject();

  auto groupID = m_Segmentation->GetGroupIndexOfLabel(currentLabel->GetValue());
  auto group = m_Segmentation->GetLabelSet(groupID);
  group->SetAllLabelsVisible(false);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
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

