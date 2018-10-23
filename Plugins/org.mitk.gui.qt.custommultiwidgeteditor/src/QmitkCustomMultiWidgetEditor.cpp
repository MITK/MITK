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

#include "QmitkCustomMultiWidgetEditor.h"

#include <berryUIException.h>
#include <berryIWorkbenchPage.h>
#include <berryIPreferencesService.h>
#include <berryIPreferences.h>

#include <QWidget>

// mitk core
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>

const QString QmitkCustomMultiWidgetEditor::EDITOR_ID = "org.mitk.editors.custommultiwidget";

QmitkCustomMultiWidgetEditor::QmitkCustomMultiWidgetEditor()
  : m_CustomMultiWidget(nullptr)
  , m_InteractionSchemeToolBar(nullptr)
  , m_ConfigurationToolBar(nullptr)
{
  // nothing here
}

QmitkCustomMultiWidgetEditor::~QmitkCustomMultiWidgetEditor()
{
  // nothing here
}

void QmitkCustomMultiWidgetEditor::Activated()
{
  // nothing here
}

void QmitkCustomMultiWidgetEditor::Deactivated()
{
  // nothing here
}

void QmitkCustomMultiWidgetEditor::Visible()
{
  // nothing here
}

void QmitkCustomMultiWidgetEditor::Hidden()
{
  // nothing here
}

QmitkRenderWindow* QmitkCustomMultiWidgetEditor::GetActiveQmitkRenderWindow() const
{
  if (nullptr != m_CustomMultiWidget)
  {
    auto activeRenderWindowWidget = m_CustomMultiWidget->GetActiveRenderWindowWidget();
    if (nullptr != activeRenderWindowWidget)
    {
      return activeRenderWindowWidget->GetRenderWindow();
    }
  }

  return nullptr;
}

QHash<QString, QmitkRenderWindow*> QmitkCustomMultiWidgetEditor::GetQmitkRenderWindows() const
{
  QHash<QString, QmitkRenderWindow*> result;
  if (nullptr == m_CustomMultiWidget)
  {
    return result;
  }

  // create QHash on demand
  QmitkCustomMultiWidget::RenderWindowWidgetMap renderWindowWidgets = m_CustomMultiWidget->GetRenderWindowWidgets();
  for (const auto& renderWindowWidget : renderWindowWidgets)
  {
    result.insert(renderWindowWidget.first, renderWindowWidget.second->GetRenderWindow());
  }
  return result;
}

QmitkRenderWindow* QmitkCustomMultiWidgetEditor::GetQmitkRenderWindow(const QString& id) const
{
  if (nullptr == m_CustomMultiWidget)
  {
    return nullptr;
  }

  return m_CustomMultiWidget->GetRenderWindow(id);
}

mitk::Point3D QmitkCustomMultiWidgetEditor::GetSelectedPosition(const QString& id) const
{
  if (nullptr == m_CustomMultiWidget)
  {
    return mitk::Point3D();
  }

  return m_CustomMultiWidget->GetCrossPosition(id);
}

void QmitkCustomMultiWidgetEditor::SetSelectedPosition(const mitk::Point3D& pos, const QString& id)
{
  if (nullptr != m_CustomMultiWidget)
  {
    m_CustomMultiWidget->MoveCrossToPosition(id, pos);
  }
}

void QmitkCustomMultiWidgetEditor::EnableDecorations(bool enable, const QStringList& decorations)
{
  m_MultiWidgetDecorationManager->ShowDecorations(enable, decorations);
}

bool QmitkCustomMultiWidgetEditor::IsDecorationEnabled(const QString& decoration) const
{
  return m_MultiWidgetDecorationManager->IsDecorationVisible(decoration);
}

QStringList QmitkCustomMultiWidgetEditor::GetDecorations() const
{
  return m_MultiWidgetDecorationManager->GetDecorations();
}

void QmitkCustomMultiWidgetEditor::EnableSlicingPlanes(bool enable)
{
  // #TODO: nothing here
}

bool QmitkCustomMultiWidgetEditor::IsSlicingPlanesEnabled() const
{
  // #TODO: nothing here
  return false;
}

QmitkCustomMultiWidget* QmitkCustomMultiWidgetEditor::GetCustomMultiWidget()
{
  return m_CustomMultiWidget;
}

void QmitkCustomMultiWidgetEditor::OnLayoutSet(int row, int column)
{
  m_CustomMultiWidget->ResetLayout(row, column);
  SetControlledRenderer();
}

void QmitkCustomMultiWidgetEditor::OnSynchronize(bool synchronized)
{
  m_CustomMultiWidget->Synchronize(synchronized);
}

void QmitkCustomMultiWidgetEditor::OnViewDirectionChanged(mitk::SliceNavigationController::ViewDirection viewDirection)
{
  m_RenderWindowViewDirectionController->SetViewDirectionOfRenderer(viewDirection);
}

//////////////////////////////////////////////////////////////////////////
// PRIVATE
//////////////////////////////////////////////////////////////////////////
void QmitkCustomMultiWidgetEditor::SetFocus()
{
  if (nullptr != m_CustomMultiWidget)
  {
    m_CustomMultiWidget->setFocus();
  }
}

void QmitkCustomMultiWidgetEditor::CreateQtPartControl(QWidget* parent)
{
  if (nullptr == m_CustomMultiWidget)
  {
    QHBoxLayout* layout = new QHBoxLayout(parent);
    layout->setContentsMargins(0, 0, 0, 0);

    berry::IBerryPreferences* preferences = dynamic_cast<berry::IBerryPreferences*>(GetPreferences().GetPointer());
    mitk::BaseRenderer::RenderingMode::Type renderingMode = static_cast<mitk::BaseRenderer::RenderingMode::Type>(preferences->GetInt("Rendering Mode", 0));

    m_CustomMultiWidget = new QmitkCustomMultiWidget(parent, 0, 0, renderingMode);

    // create left toolbar: interaction scheme toolbar to switch how the render window navigation behaves
    if (nullptr == m_InteractionSchemeToolBar)
    {
      m_InteractionSchemeToolBar = new QmitkInteractionSchemeToolBar(parent);
      layout->addWidget(m_InteractionSchemeToolBar);
    }
    m_InteractionSchemeToolBar->SetInteractionEventHandler(m_CustomMultiWidget->GetInteractionEventHandler());

    // add center widget: the custom multi widget
    layout->addWidget(m_CustomMultiWidget);

    m_CustomMultiWidget->SetDataStorage(GetDataStorage());
    m_CustomMultiWidget->InitializeRenderWindowWidgets();

    // create right toolbar: configuration toolbar to change the render window widget layout
    if (nullptr == m_ConfigurationToolBar)
    {
      m_ConfigurationToolBar = new QmitkMultiWidgetConfigurationToolBar(m_CustomMultiWidget);
      layout->addWidget(m_ConfigurationToolBar);
    }

    connect(m_ConfigurationToolBar, SIGNAL(LayoutSet(int, int)), SLOT(OnLayoutSet(int, int)));
    connect(m_ConfigurationToolBar, SIGNAL(Synchronized(bool)), SLOT(OnSynchronize(bool)));
    connect(m_ConfigurationToolBar, SIGNAL(ViewDirectionChanged(mitk::SliceNavigationController::ViewDirection)), SLOT(OnViewDirectionChanged(mitk::SliceNavigationController::ViewDirection)));

    m_MultiWidgetDecorationManager = std::make_unique<QmitkMultiWidgetDecorationManager>(m_CustomMultiWidget);

    m_RenderWindowViewDirectionController = std::make_unique<mitk::RenderWindowViewDirectionController>();
    m_RenderWindowViewDirectionController->SetDataStorage(GetDataStorage());
    SetControlledRenderer();

    OnPreferencesChanged(preferences);
  }
}

void QmitkCustomMultiWidgetEditor::OnPreferencesChanged(const berry::IBerryPreferences* preferences)
{
  if (m_CustomMultiWidget->GetRenderWindowWidgets().empty())
  {
    return;
  }
  // update decoration preferences
  m_MultiWidgetDecorationManager->DecorationPreferencesChanged(preferences);

  // zooming and panning preferences
  bool constrainedZooming = preferences->GetBool("Use constrained zooming and panning", true);
  mitk::RenderingManager::GetInstance()->SetConstrainedPanningZooming(constrainedZooming);

  mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(GetDataStorage());
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkCustomMultiWidgetEditor::SetControlledRenderer()
{
  if (nullptr == m_RenderWindowViewDirectionController || nullptr == m_CustomMultiWidget)
  {
    return;
  }

  RenderWindowLayerUtilities::RendererVector controlledRenderer;
  auto renderWindowWidgets = m_CustomMultiWidget->GetRenderWindowWidgets();
  for (const auto& renderWindowWidget : renderWindowWidgets)
  {
    auto renderWindow = renderWindowWidget.second->GetRenderWindow();
    auto vtkRenderWindow = renderWindow->GetRenderWindow();
    mitk::BaseRenderer* baseRenderer = mitk::BaseRenderer::GetInstance(vtkRenderWindow);
    if (nullptr != baseRenderer)
    {
      controlledRenderer.push_back(baseRenderer);
    }
  }

  m_RenderWindowViewDirectionController->SetControlledRenderer(controlledRenderer);
}
