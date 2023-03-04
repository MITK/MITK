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

QmitkMultiLabelInspector::QmitkMultiLabelInspector(QWidget* parent/* = nullptr*/)
  : QWidget(parent)
{
  m_Controls.setupUi(this);

  m_Model = new QmitkMultiLabelTreeModel(this);

  m_Controls.view->setModel(m_Model);

  m_ColorItemDelegate = new QmitkLabelColorItemDelegate(this);

  auto visibleIcon = QmitkStyleManager::ThemeIcon(QLatin1String(":/Qmitk/visible.svg"));
  auto invisibleIcon = QmitkStyleManager::ThemeIcon(QLatin1String(":/Qmitk/invisible.svg"));
  m_VisibilityItemDelegate = new QmitkLabelToggleItemDelegate(visibleIcon, invisibleIcon, this);

  auto lockIcon = QmitkStyleManager::ThemeIcon(QLatin1String(":/Qmitk/lock.svg"));
  auto unlockIcon = QmitkStyleManager::ThemeIcon(QLatin1String(":/Qmitk/unlock.svg"));
  m_LockItemDelegate = new QmitkLabelToggleItemDelegate(lockIcon, unlockIcon, this);

  this->m_Controls.view->setItemDelegateForColumn(1, m_LockItemDelegate);
  this->m_Controls.view->setItemDelegateForColumn(2, m_ColorItemDelegate);
  this->m_Controls.view->setItemDelegateForColumn(3, m_VisibilityItemDelegate);

  this->m_Controls.view->header()->setSectionResizeMode(0,QHeaderView::Stretch);
  this->m_Controls.view->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
  this->m_Controls.view->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
  this->m_Controls.view->header()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
  this->m_Controls.view->setContextMenuPolicy(Qt::CustomContextMenu);

  connect(m_Model, &QAbstractItemModel::modelReset, this, &QmitkMultiLabelInspector::OnModelReset);
  connect(m_Controls.view->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), SLOT(ChangeModelSelection(const QItemSelection&, const QItemSelection&)));
  connect(m_Controls.view, &QAbstractItemView::customContextMenuRequested, this, &QmitkMultiLabelInspector::OnContextMenuRequested);
  connect(m_Controls.view, &QAbstractItemView::doubleClicked, this, &QmitkMultiLabelInspector::OnItemDoubleClicked);
}

void QmitkMultiLabelInspector::Initialize()
{
  m_LastValidSelectedLabels = {};
  m_Model->SetSegmentation(m_Segmentation);
  m_Controls.view->expandAll();
}

void QmitkMultiLabelInspector::SetMultiSelectionMode(bool multiMode)
{
  if (multiMode)
  {
    m_Controls.view->setSelectionMode(QAbstractItemView::SelectionMode::MultiSelection);
  }
  else
  {
    m_Controls.view->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
  }
}

bool QmitkMultiLabelInspector::GetMultiSelectionMode() const
{
  return QAbstractItemView::SelectionMode::MultiSelection == m_Controls.view->selectionMode();
}

void QmitkMultiLabelInspector::SetAllowVisibilityModification(bool vmod)
{
  m_AllowVisibilityModification = vmod;
}

bool QmitkMultiLabelInspector::GetAllowVisibilityModification() const
{
  return m_AllowVisibilityModification;
}

void QmitkMultiLabelInspector::SetAllowLockModification(bool lmod)
{
  m_AllowLockModification = lmod;
}

bool QmitkMultiLabelInspector::GetAllowLockModification() const
{
  return m_AllowLockModification;
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
    QModelIndexList matched = m_Model->match(m_Model->index(0, 0), QmitkMultiLabelTreeModel::ItemModelRole::LabelValueRole, QVariant(labelID), 1, Qt::MatchRecursive);
    if (!matched.empty())
    {
      newCurrentSelection.select(matched.front(), matched.front());
    }
  }

  m_Controls.view->selectionModel()->select(newCurrentSelection, QItemSelectionModel::ClearAndSelect);
}

void QmitkMultiLabelInspector::SetSelectedLabel(mitk::LabelSetImage::LabelValueType selectedLabel)
{
  this->SetSelectedLabels({ selectedLabel });
}

QmitkMultiLabelInspector::LabelValueVectorType QmitkMultiLabelInspector::GetSelectedLabelsFromSelectionModel() const
{
  LabelValueVectorType result;
  QModelIndexList selectedIndexes = m_Controls.view->selectionModel()->selectedIndexes();
  for (const auto& index : qAsConst(selectedIndexes))
  {
    QVariant qvariantDataNode = m_Model->data(index, QmitkMultiLabelTreeModel::ItemModelRole::LabelValueRole);
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

void QmitkMultiLabelInspector::ChangeModelSelection(const QItemSelection& selected, const QItemSelection& deselected)
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
  auto currentIndex = this->m_Controls.view->currentIndex();
  auto labelVariant = currentIndex.data(QmitkMultiLabelTreeModel::ItemModelRole::LabelDataRole);
  mitk::Label::Pointer currentIndexLabel = nullptr;

  if (labelVariant.isValid())
  {
    auto uncastedLabel = labelVariant.value<void*>();
    currentIndexLabel = static_cast<mitk::Label*>(uncastedLabel);
  }
  return currentIndexLabel;
}

void QmitkMultiLabelInspector::OnContextMenuRequested(const QPoint& pos)
{
  auto selectedLabelValues = this->GetSelectedLabels();
  auto currentLabel = this->GetCurrentLabel();
  if (nullptr== currentLabel)
    return;

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
      QAction* renameAction = new QAction(QIcon(":/Qmitk/RenameLabel.png"), "Rename...", this);
      renameAction->setEnabled(true);
      QObject::connect(renameAction, SIGNAL(triggered(bool)), this, SLOT(OnRenameLabel(bool)));
      menu->addAction(renameAction);

      QAction* removeAction = new QAction(QIcon(":/Qmitk/RemoveLabel.png"), "Remove...", this);
      removeAction->setEnabled(true);
      QObject::connect(removeAction, SIGNAL(triggered(bool)), this, SLOT(OnRemoveLabel(bool)));
      menu->addAction(removeAction);

      QAction* eraseAction = new QAction(QIcon(":/Qmitk/EraseLabel.png"), "Erase...", this);
      eraseAction->setEnabled(true);
      QObject::connect(eraseAction, SIGNAL(triggered(bool)), this, SLOT(OnEraseLabel(bool)));
      menu->addAction(eraseAction);
    }

    if (m_AllowLockModification)
    {
      QAction* lockAllAction = new QAction(QIcon(":/Qmitk/lock.png"), "Lock all", this);
      lockAllAction->setEnabled(true);
      QObject::connect(lockAllAction, SIGNAL(triggered(bool)), this, SLOT(OnLockAllLabels(bool)));
      menu->addAction(lockAllAction);

      QAction* unlockAllAction = new QAction(QIcon(":/Qmitk/unlock.png"), "Unlock all", this);
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

      QAction* viewAllAction = new QAction(QIcon(":/Qmitk/visible.png"), "View all", this);
      viewAllAction->setEnabled(true);
      QObject::connect(viewAllAction, SIGNAL(triggered(bool)), this, SLOT(OnSetAllLabelsVisible(bool)));
      menu->addAction(viewAllAction);

      QAction* hideAllAction = new QAction(QIcon(":/Qmitk/invisible.png"), "Hide all", this);
      hideAllAction->setEnabled(true);
      QObject::connect(hideAllAction, SIGNAL(triggered(bool)), this, SLOT(OnSetAllLabelsInvisible(bool)));
      menu->addAction(hideAllAction);

      QSlider* opacitySlider = new QSlider;
      opacitySlider->setMinimum(0);
      opacitySlider->setMaximum(100);
      opacitySlider->setOrientation(Qt::Horizontal);
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
    m_Segmentation->RemoveLabels(this->GetSelectedLabels(), m_Segmentation->GetActiveLayer());
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

void QmitkMultiLabelInspector::OnEraseLabel(bool /*value*/)
{
  auto currentLabel = GetCurrentLabel();
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

void QmitkMultiLabelInspector::OnRemoveLabel(bool /*value*/)
{
  auto currentLabel = GetCurrentLabel();
  QString question = "Do you really want to remove label \"";
  question.append(
    QString::fromStdString(currentLabel->GetName()));
  question.append("\"?");

  QMessageBox::StandardButton answerButton =
    QMessageBox::question(this, "Remove label", question, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

  if (answerButton == QMessageBox::Yes)
  {
    this->WaitCursorOn();
    m_Segmentation->RemoveLabel(currentLabel->GetValue(), m_Segmentation->GetActiveLayer());
    this->WaitCursorOff();
  }
}

void QmitkMultiLabelInspector::OnRenameLabel(bool /*value*/)
{
 //TODO
}

void QmitkMultiLabelInspector::OnUnlockAllLabels(bool /*value*/)
{
  auto currentLabel = GetCurrentLabel();

  auto groupID = m_Segmentation->GetGroupIndexOfLabel(currentLabel->GetValue());
  auto group = m_Segmentation->GetLabelSet(groupID);
  group->SetAllLabelsLocked(false);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkMultiLabelInspector::OnLockAllLabels(bool /*value*/)
{
  auto currentLabel = GetCurrentLabel();

  auto groupID = m_Segmentation->GetGroupIndexOfLabel(currentLabel->GetValue());
  auto group = m_Segmentation->GetLabelSet(groupID);
  group->SetAllLabelsLocked(true);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkMultiLabelInspector::OnSetAllLabelsVisible(bool /*value*/)
{
  auto currentLabel = GetCurrentLabel();

  auto groupID = m_Segmentation->GetGroupIndexOfLabel(currentLabel->GetValue());
  auto group = m_Segmentation->GetLabelSet(groupID);
  group->SetAllLabelsVisible(true);
}

void QmitkMultiLabelInspector::OnSetAllLabelsInvisible(bool /*value*/)
{
  auto currentLabel = GetCurrentLabel();

  auto groupID = m_Segmentation->GetGroupIndexOfLabel(currentLabel->GetValue());
  auto group = m_Segmentation->GetLabelSet(groupID);
  group->SetAllLabelsVisible(false);
}

void QmitkMultiLabelInspector::OnSetOnlyActiveLabelVisible(bool /*value*/)
{
  auto currentLabel = GetCurrentLabel();
  const auto labelID = currentLabel->GetValue();
  auto groupID = m_Segmentation->GetGroupIndexOfLabel(currentLabel->GetValue());
  auto group = m_Segmentation->GetLabelSet(groupID);
  group->SetAllLabelsVisible(false);
  currentLabel->SetVisible(true);

  group->UpdateLookupTable(labelID);

  this->PrepareGoToLabel(labelID);
}

void QmitkMultiLabelInspector::OnItemDoubleClicked(const QModelIndex& index)
{
  if (!index.isValid()) return;

  auto labelVariant = index.data(QmitkMultiLabelTreeModel::ItemModelRole::LabelValueRole);

  if (!labelVariant.isValid()) return;

  const auto labelID = labelVariant.value<mitk::Label::PixelType>();

  if (QApplication::queryKeyboardModifiers().testFlag(Qt::AltModifier))
  {
    this->OnRemoveLabel(false);
//  t  this->OnRenameLabelShortcutActivated();
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

