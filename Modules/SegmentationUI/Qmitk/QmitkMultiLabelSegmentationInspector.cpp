/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkMultiLabelSegmentationInspector.h>

#include <QmitkMultiLabelSegmentationTreeModel.h>
#include <QmitkLabelColorItemDelegate.h>
#include <QmitkLabelToggleItemDelegate.h>
#include <QmitkStyleManager.h>

QmitkMultiLabelSegmentationInspector::QmitkMultiLabelSegmentationInspector(QWidget* parent/* = nullptr*/)
  : QWidget(parent)
{
  m_Controls.setupUi(this);

  m_Model = new QmitkMultiLabelSegmentationTreeModel(this);

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

  connect(m_Model, &QAbstractItemModel::modelReset, this, &QmitkMultiLabelSegmentationInspector::OnModelReset);
  connect(m_Controls.view->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), SLOT(ChangeModelSelection(const QItemSelection&, const QItemSelection&)));
}

void QmitkMultiLabelSegmentationInspector::Initialize()
{
  m_LastValidSelectedLabels = {};
  m_Model->SetSegmentation(m_Segmentation.Lock());
  m_Controls.view->expandAll();
}

void QmitkMultiLabelSegmentationInspector::SetSelectionMode(SelectionMode mode)
{
  m_Controls.view->setSelectionMode(mode);
}

QmitkMultiLabelSegmentationInspector::SelectionMode QmitkMultiLabelSegmentationInspector::GetSelectionMode() const
{
  return m_Controls.view->selectionMode();
}

void QmitkMultiLabelSegmentationInspector::SetMultiLabelSegmentation(mitk::LabelSetImage* segmentation)
{
  if (segmentation != m_Segmentation.Lock())
  {
    m_Segmentation = segmentation;
    this->Initialize();
  }
}

void QmitkMultiLabelSegmentationInspector::OnModelReset()
{
  m_LastValidSelectedLabels = {};
}

bool EqualLabelSelections(const QmitkMultiLabelSegmentationInspector::LabelValueVectorType& selection1, const QmitkMultiLabelSegmentationInspector::LabelValueVectorType& selection2)
{
  if (selection1.size() == selection2.size())
  {
    // lambda to compare node pointer inside both lists
    auto lambda = [](mitk::LabelSetImage::LabelValueType lhs, mitk::LabelSetImage::LabelValueType rhs) { return lhs == rhs; };
    return std::is_permutation(selection1.begin(), selection1.end(), selection2.begin(), selection2.end(), lambda);
  }

  return false;
}

void QmitkMultiLabelSegmentationInspector::SetSelectedLabels(const LabelValueVectorType& selectedLabels)
{
  bool equal = EqualLabelSelections(this->GetSelectedLabels(), selectedLabels);
  if (equal)
  {
    return;
  }

  this->UpdateSelectionModel(selectedLabels);
  m_LastValidSelectedLabels = selectedLabels;
}

void QmitkMultiLabelSegmentationInspector::UpdateSelectionModel(const LabelValueVectorType& selectedLabels)
{
  // create new selection by retrieving the corresponding indices of the labels
  QItemSelection newCurrentSelection;
  for (const auto& labelID : selectedLabels)
  {
    QModelIndexList matched = m_Model->match(m_Model->index(0, 0), QmitkMultiLabelSegmentationTreeModel::ItemModelRole::LabelValueRole, QVariant(labelID), 1, Qt::MatchRecursive);
    if (!matched.empty())
    {
      newCurrentSelection.select(matched.front(), matched.front());
    }
  }

  m_Controls.view->selectionModel()->select(newCurrentSelection, QItemSelectionModel::ClearAndSelect);
}

void QmitkMultiLabelSegmentationInspector::SetSelectedLabel(mitk::LabelSetImage::LabelValueType selectedLabel)
{
  this->SetSelectedLabels({ selectedLabel });
}

QmitkMultiLabelSegmentationInspector::LabelValueVectorType QmitkMultiLabelSegmentationInspector::GetSelectedLabelsFromSelectionModel() const
{
  LabelValueVectorType result;
  QModelIndexList selectedIndexes = m_Controls.view->selectionModel()->selectedIndexes();
  for (const auto& index : qAsConst(selectedIndexes))
  {
    QVariant qvariantDataNode = m_Model->data(index, QmitkMultiLabelSegmentationTreeModel::ItemModelRole::LabelValueRole);
    if (qvariantDataNode.canConvert<mitk::LabelSetImage::LabelValueType>())
    {
      result.push_back(qvariantDataNode.value<mitk::LabelSetImage::LabelValueType>());
    }
  }
  return result;
}

QmitkMultiLabelSegmentationInspector::LabelValueVectorType QmitkMultiLabelSegmentationInspector::GetSelectedLabels() const
{
  return m_LastValidSelectedLabels;
}

void QmitkMultiLabelSegmentationInspector::ChangeModelSelection(const QItemSelection& selected, const QItemSelection& deselected)
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

