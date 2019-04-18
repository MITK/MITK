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


#include "QmitkMultiNodeSelectionWidget.h"

#include <algorithm>

#include "QmitkNodeSelectionDialog.h"
#include "QmitkCustomVariants.h"
#include "internal/QmitkNodeSelectionListItemWidget.h"

QmitkMultiNodeSelectionWidget::QmitkMultiNodeSelectionWidget(QWidget* parent) : QmitkAbstractNodeSelectionWidget(parent)
{
  m_Controls.setupUi(this);
  m_Overlay = new QmitkSimpleTextOverlayWidget(m_Controls.list);
  m_Overlay->setVisible(false);
  m_CheckFunction = [](const NodeList &) { return ""; };

  this->UpdateList();
  this->UpdateInfo();

  connect(m_Controls.btnChange, SIGNAL(clicked(bool)), this, SLOT(OnEditSelection()));
}

QmitkMultiNodeSelectionWidget::NodeList QmitkMultiNodeSelectionWidget::CompileEmitSelection() const
{
  NodeList result;

  for (int i = 0; i < m_Controls.list->count(); ++i)
  {
    QListWidgetItem* item = m_Controls.list->item(i);

    auto node = item->data(Qt::UserRole).value<mitk::DataNode::Pointer>();
    result.append(node);
  }


  if (!m_SelectOnlyVisibleNodes)
  {
    for (auto node : m_CurrentSelection)
    {
      if (!result.contains(node))
      {
        result.append(node);
      }
    }
  }

  return result;
}

void QmitkMultiNodeSelectionWidget::OnNodePredicateChanged(mitk::NodePredicateBase* /*newPredicate*/)
{
  this->UpdateInfo();
  this->UpdateList();
};

void QmitkMultiNodeSelectionWidget::OnDataStorageChanged()
{
  this->UpdateInfo();
  this->UpdateList();
};

QmitkMultiNodeSelectionWidget::NodeList QmitkMultiNodeSelectionWidget::GetSelectedNodes() const
{
  return m_CurrentSelection;
};

void QmitkMultiNodeSelectionWidget::SetSelectionCheckFunction(const SelectionCheckFunctionType &checkFunction)
{
  m_CheckFunction = checkFunction;

  auto newEmission = this->CompileEmitSelection();
  auto newCheckResponse = m_CheckFunction(newEmission);

  if (newCheckResponse.empty() && !m_CheckResponse.empty())
  {
    emit CurrentSelectionChanged(newEmission);
  }
  m_CheckResponse = newCheckResponse;
  this->UpdateInfo();
};

void QmitkMultiNodeSelectionWidget::OnEditSelection()
{
  QmitkNodeSelectionDialog* dialog = new QmitkNodeSelectionDialog(this, m_PopUpTitel, m_PopUpHint);

  dialog->SetDataStorage(m_DataStorage.Lock());
  dialog->SetNodePredicate(m_NodePredicate);
  dialog->SetCurrentSelection(this->CompileEmitSelection());
  dialog->SetSelectOnlyVisibleNodes(m_SelectOnlyVisibleNodes);
  dialog->SetSelectionMode(QAbstractItemView::MultiSelection);

  m_Controls.btnChange->setChecked(true);
  if (dialog->exec())
  {
    auto lastEmission = this->CompileEmitSelection();

    m_CurrentSelection = dialog->GetSelectedNodes();
    this->UpdateList();

    auto newEmission = this->CompileEmitSelection();

    m_CheckResponse = m_CheckFunction(newEmission);
    this->UpdateInfo();

    if (!EqualNodeSelections(lastEmission, newEmission))
    {
      if (m_CheckResponse.empty())
      {
        emit CurrentSelectionChanged(newEmission);
      }
    }
  }
  m_Controls.btnChange->setChecked(false);

  delete dialog;
};

void QmitkMultiNodeSelectionWidget::UpdateInfo()
{
  if (!m_Controls.list->count())
  {
    if (m_IsOptional)
    {
      m_Overlay->SetOverlayText(m_EmptyInfo);
    }
    else
    {
      m_Overlay->SetOverlayText(m_InvalidInfo);
    }
  }
  else
  {
    if (!m_CheckResponse.empty())
    {
      m_Overlay->SetOverlayText(QString::fromStdString(m_CheckResponse));
    }
  }

  m_Overlay->setVisible(m_Controls.list->count() == 0 || !m_CheckResponse.empty());

  for (auto i = 0; i < m_Controls.list->count(); ++i)
  {
    auto item = m_Controls.list->item(i);
    auto widget = qobject_cast<QmitkNodeSelectionListItemWidget*>(m_Controls.list->itemWidget(item));
    widget->SetClearAllowed(m_IsOptional || m_CurrentSelection.size() > 1);
  }
};

void QmitkMultiNodeSelectionWidget::UpdateList()
{
  m_Controls.list->clear();

  for (auto node : m_CurrentSelection)
  {
    if (m_NodePredicate.IsNull() || m_NodePredicate->CheckNode(node))
    {
      QListWidgetItem *newItem = new QListWidgetItem;

      newItem->setSizeHint(QSize(0, 40));
      QmitkNodeSelectionListItemWidget* widget = new QmitkNodeSelectionListItemWidget;

      widget->SetSelectedNode(node);
      widget->SetClearAllowed(m_IsOptional || m_CurrentSelection.size() > 1);

      connect(widget, &QmitkNodeSelectionListItemWidget::ClearSelection, this, &QmitkMultiNodeSelectionWidget::OnClearSelection);
      newItem->setData(Qt::UserRole, QVariant::fromValue<mitk::DataNode::Pointer>(node));

      m_Controls.list->addItem(newItem);
      m_Controls.list->setItemWidget(newItem, widget);
    }
  }
};

void QmitkMultiNodeSelectionWidget::SetSelectOnlyVisibleNodes(bool selectOnlyVisibleNodes)
{
  auto lastEmission = this->CompileEmitSelection();

  m_SelectOnlyVisibleNodes = selectOnlyVisibleNodes;

  auto newEmission = this->CompileEmitSelection();

  if (!EqualNodeSelections(lastEmission, newEmission))
  {
    m_CheckResponse = m_CheckFunction(newEmission);
    if (m_CheckResponse.empty())
    {
      emit CurrentSelectionChanged(newEmission);
    }
    this->UpdateList();
    this->UpdateInfo();
  }
};

void QmitkMultiNodeSelectionWidget::SetCurrentSelection(NodeList selectedNodes)
{
  auto lastEmission = this->CompileEmitSelection();

  m_CurrentSelection = selectedNodes;
  this->UpdateList();

  auto newEmission = this->CompileEmitSelection();

  if (!EqualNodeSelections(lastEmission, newEmission))
  {
    m_CheckResponse = m_CheckFunction(newEmission);
    if (m_CheckResponse.empty())
    {
      emit CurrentSelectionChanged(newEmission);
    }
    this->UpdateInfo();
  }
};

void QmitkMultiNodeSelectionWidget::OnClearSelection(const mitk::DataNode* node)
{
  auto finding = std::find(std::begin(m_CurrentSelection), std::end(m_CurrentSelection), node);
  m_CurrentSelection.erase(finding);

  this->UpdateList();
  auto newEmission = this->CompileEmitSelection();
  m_CheckResponse = m_CheckFunction(newEmission);

  if (m_CheckResponse.empty())
  {
    emit CurrentSelectionChanged(newEmission);
  }
  this->UpdateInfo();
};

void QmitkMultiNodeSelectionWidget::NodeRemovedFromStorage(const mitk::DataNode* node)
{
  auto finding = std::find(std::begin(m_CurrentSelection), std::end(m_CurrentSelection), node);

  if (finding != std::end(m_CurrentSelection))
  {
    this->OnClearSelection(node);
  }
}