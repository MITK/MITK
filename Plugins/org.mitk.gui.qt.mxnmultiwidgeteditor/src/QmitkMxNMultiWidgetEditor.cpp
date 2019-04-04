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

const QString QmitkMxNMultiWidgetEditor::EDITOR_ID = "org.mitk.editors.mxnmultiwidget";

class QmitkMxNMultiWidgetEditor::Impl final
{

public:

  Impl();
  ~Impl() = default;

  QmitkMxNMultiWidget* m_MxNMultiWidget;
  QmitkInteractionSchemeToolBar* m_InteractionSchemeToolBar;
  QmitkMultiWidgetConfigurationToolBar* m_ConfigurationToolBar;

  std::unique_ptr<QmitkMultiWidgetDecorationManager> m_MultiWidgetDecorationManager;
};

QmitkMxNMultiWidgetEditor::Impl::Impl()
  : m_MxNMultiWidget(nullptr)
  , m_InteractionSchemeToolBar(nullptr)
  , m_ConfigurationToolBar(nullptr)
{
  // nothing here
}

//////////////////////////////////////////////////////////////////////////
// QmitkMxNMultiWidgetEditor
//////////////////////////////////////////////////////////////////////////
QmitkMxNMultiWidgetEditor::QmitkMxNMultiWidgetEditor()
  : m_Impl(new Impl())
{}

QmitkMxNMultiWidgetEditor::~QmitkMxNMultiWidgetEditor()
{
  GetSite()->GetPage()->RemovePartListener(this);
}

QmitkRenderWindow* QmitkMxNMultiWidgetEditor::GetActiveQmitkRenderWindow() const
{
  if (nullptr != m_Impl->m_MxNMultiWidget)
  {
    auto activeRenderWindowWidget = m_Impl->m_MxNMultiWidget->GetActiveRenderWindowWidget();
    if (nullptr != activeRenderWindowWidget)
    {
      return activeRenderWindowWidget->GetRenderWindow();
    }
  }

  return nullptr;
}

QHash<QString, QmitkRenderWindow*> QmitkMxNMultiWidgetEditor::GetQmitkRenderWindows() const
{
  QHash<QString, QmitkRenderWindow*> result;
  if (nullptr == m_Impl->m_MxNMultiWidget)
  {
    return result;
  }

  result = m_Impl->m_MxNMultiWidget->GetRenderWindows();
  return result;
}

QmitkRenderWindow* QmitkMxNMultiWidgetEditor::GetQmitkRenderWindow(const QString& id) const
{
  if (nullptr == m_Impl->m_MxNMultiWidget)
  {
    return nullptr;
  }

  return m_Impl->m_MxNMultiWidget->GetRenderWindow(id);
}

mitk::Point3D QmitkMxNMultiWidgetEditor::GetSelectedPosition(const QString& id) const
{
  if (nullptr == m_Impl->m_MxNMultiWidget)
  {
    return mitk::Point3D();
  }

  return m_Impl->m_MxNMultiWidget->GetSelectedPosition(id);
}

void QmitkMxNMultiWidgetEditor::SetSelectedPosition(const mitk::Point3D& pos, const QString& id)
{
  if (nullptr != m_Impl->m_MxNMultiWidget)
  {
    m_Impl->m_MxNMultiWidget->SetSelectedPosition(id, pos);
  }
}

void QmitkMxNMultiWidgetEditor::EnableDecorations(bool enable, const QStringList& decorations)
{
  m_Impl->m_MultiWidgetDecorationManager->ShowDecorations(enable, decorations);
}

bool QmitkMxNMultiWidgetEditor::IsDecorationEnabled(const QString& decoration) const
{
  return m_Impl->m_MultiWidgetDecorationManager->IsDecorationVisible(decoration);
}

QStringList QmitkMxNMultiWidgetEditor::GetDecorations() const
{
  return m_Impl->m_MultiWidgetDecorationManager->GetDecorations();
}

berry::IPartListener::Events::Types QmitkMxNMultiWidgetEditor::GetPartEventTypes() const
{
  return Events::CLOSED | Events::OPENED;
}

void QmitkMxNMultiWidgetEditor::PartOpened(const berry::IWorkbenchPartReference::Pointer& partRef)
{
  if (partRef->GetId() == QmitkMxNMultiWidgetEditor::EDITOR_ID)
  {
    m_Impl->m_MxNMultiWidget->ActivateAllCrosshairs(true);
  }
}

void QmitkMxNMultiWidgetEditor::PartClosed(const berry::IWorkbenchPartReference::Pointer& partRef)
{
  if (partRef->GetId() == QmitkMxNMultiWidgetEditor::EDITOR_ID)
  {
    m_Impl->m_MxNMultiWidget->ActivateAllCrosshairs(false);
  }
}

QmitkMxNMultiWidget* QmitkMxNMultiWidgetEditor::GetMxNMultiWidget()
{
  return m_Impl->m_MxNMultiWidget;
}

void QmitkMxNMultiWidgetEditor::OnLayoutSet(int row, int column)
{
  m_Impl->m_MxNMultiWidget->ResetLayout(row, column);
  m_Impl->m_MxNMultiWidget->ActivateAllCrosshairs(true);
  FirePropertyChange(berry::IWorkbenchPartConstants::PROP_INPUT);
}

void QmitkMxNMultiWidgetEditor::OnSynchronize(bool synchronized)
{
  m_Impl->m_MxNMultiWidget->Synchronize(synchronized);
}

//////////////////////////////////////////////////////////////////////////
// PRIVATE
//////////////////////////////////////////////////////////////////////////
void QmitkMxNMultiWidgetEditor::SetFocus()
{
  if (nullptr != m_Impl->m_MxNMultiWidget)
  {
    m_Impl->m_MxNMultiWidget->setFocus();
  }
}

void QmitkMxNMultiWidgetEditor::CreateQtPartControl(QWidget* parent)
{
  if (nullptr == m_Impl->m_MxNMultiWidget)
  {
    QHBoxLayout* layout = new QHBoxLayout(parent);
    layout->setContentsMargins(0, 0, 0, 0);

    berry::IBerryPreferences* preferences = dynamic_cast<berry::IBerryPreferences*>(GetPreferences().GetPointer());
    mitk::BaseRenderer::RenderingMode::Type renderingMode = static_cast<mitk::BaseRenderer::RenderingMode::Type>(preferences->GetInt("Rendering Mode", 0));

    m_Impl->m_MxNMultiWidget = new QmitkMxNMultiWidget(parent, 0, 0, renderingMode);

    // create left toolbar: interaction scheme toolbar to switch how the render window navigation behaves
    if (nullptr == m_Impl->m_InteractionSchemeToolBar)
    {
      m_Impl->m_InteractionSchemeToolBar = new QmitkInteractionSchemeToolBar(parent);
      layout->addWidget(m_Impl->m_InteractionSchemeToolBar);
    }
    m_Impl->m_InteractionSchemeToolBar->SetInteractionEventHandler(m_Impl->m_MxNMultiWidget->GetInteractionEventHandler());

    // add center widget: the mxn multi widget
    layout->addWidget(m_Impl->m_MxNMultiWidget);

    m_Impl->m_MxNMultiWidget->SetDataStorage(GetDataStorage());
    m_Impl->m_MxNMultiWidget->InitializeRenderWindowWidgets();

    // create right toolbar: configuration toolbar to change the render window widget layout
    if (nullptr == m_Impl->m_ConfigurationToolBar)
    {
      m_Impl->m_ConfigurationToolBar = new QmitkMultiWidgetConfigurationToolBar();
      layout->addWidget(m_Impl->m_ConfigurationToolBar);
    }

    connect(m_Impl->m_ConfigurationToolBar, &QmitkMultiWidgetConfigurationToolBar::LayoutSet, this, &QmitkMxNMultiWidgetEditor::OnLayoutSet);
    connect(m_Impl->m_ConfigurationToolBar, &QmitkMultiWidgetConfigurationToolBar::Synchronized, this, &QmitkMxNMultiWidgetEditor::OnSynchronize);

    m_Impl->m_MultiWidgetDecorationManager = std::make_unique<QmitkMultiWidgetDecorationManager>(m_Impl->m_MxNMultiWidget);

    GetSite()->GetPage()->AddPartListener(this);

    OnPreferencesChanged(preferences);
  }
}

void QmitkMxNMultiWidgetEditor::OnPreferencesChanged(const berry::IBerryPreferences* preferences)
{
  if (m_Impl->m_MxNMultiWidget->GetRenderWindowWidgets().empty())
  {
    return;
  }

  // update decoration preferences
  m_Impl->m_MultiWidgetDecorationManager->DecorationPreferencesChanged(preferences);

  // zooming and panning preferences
  bool constrainedZooming = preferences->GetBool("Use constrained zooming and panning", true);
  mitk::RenderingManager::GetInstance()->SetConstrainedPanningZooming(constrainedZooming);

  mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(GetDataStorage());
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}
