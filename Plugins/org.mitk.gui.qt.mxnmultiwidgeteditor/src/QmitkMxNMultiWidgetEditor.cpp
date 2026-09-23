/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMxNMultiWidgetEditor.h"

#include <mitkCoreServices.h>
#include <mitkIPreferencesService.h>
#include <mitkIPreferences.h>
#include <mitkLog.h>

#include <berryIWorkbenchPage.h>
#include <berryIWorkbenchPartConstants.h>
#include <berryUIException.h>

// mxn multi widget editor plugin
#include <QmitkMultiWidgetDecorationManager.h>

// mitk qt widgets module
#include <QmitkMxNMultiWidget.h>
#include <QmitkInteractionSchemeToolBar.h>

// qt
#include <QHBoxLayout>

// c++
#include <optional>

const QString QmitkMxNMultiWidgetEditor::EDITOR_ID = "org.mitk.editors.mxnmultiwidget";

struct QmitkMxNMultiWidgetEditor::Impl final
{
  Impl();
  ~Impl() = default;

  QmitkInteractionSchemeToolBar* m_InteractionSchemeToolBar;

  // The scheme is editor-wide state the layout editor's toggle reads back, so
  // the two cannot disagree about which mode is live.
  mitk::InteractionSchemeSwitcher::InteractionScheme m_InteractionScheme;

  /** Empty until the preferences have been read for the first time. */
  std::optional<bool> m_PACSInteraction;
};

QmitkMxNMultiWidgetEditor::Impl::Impl()
  : m_InteractionSchemeToolBar(nullptr)
  , m_InteractionScheme(mitk::InteractionSchemeSwitcher::MITKStandard)
{
  // nothing here
}

//////////////////////////////////////////////////////////////////////////
// QmitkMxNMultiWidgetEditor
//////////////////////////////////////////////////////////////////////////
QmitkMxNMultiWidgetEditor::QmitkMxNMultiWidgetEditor()
  : QmitkAbstractMultiWidgetEditor()
  , m_Impl(std::make_unique<Impl>())
{
  // nothing here
}

QmitkMxNMultiWidgetEditor::~QmitkMxNMultiWidgetEditor()
{
  GetSite()->GetPage()->RemovePartListener(this);
}

berry::IPartListener::Events::Types QmitkMxNMultiWidgetEditor::GetPartEventTypes() const
{
  // Only OPENED: the other three existed solely to switch the built-in
  // render-window menu on and off, and this editor never shows it.
  return Events::OPENED;
}

void QmitkMxNMultiWidgetEditor::PartOpened(const berry::IWorkbenchPartReference::Pointer& partRef)
{
  if (partRef->GetId() == QmitkMxNMultiWidgetEditor::EDITOR_ID)
  {
    const auto& multiWidget = dynamic_cast<QmitkMxNMultiWidget*>(GetMultiWidget());
    if (nullptr != multiWidget)
    {
      multiWidget->EnableCrosshair();
    }
  }
}

void QmitkMxNMultiWidgetEditor::OnLayoutSet(int row, int column)
{
  const auto &multiWidget = dynamic_cast<QmitkMxNMultiWidget*>(GetMultiWidget());
  if (nullptr != multiWidget)
  {
    QmitkAbstractMultiWidgetEditor::OnLayoutSet(row, column);
    multiWidget->EnableCrosshair();
  }
}

void QmitkMxNMultiWidgetEditor::OnInteractionSchemeApplied(mitk::InteractionSchemeSwitcher::InteractionScheme scheme)
{
  m_Impl->m_InteractionScheme = scheme;
  m_Impl->m_InteractionSchemeToolBar->setVisible(QmitkAbstractMultiWidget::IsPACSScheme(scheme));
  m_Impl->m_InteractionSchemeToolBar->SetInteractionScheme(scheme);
}

mitk::InteractionSchemeSwitcher::InteractionScheme
QmitkMxNMultiWidgetEditor::GetInteractionScheme() const
{
  return m_Impl->m_InteractionScheme;
}

//////////////////////////////////////////////////////////////////////////
// PRIVATE
//////////////////////////////////////////////////////////////////////////
void QmitkMxNMultiWidgetEditor::SetFocus()
{
  const auto& multiWidget = GetMultiWidget();
  if (nullptr != multiWidget)
  {
    multiWidget->setFocus();
  }
}

void QmitkMxNMultiWidgetEditor::CreateQtPartControl(QWidget* parent)
{
  QHBoxLayout *layout = new QHBoxLayout(parent);
  layout->setContentsMargins(0, 0, 0, 0);

  auto* preferences = this->GetPreferences();

  // create left toolbar: interaction scheme toolbar to switch how the render window navigation behaves in PACS mode
  if (nullptr == m_Impl->m_InteractionSchemeToolBar)
  {
    m_Impl->m_InteractionSchemeToolBar = new QmitkInteractionSchemeToolBar(parent);
    // Keeps the tool bar from showing before the applied scheme is known; the
    // sync at the end of this method decides whether it stays hidden.
    m_Impl->m_InteractionSchemeToolBar->setVisible(false);
    layout->addWidget(m_Impl->m_InteractionSchemeToolBar);

    // The tool bar only requests a scheme; the multi widget reports back what it
    // applied. Keeping the two directions apart is what lets the tool bars follow
    // scheme changes that did not originate from them.
    connect(m_Impl->m_InteractionSchemeToolBar, &QmitkInteractionSchemeToolBar::InteractionSchemeChanged,
      this, &QmitkMxNMultiWidgetEditor::OnInteractionSchemeChanged);
  }

  auto multiWidget = GetMultiWidget();
  if (nullptr == multiWidget)
  {
    multiWidget = new QmitkMxNMultiWidget(parent);

    multiWidget->SetDataStorage(GetDataStorage());
    multiWidget->InitializeMultiWidget();
    SetMultiWidget(multiWidget);
    connect(static_cast<QmitkMxNMultiWidget*>(multiWidget), &QmitkMxNMultiWidget::LayoutChanged,
      this, &QmitkMxNMultiWidgetEditor::OnLayoutChanged);
    connect(static_cast<QmitkMxNMultiWidget*>(multiWidget), &QmitkMxNMultiWidget::LayoutEditorRequested,
      this, &QmitkMxNMultiWidgetEditor::OnLayoutEditorRequested);
    connect(multiWidget, &QmitkAbstractMultiWidget::InteractionSchemeChanged,
      this, &QmitkMxNMultiWidgetEditor::OnInteractionSchemeApplied);
  }

  layout->addWidget(multiWidget);

  // No configuration toolbar on the right: the layout editor is summoned from
  // the per-cell sync barcode and owns the layout and synchronization controls,
  // which left that toolbar holding the interaction-scheme switch alone - a
  // full-height column for one button. The switch lives in the layout editor
  // with the rest of the editor-wide configuration.

  GetSite()->GetPage()->AddPartListener(this);

  this->OnPreferencesChanged(preferences);

  this->OnInteractionSchemeApplied(multiWidget->GetInteractionScheme());
}

void QmitkMxNMultiWidgetEditor::OnPreferencesChanged(const mitk::IPreferences* preferences)
{
  const auto& multiWidget = GetMultiWidget();
  if (nullptr == multiWidget)
  {
    return;
  }

  // update decoration preferences
  //m_Impl->m_MultiWidgetDecorationManager->DecorationPreferencesChanged(preferences);

  int crosshairGapSize = preferences->GetInt("crosshair gap size", 32);
  multiWidget->SetCrosshairGap(crosshairGapSize);

  if (auto* mxnMultiWidget = dynamic_cast<QmitkMxNMultiWidget*>(multiWidget))
  {
    mxnMultiWidget->SetLevelWindowReadoutVisible(
      preferences->GetBool("Show level/window readout", true));
    mxnMultiWidget->SetNavigatorExpanded(preferences->GetBool("Expanded navigator", false));
  }

  // Only a change of the preference itself overrides the interaction scheme, so
  // that the PACS tool or the crosshair rotation mode the user picked survives
  // unrelated preference edits. A change that the active scheme already agrees
  // with is no reason to reset it either.
  const bool pacsInteraction = preferences->GetBool("PACS like mouse interaction", false);
  const bool preferenceChanged = m_Impl->m_PACSInteraction != pacsInteraction;
  m_Impl->m_PACSInteraction = pacsInteraction;

  if (preferenceChanged && pacsInteraction != QmitkAbstractMultiWidget::IsPACSScheme(multiWidget->GetInteractionScheme()))
  {
    this->OnInteractionSchemeChanged(pacsInteraction ?
      mitk::InteractionSchemeSwitcher::PACSStandard :
      mitk::InteractionSchemeSwitcher::MITKStandard);
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkMxNMultiWidgetEditor::OnLayoutChanged()
{
  FirePropertyChange(berry::IWorkbenchPartConstants::PROP_INPUT);
}

void QmitkMxNMultiWidgetEditor::OnLayoutEditorRequested(QmitkMxNMultiWidget::LayoutEditorRequest request)
{
  // A toggle request hides an already-visible layout editor instead of
  // re-activating it, so a second press on the barcode closes what the first
  // opened; a show request only ever brings the editor up.
  auto page = this->GetSite()->GetPage();
  if (page.IsNull())
  {
    return;
  }

  const QString viewId = QStringLiteral("org.mitk.views.mxnlayouteditor");
  auto view = page->FindView(viewId);
  if (QmitkMxNMultiWidget::LayoutEditorRequest::Toggle == request && view.IsNotNull()
      && page->IsPartVisible(view))
  {
    page->HideView(view);
    return;
  }

  try
  {
    page->ShowView(viewId);
  }
  catch (const berry::PartInitException& e)
  {
    MITK_ERROR << "Could not open the MxN layout editor view: " << e.what();
  }
}
