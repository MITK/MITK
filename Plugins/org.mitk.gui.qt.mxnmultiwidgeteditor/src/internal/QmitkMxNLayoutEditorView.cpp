/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMxNLayoutEditorView.h"

#include <QmitkAbstractMultiWidgetEditor.h>
#include <QmitkMultiWidgetLayoutSelectionWidget.h>
#include <QmitkMxNLayoutEditorWidget.h>
#include <QmitkMxNMultiWidget.h>

#include <mitkDataNode.h>

#include <QMessageBox>
#include <QVBoxLayout>

const std::string QmitkMxNLayoutEditorView::VIEW_ID = "org.mitk.views.mxnlayouteditor";

void QmitkMxNLayoutEditorView::CreateQtPartControl(QWidget* parent)
{
  auto* layout = new QVBoxLayout(parent);
  layout->setContentsMargins(0, 0, 0, 0);

  m_LayoutEditorWidget = new QmitkMxNLayoutEditorWidget(parent);
  layout->addWidget(m_LayoutEditorWidget);

  // Wire the render window part that is already active when the view opens;
  // later activations arrive through the part listener.
  this->RenderWindowPartActivated(this->GetRenderWindowPart());
}

void QmitkMxNLayoutEditorView::SetFocus()
{
  if (nullptr != m_LayoutEditorWidget)
  {
    m_LayoutEditorWidget->setFocus();
  }
}

void QmitkMxNLayoutEditorView::RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart)
{
  if (nullptr == m_LayoutEditorWidget)
  {
    return;
  }

  auto* multiWidgetEditor = dynamic_cast<QmitkAbstractMultiWidgetEditor*>(renderWindowPart);
  auto* multiWidget = nullptr != multiWidgetEditor
    ? dynamic_cast<QmitkMxNMultiWidget*>(multiWidgetEditor->GetMultiWidget())
    : nullptr;

  this->DisconnectLayoutControls();
  m_LayoutEditorWidget->SetMultiWidget(multiWidget);

  if (nullptr == multiWidget)
  {
    return;
  }

  // The layout-shape controls talk to the editor part / multi widget with the
  // same connections the former toolbar popup had, except that the three
  // destructive paths (grid set, preset / file load, data-based layout) are gated
  // behind a confirmation: applying a new layout rebuilds the cells and discards
  // the current synchronization groups, so warn first unless the configuration is
  // still the trivial default (ConfirmDestructiveLayoutChange). SaveLayout is
  // read-only and stays direct.
  auto* layoutSelection = m_LayoutEditorWidget->GetLayoutSelectionWidget();
  layoutSelection->SetDataStorage(this->GetDataStorage());
  m_LayoutConnections.push_back(connect(
    layoutSelection, &QmitkMultiWidgetLayoutSelectionWidget::LayoutSet,
    m_LayoutEditorWidget, [this, multiWidgetEditor](int row, int column)
    {
      if (this->ConfirmDestructiveLayoutChange())
      {
        multiWidgetEditor->OnLayoutSet(row, column);
      }
    }));
  m_LayoutConnections.push_back(connect(
    layoutSelection, &QmitkMultiWidgetLayoutSelectionWidget::SetDataBasedLayout,
    m_LayoutEditorWidget, [this, multiWidget](const QList<mitk::DataNode::Pointer>& nodes)
    {
      if (this->ConfirmDestructiveLayoutChange())
      {
        multiWidget->SetDataBasedLayout(nodes);
      }
    }));
  // Direct connection: the stream pointer is only valid during the emit.
  m_LayoutConnections.push_back(connect(
    layoutSelection, &QmitkMultiWidgetLayoutSelectionWidget::SaveLayout,
    multiWidget, &QmitkMxNMultiWidget::SaveLayout, Qt::DirectConnection));
  m_LayoutConnections.push_back(connect(
    layoutSelection, &QmitkMultiWidgetLayoutSelectionWidget::LoadLayout,
    m_LayoutEditorWidget, [this, multiWidget](const nlohmann::json* jsonData)
    {
      if (this->ConfirmDestructiveLayoutChange())
      {
        multiWidget->LoadLayout(jsonData);
      }
    }));
}

bool QmitkMxNLayoutEditorView::ConfirmDestructiveLayoutChange()
{
  if (nullptr == m_LayoutEditorWidget || !m_LayoutEditorWidget->HasNonTrivialSyncConfig())
  {
    return true;
  }

  const auto answer = QMessageBox::warning(
    m_LayoutEditorWidget, tr("Replace the current layout?"),
    tr("Applying a new layout replaces the current window arrangement and discards its "
       "synchronization groups. This cannot be undone.\n\nContinue?"),
    QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel);
  return QMessageBox::Yes == answer;
}

void QmitkMxNLayoutEditorView::RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart)
{
  if (nullptr == m_LayoutEditorWidget)
  {
    return;
  }

  auto* multiWidgetEditor = dynamic_cast<QmitkAbstractMultiWidgetEditor*>(renderWindowPart);
  if (nullptr != multiWidgetEditor
      && m_LayoutEditorWidget->GetMultiWidget() == multiWidgetEditor->GetMultiWidget())
  {
    this->DisconnectLayoutControls();
    m_LayoutEditorWidget->SetMultiWidget(nullptr);
  }
}

void QmitkMxNLayoutEditorView::DisconnectLayoutControls()
{
  for (const auto& connection : m_LayoutConnections)
  {
    QObject::disconnect(connection);
  }
  m_LayoutConnections.clear();
}
