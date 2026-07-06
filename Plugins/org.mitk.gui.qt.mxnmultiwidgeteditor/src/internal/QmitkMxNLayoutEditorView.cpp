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

  // The layout-shape controls talk to the editor part / multi widget with
  // the same connections the former toolbar popup had.
  auto* layoutSelection = m_LayoutEditorWidget->GetLayoutSelectionWidget();
  layoutSelection->SetDataStorage(this->GetDataStorage());
  m_LayoutConnections.push_back(connect(
    layoutSelection, &QmitkMultiWidgetLayoutSelectionWidget::LayoutSet,
    multiWidgetEditor, &QmitkAbstractMultiWidgetEditor::OnLayoutSet));
  m_LayoutConnections.push_back(connect(
    layoutSelection, &QmitkMultiWidgetLayoutSelectionWidget::SetDataBasedLayout,
    multiWidget, &QmitkMxNMultiWidget::SetDataBasedLayout));
  // Direct connection: the stream pointer is only valid during the emit.
  m_LayoutConnections.push_back(connect(
    layoutSelection, &QmitkMultiWidgetLayoutSelectionWidget::SaveLayout,
    multiWidget, &QmitkMxNMultiWidget::SaveLayout, Qt::DirectConnection));
  m_LayoutConnections.push_back(connect(
    layoutSelection, &QmitkMultiWidgetLayoutSelectionWidget::LoadLayout,
    multiWidget, &QmitkMxNMultiWidget::LoadLayout));
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
