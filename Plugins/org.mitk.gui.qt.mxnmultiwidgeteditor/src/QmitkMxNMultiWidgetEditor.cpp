/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical Image Computing.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "QmitkMxNMultiWidgetEditor.h"

#include <berryIPreferencesService.h>
#include <berryIPreferences.h>
#include <berryIWorkbenchPage.h>
#include <berryIWorkbenchPartConstants.h>
#include <berryUIException.h>

// mxn multi widget editor plugin
#include "QmitkMultiWidgetDecorationManager.h"

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
  : m_Impl(std::make_unique<Impl>())
{}

QmitkMxNMultiWidgetEditor::~QmitkMxNMultiWidgetEditor()
{
  GetSite()->GetPage()->RemovePartListener(this);
}

void QmitkMxNMultiWidgetEditor::OnLayoutSet(int row, int column)
{
  const auto& multiWidget = dynamic_cast<QmitkMxNMultiWidget*>(GetMultiWidget());
  if (nullptr != multiWidget)
  {
    multiWidget->SetLayout(row, column);
    multiWidget->SetCrosshairVisibility(true);
    FirePropertyChange(berry::IWorkbenchPartConstants::PROP_INPUT);
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
  auto multiWidget = GetMultiWidget();
  if (nullptr == multiWidget)
  {
    QHBoxLayout* layout = new QHBoxLayout(parent);
    layout->setContentsMargins(0, 0, 0, 0);

    berry::IBerryPreferences* preferences = dynamic_cast<berry::IBerryPreferences*>(GetPreferences().GetPointer());
    mitk::BaseRenderer::RenderingMode::Type renderingMode = static_cast<mitk::BaseRenderer::RenderingMode::Type>(preferences->GetInt("Rendering Mode", 0));

    multiWidget = new QmitkMxNMultiWidget(parent, 0, nullptr, renderingMode);

    // create left toolbar: interaction scheme toolbar to switch how the render window navigation behaves
    if (nullptr == m_Impl->m_InteractionSchemeToolBar)
    {
      m_Impl->m_InteractionSchemeToolBar = new QmitkInteractionSchemeToolBar(parent);
      layout->addWidget(m_Impl->m_InteractionSchemeToolBar);
    }
    m_Impl->m_InteractionSchemeToolBar->SetInteractionEventHandler(multiWidget->GetInteractionEventHandler());

    // add center widget: the mxn multi widget
    layout->addWidget(multiWidget);

    multiWidget->SetDataStorage(GetDataStorage());
    multiWidget->InitializeMultiWidget();
    SetMultiWidget(multiWidget);

    // create right toolbar: configuration toolbar to change the render window widget layout
    if (nullptr == m_Impl->m_ConfigurationToolBar)
    {
      m_Impl->m_ConfigurationToolBar = new QmitkMultiWidgetConfigurationToolBar(multiWidget);
      layout->addWidget(m_Impl->m_ConfigurationToolBar);
    }

    connect(m_Impl->m_ConfigurationToolBar, &QmitkMultiWidgetConfigurationToolBar::LayoutSet, this, &QmitkMxNMultiWidgetEditor::OnLayoutSet);
    connect(m_Impl->m_ConfigurationToolBar, &QmitkMultiWidgetConfigurationToolBar::Synchronized, this, &QmitkMxNMultiWidgetEditor::OnSynchronize);

    //m_Impl->m_MultiWidgetDecorationManager = std::make_unique<QmitkMultiWidgetDecorationManager>(multiWidget);

    GetSite()->GetPage()->AddPartListener(this);

    OnPreferencesChanged(preferences);
  }
}

void QmitkMxNMultiWidgetEditor::OnPreferencesChanged(const berry::IBerryPreferences* preferences)
{
  const auto& multiWidget = GetMultiWidget();
  if (nullptr == multiWidget)
  {
    return;
  }

  // update decoration preferences
  //m_Impl->m_MultiWidgetDecorationManager->DecorationPreferencesChanged(preferences);

  // zooming and panning preferences
  bool constrainedZooming = preferences->GetBool("Use constrained zooming and panning", true);
  mitk::RenderingManager::GetInstance()->SetConstrainedPanningZooming(constrainedZooming);

  mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(GetDataStorage());
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}
