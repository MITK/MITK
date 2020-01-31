/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "QmitkSingleNodeSelectionWidget.h"

#include <berryQtStyleManager.h>

#include "mitkNodePredicateFunction.h"
#include "mitkNodePredicateAnd.h"

#include <QMouseEvent>

#include "QmitkNodeSelectionDialog.h"
#include "QmitkNodeDetailsDialog.h"

QmitkSingleNodeSelectionWidget::QmitkSingleNodeSelectionWidget(QWidget* parent) : QmitkAbstractNodeSelectionWidget(parent), m_AutoSelectNewNodes(false)
{
  m_Controls.setupUi(this);

  m_Controls.btnSelect->installEventFilter(this);
  m_Controls.btnSelect->setVisible(true);
  m_Controls.btnClear->setVisible(false);

  m_Controls.btnClear->setIcon(berry::QtStyleManager::ThemeIcon(QStringLiteral(":/org.mitk.gui.qt.common/times.svg")));

  this->UpdateInfo();

  connect(m_Controls.btnClear, SIGNAL(clicked(bool)), this, SLOT(OnClearSelection()));
}

QmitkSingleNodeSelectionWidget::~QmitkSingleNodeSelectionWidget()
{
}

mitk::DataNode::Pointer QmitkSingleNodeSelectionWidget::ExtractCurrentValidSelection(const NodeList& nodes) const
{
  mitk::DataNode::Pointer result = nullptr;

  if (!m_DataStorage.IsExpired())
  {
    auto storage = m_DataStorage.Lock();
    if (storage.IsNotNull())
    {
      for (auto node : nodes)
      {
        bool valid = storage->Exists(node);
        if (valid && m_NodePredicate.IsNotNull())
        {
          valid = m_NodePredicate->CheckNode(node);
        }

        if (valid)
        {
          result = node;
          break;
        }
      }
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

void QmitkSingleNodeSelectionWidget::OnNodePredicateChanged(const mitk::NodePredicateBase* /*newPredicate*/)
{
  auto lastEmission = this->CompileEmitSelection();

  if (m_NodePredicate.IsNotNull() && m_SelectedNode.IsNotNull() && !m_NodePredicate->CheckNode(m_SelectedNode))
  {
    m_SelectedNode = nullptr;
  }

  if (m_SelectedNode.IsNull())
  {
    m_SelectedNode = this->ExtractCurrentValidSelection(m_ExternalSelection);
  }

  this->DoAutoSelectIfNeeded();

  this->EmitAndUpdateIfNeeded(lastEmission);
};

void QmitkSingleNodeSelectionWidget::OnDataStorageChanged()
{
  auto lastEmission = this->CompileEmitSelection();

  if (m_DataStorage.IsExpired())
  {
    m_SelectedNode = nullptr;
  }
  else if (m_SelectedNode.IsNotNull())
  {
    auto storage = m_DataStorage.Lock();

    if (storage.IsNotNull() && !storage->Exists(m_SelectedNode))
    {
      m_SelectedNode = nullptr;
    }
  }

  if (m_SelectedNode.IsNull())
  {
    m_SelectedNode = this->ExtractCurrentValidSelection(m_ExternalSelection);
  }

  this->DoAutoSelectIfNeeded();

  this->EmitAndUpdateIfNeeded(lastEmission);
};

void QmitkSingleNodeSelectionWidget::OnClearSelection()
{
  if (m_IsOptional)
  {
    NodeList emptyList;
    this->SetCurrentSelection(emptyList);
  }

  this->UpdateInfo();
}

mitk::DataNode::Pointer QmitkSingleNodeSelectionWidget::GetSelectedNode() const
{
  return m_SelectedNode;
};

bool QmitkSingleNodeSelectionWidget::eventFilter(QObject *obj, QEvent *ev)
{
  if (obj == m_Controls.btnSelect)
  {
    if (ev->type() == QEvent::MouseButtonRelease)
    {
      auto mouseEv = dynamic_cast<QMouseEvent*>(ev);
      if (!mouseEv)
      {
        return false;
      }

      if (mouseEv->button() == Qt::LeftButton)
      {
        if (this->isEnabled())
        {
          this->EditSelection();
          return true;
        }
      }
      else
      {
        auto selection = this->CompileEmitSelection();
        if (!selection.empty())
        {
          QmitkNodeDetailsDialog infoDialog(selection, this);
          infoDialog.exec();
          return true;
        }
      }
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

  m_Controls.btnSelect->setChecked(true);

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

    this->EmitAndUpdateIfNeeded(lastEmission);
  }

  m_Controls.btnSelect->setChecked(false);

  delete dialog;
};

void QmitkSingleNodeSelectionWidget::UpdateInfo()
{
  if (m_SelectedNode.IsNull())
  {
    if (m_IsOptional)
    {
      m_Controls.btnSelect->SetNodeInfo(m_EmptyInfo);
    }
    else
    {
      m_Controls.btnSelect->SetNodeInfo(m_InvalidInfo);
    }
    m_Controls.btnSelect->SetSelectionIsOptional(m_IsOptional);
    m_Controls.btnClear->setVisible(false);
  }
  else
  {
    m_Controls.btnClear->setVisible(m_IsOptional);
  }

  m_Controls.btnSelect->SetSelectedNode(m_SelectedNode);
};

void QmitkSingleNodeSelectionWidget::SetSelectOnlyVisibleNodes(bool selectOnlyVisibleNodes)
{
  auto lastEmission = this->CompileEmitSelection();

  m_SelectOnlyVisibleNodes = selectOnlyVisibleNodes;

  this->EmitAndUpdateIfNeeded(lastEmission);
};

void QmitkSingleNodeSelectionWidget::DoAutoSelectIfNeeded(const mitk::DataNode* ignoreNode)
{
  if (m_SelectedNode.IsNull() && m_AutoSelectNewNodes && !m_DataStorage.IsExpired())
  {
    auto storage = m_DataStorage.Lock();
    if (storage.IsNotNull())
    {
      auto ignoreCheck = [ignoreNode](const mitk::DataNode * node)
      {
        return node != ignoreNode;
      };
      mitk::NodePredicateFunction::Pointer isNotIgnoredNode = mitk::NodePredicateFunction::New(ignoreCheck);
      mitk::NodePredicateBase::Pointer predicate = isNotIgnoredNode.GetPointer();

      if (m_NodePredicate.IsNotNull())
      {
        predicate = mitk::NodePredicateAnd::New(m_NodePredicate.GetPointer(), predicate.GetPointer()).GetPointer();
      }

      m_SelectedNode = storage->GetNode(predicate);
    }
  }
}

void QmitkSingleNodeSelectionWidget::EmitAndUpdateIfNeeded(const NodeList& lastEmission)
{
  auto newEmission = this->CompileEmitSelection();

  if (!EqualNodeSelections(lastEmission, newEmission))
  {
    this->UpdateInfo();
    emit CurrentSelectionChanged(newEmission);
  }
};

void QmitkSingleNodeSelectionWidget::SetCurrentSelectedNode(mitk::DataNode* selectedNode)
{
  NodeList selection;
  if (selectedNode)
  {
    selection.append(selectedNode);
  }
  this->SetCurrentSelection(selection);
};

void QmitkSingleNodeSelectionWidget::SetCurrentSelection(NodeList selectedNodes)
{
  auto lastEmission = this->CompileEmitSelection();

  m_ExternalSelection = selectedNodes;
  m_SelectedNode = this->ExtractCurrentValidSelection(selectedNodes);

  this->DoAutoSelectIfNeeded();

  this->EmitAndUpdateIfNeeded(lastEmission);
};

void QmitkSingleNodeSelectionWidget::NodeAddedToStorage(const mitk::DataNode* /*node*/)
{
  if (m_SelectedNode.IsNull() && m_AutoSelectNewNodes && !m_DataStorage.IsExpired())
  {
    auto lastEmission = this->CompileEmitSelection();

    this->DoAutoSelectIfNeeded();

    this->EmitAndUpdateIfNeeded(lastEmission);
  }
}

void QmitkSingleNodeSelectionWidget::NodeRemovedFromStorage(const mitk::DataNode* node)
{
  if (m_SelectedNode == node && node != nullptr)
  {
    m_SelectedNode = nullptr;

    //This event is triggerd before the node is realy removed from storage.
    //Therefore we have to explictly ignore him in the autoselect search.
    this->DoAutoSelectIfNeeded(node);

    auto newEmission = this->CompileEmitSelection();

    emit CurrentSelectionChanged(newEmission);
    this->UpdateInfo();
  }
}

bool QmitkSingleNodeSelectionWidget::GetAutoSelectNewNodes() const
{
  return m_AutoSelectNewNodes;
}

void QmitkSingleNodeSelectionWidget::SetAutoSelectNewNodes(bool autoSelect)
{
  m_AutoSelectNewNodes = autoSelect;
  this->NodeAddedToStorage(nullptr);
}
