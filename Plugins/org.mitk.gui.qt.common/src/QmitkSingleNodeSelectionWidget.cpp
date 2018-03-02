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


#include "QmitkSingleNodeSelectionWidget.h"

#include <QmitkNodeSelectionDialog.h>
#include <QmitkModelViewSelectionConnector.h>

QmitkSingleNodeSelectionWidget::QmitkSingleNodeSelectionWidget(QWidget* parent) : QmitkAbstractNodeSelectionWidget(parent)
{
  m_Controls.setupUi(this);

  this->UpdateInfo();

  m_Controls.nodeInfo->installEventFilter(this);
  m_Controls.nodeInfo->setVisible(true);
}

mitk::DataNode::Pointer QmitkSingleNodeSelectionWidget::ExtractCurrentValidSelection(const NodeList& nodes) const
{
  mitk::DataNode::Pointer result = nullptr;

  for (auto node : nodes)
  {
    bool valid = true;
    if (m_NodePredicate.IsNotNull())
    {
      valid = m_NodePredicate->CheckNode(node);
    }
    if (valid)
    {
      result = node;
      break;
    }
  }

  return result;
}

QmitkSingleNodeSelectionWidget::NodeList QmitkSingleNodeSelectionWidget::CompileEmitSelection() const
{
  NodeList result;

  if (!m_SelectOnlyVisibleNodes)
  {
    result = m_ExternalSelection;
  }

  if (m_SelectedNode.IsNotNull() && !result.contains(m_SelectedNode))
  {
    result.append(m_SelectedNode);
  }

  return result;
}

void QmitkSingleNodeSelectionWidget::OnNodePredicateChanged(mitk::NodePredicateBase* /*newPredicate*/)
{
    m_SelectedNode = this->ExtractCurrentValidSelection(m_ExternalSelection);
};

mitk::DataNode::Pointer QmitkSingleNodeSelectionWidget::GetSelectedNode() const
{
  return m_SelectedNode;
};

bool QmitkSingleNodeSelectionWidget::eventFilter(QObject *obj, QEvent *ev)
{
  if (obj == m_Controls.nodeInfo)
  {
    if (ev->type() == QEvent::MouseButtonRelease)
    {
      this->EditSelection();
      return true;
    }
  }

  return false;
}

void QmitkSingleNodeSelectionWidget::EditSelection()
{
  QmitkNodeSelectionDialog* dialog = new QmitkNodeSelectionDialog(this, m_PopUpTitel, m_PopUpHint);

  dialog->SetDataStorage(m_DataStorage.Lock());
  dialog->SetNodePredicate(m_NodePredicate);
  NodeList list;
  if (m_SelectedNode.IsNotNull())
  {
    list.append(m_SelectedNode);
  }
  dialog->SetCurrentSelection(list);
  dialog->SetSelectOnlyVisibleNodes(m_SelectOnlyVisibleNodes);
  dialog->SetSelectionMode(QAbstractItemView::SingleSelection);

  if (dialog->exec())
  {
    auto lastEmission = this->CompileEmitSelection();

    auto nodes = dialog->GetSelectedNodes();
    if (nodes.empty())
    {
      m_SelectedNode = nullptr;
    }
    else
    {
      m_SelectedNode = nodes.first();
    }

    auto newEmission = this->CompileEmitSelection();

    if (!EqualNodeSelections(lastEmission, newEmission))
    {
      emit CurrentSelectionChanged(newEmission);
      this->UpdateInfo();
    }
  }

  delete dialog;
};

void QmitkSingleNodeSelectionWidget::UpdateInfo()
{
  if (m_SelectedNode.IsNull())
  {
    if (m_IsOptional)
    {
      m_Controls.nodeInfo->setText(m_EmptyInfo);
    }
    else
    {
      m_Controls.nodeInfo->setText(m_InvalidInfo);
    }
  }
  else
  {
    auto name = m_SelectedNode->GetName();
    m_Controls.nodeInfo->setText(QString::fromStdString(name));
  }

};

void QmitkSingleNodeSelectionWidget::SetSelectOnlyVisibleNodes(bool selectOnlyVisibleNodes)
{
  auto lastEmission = this->CompileEmitSelection();

  m_SelectOnlyVisibleNodes = selectOnlyVisibleNodes;

  auto newEmission = this->CompileEmitSelection();

  if (!EqualNodeSelections(lastEmission, newEmission))
  {
    emit CurrentSelectionChanged(newEmission);
    this->UpdateInfo();
  }
};

void QmitkSingleNodeSelectionWidget::SetCurrentSelection(NodeList selectedNodes)
{
  auto lastEmission = this->CompileEmitSelection();

  m_ExternalSelection = selectedNodes;
  m_SelectedNode = this->ExtractCurrentValidSelection(selectedNodes);

  auto newEmission = this->CompileEmitSelection();

  if (!EqualNodeSelections(lastEmission, newEmission))
  {
    emit CurrentSelectionChanged(newEmission);
    this->UpdateInfo();
  }
};
