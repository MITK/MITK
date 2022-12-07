/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMultiNodeSelectionWidget.h"

#include <algorithm>

#include <QmitkCustomVariants.h>
#include <QmitkNodeSelectionDialog.h>
#include <QmitkNodeSelectionListItemWidget.h>

QmitkMultiNodeSelectionWidget::QmitkMultiNodeSelectionWidget(QWidget* parent)
  : QmitkAbstractNodeSelectionWidget(parent)
{
  m_Controls.setupUi(this);
  m_Overlay = new QmitkSimpleTextOverlayWidget(m_Controls.list);
  m_Overlay->setVisible(false);
  m_CheckFunction = [](const NodeList &) { return ""; };

  this->OnInternalSelectionChanged();
  this->UpdateInfo();

  connect(m_Controls.btnChange, SIGNAL(clicked(bool)), this, SLOT(OnEditSelection()));
}

void QmitkMultiNodeSelectionWidget::SetSelectionCheckFunction(const SelectionCheckFunctionType &checkFunction)
{
  m_CheckFunction = checkFunction;

  auto newEmission = this->CompileEmitSelection();
  auto newCheckResponse = m_CheckFunction(newEmission);

  if (newCheckResponse.empty() && !m_CheckResponse.empty())
  {
    this->EmitSelection(newEmission);
  }
  m_CheckResponse = newCheckResponse;
  this->UpdateInfo();
}

void QmitkMultiNodeSelectionWidget::OnEditSelection()
{
  QmitkNodeSelectionDialog* dialog = new QmitkNodeSelectionDialog(this, m_PopUpTitel, m_PopUpHint);

  dialog->SetDataStorage(m_DataStorage.Lock());
  dialog->SetNodePredicate(m_NodePredicate);
  dialog->SetCurrentSelection(this->CompileEmitSelection());
  dialog->SetSelectOnlyVisibleNodes(m_SelectOnlyVisibleNodes);
  dialog->SetSelectionMode(QAbstractItemView::MultiSelection);
  dialog->SetSelectionCheckFunction(m_CheckFunction);

  m_Controls.btnChange->setChecked(true);
  if (dialog->exec())
  {
    this->HandleChangeOfInternalSelection(dialog->GetSelectedNodes());
  }
  m_Controls.btnChange->setChecked(false);

  delete dialog;
}

void QmitkMultiNodeSelectionWidget::UpdateInfo()
{
  if (!m_Controls.list->count())
  {
    if (m_IsOptional)
    {
      if (this->isEnabled())
      {
        m_Overlay->SetOverlayText(QStringLiteral("<font class=\"normal\">") + m_EmptyInfo + QStringLiteral("</font>"));
      }
      else
      {
        m_Overlay->SetOverlayText(QStringLiteral("<font class=\"disabled\">") + m_EmptyInfo + QStringLiteral("</font>"));
      }
    }
    else
    {
      if (this->isEnabled())
      {
        m_Overlay->SetOverlayText(QStringLiteral("<font class=\"warning\">") + m_InvalidInfo + QStringLiteral("</font>"));
      }
      else
      {
        m_Overlay->SetOverlayText(QStringLiteral("<font class=\"disabled\">") + m_InvalidInfo + QStringLiteral("</font>"));
      }
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
    widget->SetClearAllowed(m_IsOptional || m_Controls.list->count() > 1);
  }
}

void QmitkMultiNodeSelectionWidget::OnInternalSelectionChanged()
{
  m_Controls.list->clear();
  auto currentSelection = this->GetCurrentInternalSelection();
  for (auto& node : currentSelection)
  {
    if (m_NodePredicate.IsNull() || m_NodePredicate->CheckNode(node))
    {
      QListWidgetItem *newItem = new QListWidgetItem;

      newItem->setSizeHint(QSize(0, 40));
      QmitkNodeSelectionListItemWidget* widget = new QmitkNodeSelectionListItemWidget;

      widget->SetSelectedNode(node);
      widget->SetClearAllowed(m_IsOptional || currentSelection.size() > 1);

      connect(widget, &QmitkNodeSelectionListItemWidget::ClearSelection, this, &QmitkMultiNodeSelectionWidget::OnClearSelection);
      newItem->setData(Qt::UserRole, QVariant::fromValue<mitk::DataNode::Pointer>(node));

      m_Controls.list->addItem(newItem);
      m_Controls.list->setItemWidget(newItem, widget);
    }
  }
}

void QmitkMultiNodeSelectionWidget::OnClearSelection(const mitk::DataNode* node)
{
  this->RemoveNodeFromSelection(node);
}

void QmitkMultiNodeSelectionWidget::changeEvent(QEvent *event)
{
  if (event->type() == QEvent::EnabledChange)
  {
    this->UpdateInfo();
  }
  QmitkAbstractNodeSelectionWidget::changeEvent(event);
}

bool QmitkMultiNodeSelectionWidget::AllowEmissionOfSelection(const NodeList& emissionCandidates) const
{
  m_CheckResponse = m_CheckFunction(emissionCandidates);
  return m_CheckResponse.empty();
}
