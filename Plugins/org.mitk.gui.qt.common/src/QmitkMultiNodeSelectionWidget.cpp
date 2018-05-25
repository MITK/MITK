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

#include <QmitkNodeSelectionDialog.h>
#include <QmitkCustomVariants.h>

QmitkMultiNodeSelectionWidget::QmitkMultiNodeSelectionWidget(QWidget* parent) : QmitkAbstractNodeSelectionWidget(parent)
{
  m_Controls.setupUi(this);

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
    this->UpdateInfo();

    auto newEmission = this->CompileEmitSelection();

    if (!EqualNodeSelections(lastEmission, newEmission))
    {
      emit CurrentSelectionChanged(newEmission);
    }
  }
  m_Controls.btnChange->setChecked(false);

  delete dialog;
};

void QmitkMultiNodeSelectionWidget::UpdateInfo()
{
  m_Controls.infoLabel->setVisible(m_Controls.list->count()==0);

  if (!m_Controls.list->count())
  {
    if (m_IsOptional)
    {
      m_Controls.infoLabel->setText(m_EmptyInfo);
    }
    else
    {
      m_Controls.infoLabel->setText(m_InvalidInfo);
    }
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
      newItem->setText(QString::fromStdString(node->GetName()));
      newItem->setData(Qt::UserRole, QVariant::fromValue<mitk::DataNode::Pointer>(node));

      m_Controls.list->addItem(newItem);
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
    emit CurrentSelectionChanged(newEmission);
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
    emit CurrentSelectionChanged(newEmission);
    this->UpdateInfo();
  }
};
