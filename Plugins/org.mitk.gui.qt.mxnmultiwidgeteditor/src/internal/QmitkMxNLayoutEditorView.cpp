/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMxNLayoutEditorView.h"

#include <QmitkAbstractMultiWidgetEditor.h>
#include <QmitkMxNLayoutEditorWidget.h>
#include <QmitkMxNMultiWidget.h>
#include <QmitkMxNMultiWidgetEditor.h>

#include <mitkDataNode.h>

#include <QCheckBox>
#include <QMessageBox>
#include <QSignalBlocker>
#include <QVBoxLayout>

const std::string QmitkMxNLayoutEditorView::VIEW_ID = "org.mitk.views.mxnlayouteditor";

void QmitkMxNLayoutEditorView::CreateQtPartControl(QWidget* parent)
{
  auto* layout = new QVBoxLayout(parent);
  layout->setContentsMargins(0, 0, 0, 0);

  // Editor-wide mouse-interaction scheme. It lives here rather than in a
  // toolbar of its own because it is the only editor-wide control left outside
  // this view, and a full-height toolbar column for one checkbox costs canvas
  // the render windows can use.
  m_PacsSchemeBox = new QCheckBox(tr("PACS-like mouse interaction"), parent);
  m_PacsSchemeBox->setToolTip(tr("Select the left mouse button's action from a toolbar beside the "
                                 "render windows, instead of the MITK default button assignment"));
  layout->addWidget(m_PacsSchemeBox);

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

  // The scheme toggle belongs to the MxN editor part, so it is only live while
  // one is active; it shows the scheme already in effect rather than a default.
  auto* mxnEditor = dynamic_cast<QmitkMxNMultiWidgetEditor*>(multiWidgetEditor);
  m_PacsSchemeBox->setEnabled(nullptr != mxnEditor);
  if (nullptr != mxnEditor)
  {
    const QSignalBlocker blocker(m_PacsSchemeBox);
    m_PacsSchemeBox->setChecked(
      mitk::InteractionSchemeSwitcher::PACSStandard == mxnEditor->GetInteractionScheme());

    m_LayoutConnections.push_back(connect(
      m_PacsSchemeBox, &QCheckBox::toggled, m_LayoutEditorWidget, [mxnEditor](bool pacs) {
        mxnEditor->OnInteractionSchemeChanged(pacs
          ? mitk::InteractionSchemeSwitcher::PACSStandard
          : mitk::InteractionSchemeSwitcher::MITKStandard);
      }));
  }

  if (nullptr == multiWidget)
  {
    return;
  }

  // The layout editor reports what the user asked for; applying it is this
  // view's job. The three destructive paths (grid set, preset / file load,
  // data-based layout) are gated behind a confirmation: applying a new layout
  // rebuilds the cells and discards the current synchronization groups, so warn
  // first unless the configuration is still the trivial default
  // (ConfirmDestructiveLayoutChange). SaveLayout is read-only and stays direct.
  m_LayoutEditorWidget->SetDataStorage(this->GetDataStorage());
  m_LayoutConnections.push_back(connect(
    m_LayoutEditorWidget, &QmitkMxNLayoutEditorWidget::LayoutSet,
    m_LayoutEditorWidget, [this, multiWidgetEditor](int row, int column)
    {
      if (this->ConfirmDestructiveLayoutChange())
      {
        multiWidgetEditor->OnLayoutSet(row, column);
      }
    }));
  m_LayoutConnections.push_back(connect(
    m_LayoutEditorWidget, &QmitkMxNLayoutEditorWidget::SetDataBasedLayout,
    m_LayoutEditorWidget, [this, multiWidget](const QList<mitk::DataNode::Pointer>& nodes)
    {
      if (this->ConfirmDestructiveLayoutChange())
      {
        multiWidget->SetDataBasedLayout(nodes);
      }
    }));
  // Direct connection: the stream pointer is only valid during the emit.
  m_LayoutConnections.push_back(connect(
    m_LayoutEditorWidget, &QmitkMxNLayoutEditorWidget::SaveLayout,
    multiWidget, &QmitkMxNMultiWidget::SaveLayout, Qt::DirectConnection));
  m_LayoutConnections.push_back(connect(
    m_LayoutEditorWidget, &QmitkMxNLayoutEditorWidget::LoadLayout,
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
