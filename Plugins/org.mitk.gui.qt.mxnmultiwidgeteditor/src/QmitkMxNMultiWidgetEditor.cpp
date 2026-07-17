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
#include <QmitkMultiWidgetConfigurationToolBar.h>

// qt
#include <QHBoxLayout>

const QString QmitkMxNMultiWidgetEditor::EDITOR_ID = "org.mitk.editors.mxnmultiwidget";

struct QmitkMxNMultiWidgetEditor::Impl final
{
  Impl();
  ~Impl() = default;

  QmitkInteractionSchemeToolBar* m_InteractionSchemeToolBar;
  QmitkMultiWidgetConfigurationToolBar* m_ConfigurationToolBar;
};

QmitkMxNMultiWidgetEditor::Impl::Impl()
  : m_InteractionSchemeToolBar(nullptr)
  , m_ConfigurationToolBar(nullptr)
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
  return Events::CLOSED | Events::OPENED | Events::HIDDEN | Events::VISIBLE;
}

void QmitkMxNMultiWidgetEditor::PartClosed(const berry::IWorkbenchPartReference::Pointer& partRef)
{
  if (partRef->GetId() == QmitkMxNMultiWidgetEditor::EDITOR_ID)
  {
    const auto& multiWidget = dynamic_cast<QmitkMxNMultiWidget*>(GetMultiWidget());
    if (nullptr != multiWidget)
    {
      multiWidget->ActivateMenuWidget(false);
    }
  }
}

void QmitkMxNMultiWidgetEditor::PartOpened(const berry::IWorkbenchPartReference::Pointer& partRef)
{
  if (partRef->GetId() == QmitkMxNMultiWidgetEditor::EDITOR_ID)
  {
    const auto& multiWidget = dynamic_cast<QmitkMxNMultiWidget*>(GetMultiWidget());
    if (nullptr != multiWidget)
    {
      multiWidget->EnableCrosshair();
      multiWidget->ActivateMenuWidget(true);
    }
  }
}

void QmitkMxNMultiWidgetEditor::PartHidden(const berry::IWorkbenchPartReference::Pointer& partRef)
{
  if (partRef->GetId() == QmitkMxNMultiWidgetEditor::EDITOR_ID)
  {
    const auto& multiWidget = dynamic_cast<QmitkMxNMultiWidget*>(GetMultiWidget());
    if (nullptr != multiWidget)
    {
      multiWidget->ActivateMenuWidget(false);
    }
  }
}

void QmitkMxNMultiWidgetEditor::PartVisible(const berry::IWorkbenchPartReference::Pointer& partRef)
{
  if (partRef->GetId() == QmitkMxNMultiWidgetEditor::EDITOR_ID)
  {
    const auto& multiWidget = dynamic_cast<QmitkMxNMultiWidget*>(GetMultiWidget());
    if (nullptr != multiWidget)
    {
      multiWidget->ActivateMenuWidget(true);
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

void QmitkMxNMultiWidgetEditor::OnInteractionSchemeChanged(mitk::InteractionSchemeSwitcher::InteractionScheme scheme)
{
  const auto &multiWidget = GetMultiWidget();
  if (nullptr == multiWidget)
  {
    return;
  }

  if (mitk::InteractionSchemeSwitcher::PACSStandard == scheme)
  {
    m_Impl->m_InteractionSchemeToolBar->setVisible(true);
  }
  else
  {
    m_Impl->m_InteractionSchemeToolBar->setVisible(false);
  }

  QmitkAbstractMultiWidgetEditor::OnInteractionSchemeChanged(scheme);
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

  auto multiWidget = GetMultiWidget();
  if (nullptr == multiWidget)
  {
    multiWidget = new QmitkMxNMultiWidget(parent);

    // create left toolbar: interaction scheme toolbar to switch how the render window navigation behaves in PACS mode
    if (nullptr == m_Impl->m_InteractionSchemeToolBar)
    {
      m_Impl->m_InteractionSchemeToolBar = new QmitkInteractionSchemeToolBar(parent);
      layout->addWidget(m_Impl->m_InteractionSchemeToolBar);
    }
    m_Impl->m_InteractionSchemeToolBar->SetInteractionEventHandler(multiWidget->GetInteractionEventHandler());

    multiWidget->SetDataStorage(GetDataStorage());
    multiWidget->InitializeMultiWidget();
    SetMultiWidget(multiWidget);
    connect(static_cast<QmitkMxNMultiWidget*>(multiWidget), &QmitkMxNMultiWidget::LayoutChanged,
      this, &QmitkMxNMultiWidgetEditor::OnLayoutChanged);
    connect(static_cast<QmitkMxNMultiWidget*>(multiWidget), &QmitkMxNMultiWidget::LayoutEditorRequested,
      this, &QmitkMxNMultiWidgetEditor::OnLayoutEditorRequested);
  }

  layout->addWidget(multiWidget);

  // create right toolbar: configuration toolbar to change the render window widget layout
  if (nullptr == m_Impl->m_ConfigurationToolBar)
  {
    m_Impl->m_ConfigurationToolBar = new QmitkMultiWidgetConfigurationToolBar(multiWidget);
    layout->addWidget(m_Impl->m_ConfigurationToolBar);
  }

  // The layout-shape controls live in the layout editor view; the toolbar
  // button only summons it.
  connect(m_Impl->m_ConfigurationToolBar, &QmitkMultiWidgetConfigurationToolBar::LayoutEditorRequested,
          this, &QmitkMxNMultiWidgetEditor::OnLayoutEditorRequested);
  connect(m_Impl->m_ConfigurationToolBar, &QmitkMultiWidgetConfigurationToolBar::Synchronized,
          this, &QmitkMxNMultiWidgetEditor::OnSynchronize);
  connect(m_Impl->m_ConfigurationToolBar, &QmitkMultiWidgetConfigurationToolBar::InteractionSchemeChanged,
          this, &QmitkMxNMultiWidgetEditor::OnInteractionSchemeChanged);

  GetSite()->GetPage()->AddPartListener(this);

  OnPreferencesChanged(preferences);
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

  bool PACSInteractionScheme = preferences->GetBool("PACS like mouse interaction", false);
  OnInteractionSchemeChanged(PACSInteractionScheme ?
    mitk::InteractionSchemeSwitcher::PACSStandard :
    mitk::InteractionSchemeSwitcher::MITKStandard);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkMxNMultiWidgetEditor::OnLayoutChanged()
{
  FirePropertyChange(berry::IWorkbenchPartConstants::PROP_INPUT);
}

void QmitkMxNMultiWidgetEditor::OnLayoutEditorRequested()
{
  // Toggle: a second press on the requesting button hides an already-visible
  // layout editor instead of re-activating it.
  auto page = this->GetSite()->GetPage();
  if (page.IsNull())
  {
    return;
  }

  const QString viewId = QStringLiteral("org.mitk.views.mxnlayouteditor");
  auto view = page->FindView(viewId);
  if (view.IsNotNull() && page->IsPartVisible(view))
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
