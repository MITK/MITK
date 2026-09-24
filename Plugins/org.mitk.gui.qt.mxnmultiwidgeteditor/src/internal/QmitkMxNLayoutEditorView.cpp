/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMxNLayoutEditorView.h"

#include <QmitkAbstractMultiWidgetEditor.h>
#include <QmitkButtonOverlayWidget.h>
#include <QmitkIconTheme.h>
#include <QmitkMxNArrangeMode.h>
#include <QmitkMxNLayoutEditorWidget.h>
#include <QmitkMxNMultiWidget.h>
#include <QmitkMxNMultiWidgetEditor.h>

#include <berryIWorkbenchPage.h>

#include <mitkCoreServices.h>
#include <mitkDataNode.h>
#include <mitkDataStorageEditorInput.h>
#include <mitkDataStorageReference.h>
#include <mitkIDataStorageService.h>

#include <QCheckBox>
#include <QMessageBox>
#include <QTimer>

#include <memory>
#include <QSignalBlocker>
#include <QVBoxLayout>

const std::string QmitkMxNLayoutEditorView::VIEW_ID = "org.mitk.views.mxnlayouteditor";

QmitkMxNLayoutEditorView::~QmitkMxNLayoutEditorView()
{
  // A view closed without a Hidden() first must not leave its editor arranging.
  if (!m_ArrangedMultiWidget.isNull())
  {
    m_ArrangedMultiWidget->GetArrangeMode()->SetActive(false);
  }
}

void QmitkMxNLayoutEditorView::Activated()
{
}

void QmitkMxNLayoutEditorView::Deactivated()
{
}

void QmitkMxNLayoutEditorView::Visible()
{
  m_Visible = true;
  this->UpdateArrangeMode();
}

void QmitkMxNLayoutEditorView::Hidden()
{
  m_Visible = false;
  this->UpdateArrangeMode();
}

void QmitkMxNLayoutEditorView::UpdateArrangeMode()
{
  QmitkMxNMultiWidget* target =
    m_Visible && nullptr != m_LayoutEditorWidget ? m_LayoutEditorWidget->GetMultiWidget() : nullptr;
  if (m_ArrangedMultiWidget.data() == target)
  {
    return;
  }
  if (!m_ArrangedMultiWidget.isNull())
  {
    m_ArrangedMultiWidget->GetArrangeMode()->SetActive(false);
  }
  m_ArrangedMultiWidget = target;
  if (nullptr != target)
  {
    target->GetArrangeMode()->SetActive(true);
  }
}

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

  // The view configures a display it does not own, so without one it has
  // nothing to act on. Rather than leave the controls greyed out and
  // unexplained, say so over them and offer the one action that resolves it.
  //
  // Parented to the whole part, not to the editor widget: detaching disables
  // that widget, and Qt disables every child of a disabled widget - the
  // overlay's own button among them, which is exactly the button the user needs
  // at that moment. Covering the part also covers the scheme box, which is
  // equally inert without a display.
  m_NoDisplayOverlay = new QmitkButtonOverlayWidget(parent);
  m_NoDisplayOverlay->SetOverlayText(tr(
    "<b>No MxN display is open.</b><br/>This view configures the window "
    "arrangement and the synchronization of an MxN display."));
  m_NoDisplayOverlay->SetButtonText(tr(" Open MxN display"));
  m_NoDisplayOverlay->SetButtonIcon(
    QmitkIconTheme::GetIcon(QStringLiteral(":/Qmitk/mwLayout.svg")));
  m_NoDisplayOverlay->setOpacity(200);
  m_NoDisplayOverlay->setVisible(false);
  connect(m_NoDisplayOverlay, &QmitkButtonOverlayWidget::Clicked,
          this, &QmitkMxNLayoutEditorView::OpenMxNDisplay);

  // Wire the render window part that is already active when the view opens;
  // later activations arrive through the part listener.
  this->RenderWindowPartActivated(this->GetRenderWindowPart());
}

void QmitkMxNLayoutEditorView::OpenMxNDisplay()
{
  auto page = this->GetSite()->GetPage();
  if (page.IsNull())
  {
    return;
  }

  mitk::CoreServicePointer<mitk::IDataStorageService> storageService(
    mitk::CoreServices::GetDataStorageService());
  if (!storageService)
  {
    return;
  }

  // MATCH_ID: raise the display that is already there rather than open a second.
  auto storage = storageService->GetActiveDataStorageReference();
  berry::IEditorInput::Pointer input(new mitk::DataStorageEditorInput(storage));
  page->OpenEditor(input, QmitkMxNMultiWidgetEditor::EDITOR_ID, true,
                   berry::IWorkbenchPage::MATCH_ID);
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
  this->UpdateArrangeMode();
  if (nullptr != m_NoDisplayOverlay)
  {
    m_NoDisplayOverlay->setVisible(nullptr == multiWidget);
  }

  // The scheme toggle belongs to the MxN editor part, so it is only live while
  // one is active; it shows the scheme already in effect rather than a default.
  auto* mxnEditor = dynamic_cast<QmitkMxNMultiWidgetEditor*>(multiWidgetEditor);
  m_PacsSchemeBox->setEnabled(nullptr != mxnEditor);
  if (nullptr != mxnEditor)
  {
    // The box only requests a scheme; it shows what the multi widget applied,
    // wherever the change came from.
    auto showScheme = [this](mitk::InteractionSchemeSwitcher::InteractionScheme scheme) {
      const QSignalBlocker blocker(m_PacsSchemeBox);
      m_PacsSchemeBox->setChecked(QmitkAbstractMultiWidget::IsPACSScheme(scheme));
    };
    showScheme(mxnEditor->GetInteractionScheme());

    m_LayoutConnections.push_back(connect(
      m_PacsSchemeBox, &QCheckBox::toggled, m_LayoutEditorWidget, [mxnEditor](bool pacs) {
        mxnEditor->OnInteractionSchemeChanged(pacs
          ? mitk::InteractionSchemeSwitcher::PACSStandard
          : mitk::InteractionSchemeSwitcher::MITKStandard);
      }));
    if (nullptr != multiWidget)
    {
      m_LayoutConnections.push_back(connect(
        multiWidget, &QmitkAbstractMultiWidget::InteractionSchemeChanged, m_PacsSchemeBox, showScheme));
    }
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
      if (!this->ConfirmDestructiveLayoutChange())
      {
        return;
      }

      // Applying blocks the UI thread for a second or more on a large document,
      // and a window whose thread pumps no messages is not composited - so an
      // overlay raised and painted inside the apply never reaches the screen.
      // Raise it here, let this handler return so the event loop presents it,
      // and apply from a short timer.
      //
      // The document must be copied: 'jsonData' points at a local in the
      // emitter and dies with this emit. The sender's try/catch frame dies with
      // it too, so failures are reported here instead.
      multiWidget->ShowLayoutLoadFeedback();
      auto document = std::make_shared<nlohmann::json>(*jsonData);

      // One display frame is ~16 ms; this leaves the compositor room to present
      // the overlay before the thread stops answering.
      QTimer::singleShot(50, multiWidget, [this, multiWidget, document]()
      {
        try
        {
          multiWidget->ApplyLayout(*document);
        }
        catch (const std::exception& e)
        {
          QMessageBox::warning(m_LayoutEditorWidget, tr("Layout load failed"),
                               QString::fromUtf8(e.what()));
        }
        multiWidget->HideLayoutLoadFeedback();
      });
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
    this->UpdateArrangeMode();
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
