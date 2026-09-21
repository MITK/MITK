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

namespace
{
  const std::string PACS_INTERACTION_PREFERENCE = "PACS like mouse interaction";
}

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

  QmitkAbstractMultiWidgetEditor::OnInteractionSchemeChanged(scheme);

  // Read back what was actually applied. The multi widget also changes the
  // scheme on its own, for example when a crosshair rotation mode is picked,
  // and this slot is what keeps the toolbars from claiming otherwise.
  const auto appliedScheme = multiWidget->GetInteractionScheme();

  if (nullptr != m_Impl->m_InteractionSchemeToolBar)
  {
    m_Impl->m_InteractionSchemeToolBar->setVisible(QmitkAbstractMultiWidget::IsPACSScheme(appliedScheme));
    m_Impl->m_InteractionSchemeToolBar->SetInteractionScheme(appliedScheme);
  }

  if (nullptr != m_Impl->m_ConfigurationToolBar)
  {
    m_Impl->m_ConfigurationToolBar->SetInteractionScheme(appliedScheme);
  }
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
    // Matches the MITK scheme a fresh multi widget starts with. The preferences
    // below show the toolbar again if PACS mode is on.
    m_Impl->m_InteractionSchemeToolBar->setVisible(false);
    layout->addWidget(m_Impl->m_InteractionSchemeToolBar);

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
    connect(multiWidget, &QmitkAbstractMultiWidget::InteractionSchemeChanged,
      this, &QmitkMxNMultiWidgetEditor::OnInteractionSchemeChanged);
  }

  layout->addWidget(multiWidget);

  // create right toolbar: configuration toolbar to change the render window widget layout
  if (nullptr == m_Impl->m_ConfigurationToolBar)
  {
    m_Impl->m_ConfigurationToolBar = new QmitkMultiWidgetConfigurationToolBar(multiWidget);
    m_Impl->m_ConfigurationToolBar->SetDataStorage(GetDataStorage());
    layout->addWidget(m_Impl->m_ConfigurationToolBar);
  }

  connect(m_Impl->m_ConfigurationToolBar, &QmitkMultiWidgetConfigurationToolBar::LayoutSet,
          this, &QmitkMxNMultiWidgetEditor::OnLayoutSet);
  connect(m_Impl->m_ConfigurationToolBar, &QmitkMultiWidgetConfigurationToolBar::Synchronized,
          this, &QmitkMxNMultiWidgetEditor::OnSynchronize);
  connect(m_Impl->m_ConfigurationToolBar, &QmitkMultiWidgetConfigurationToolBar::InteractionSchemeChanged,
          this, &QmitkMxNMultiWidgetEditor::OnInteractionSchemeChanged);
  connect(m_Impl->m_ConfigurationToolBar, &QmitkMultiWidgetConfigurationToolBar::SetDataBasedLayout,
    static_cast<QmitkMxNMultiWidget*>(GetMultiWidget()), &QmitkMxNMultiWidget::SetDataBasedLayout);
  connect(m_Impl->m_ConfigurationToolBar, &QmitkMultiWidgetConfigurationToolBar::SaveLayout,
    static_cast<QmitkMxNMultiWidget*>(GetMultiWidget()), &QmitkMxNMultiWidget::SaveLayout, Qt::DirectConnection);
  connect(m_Impl->m_ConfigurationToolBar, &QmitkMultiWidgetConfigurationToolBar::LoadLayout,
    static_cast<QmitkMxNMultiWidget*>(GetMultiWidget()), &QmitkMxNMultiWidget::LoadLayout);

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

  // Only a change of the preference itself overrides the interaction scheme,
  // so that the PACS tool or crosshair rotation mode the user picked survives
  // unrelated preference edits.
  const bool pacsInteractionScheme = preferences->GetBool(PACS_INTERACTION_PREFERENCE, false);
  if (pacsInteractionScheme != QmitkAbstractMultiWidget::IsPACSScheme(multiWidget->GetInteractionScheme()))
  {
    OnInteractionSchemeChanged(pacsInteractionScheme ?
      mitk::InteractionSchemeSwitcher::PACSStandard :
      mitk::InteractionSchemeSwitcher::MITKStandard);
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkMxNMultiWidgetEditor::OnLayoutChanged()
{
  FirePropertyChange(berry::IWorkbenchPartConstants::PROP_INPUT);
}
