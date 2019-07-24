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
#include "QmitkCustomMultiWidget.h"

#include <berryIPreferencesService.h>
#include <berryIPreferences.h>
#include <berryIWorkbenchPage.h>
#include <berryUIException.h>

// mitk qt widgets module
#include <QmitkAbstractMultiWidget.h>
#include <QmitkInteractionSchemeToolBar.h>
#include <QmitkMultiWidgetConfigurationToolBar.h>

// mitk render window manager
#include <mitkRenderWindowViewDirectionController.h>

// qt
#include <QHBoxLayout>

const QString QmitkCustomMultiWidgetEditor::EDITOR_ID = "org.mitk.editors.custommultiwidget";

class QmitkCustomMultiWidgetEditor::Impl final
{

public:

  Impl();

  void SetControlledRenderer();

  QmitkInteractionSchemeToolBar* m_InteractionSchemeToolBar;
  QmitkMultiWidgetConfigurationToolBar* m_ConfigurationToolBar;

  std::unique_ptr<mitk::RenderWindowViewDirectionController> m_RenderWindowViewDirectionController;
};

QmitkCustomMultiWidgetEditor::Impl::Impl()
  : m_InteractionSchemeToolBar(nullptr)
  , m_ConfigurationToolBar(nullptr)
{
  // nothing here
}

void QmitkCustomMultiWidgetEditor::Impl::SetControlledRenderer()
{
  /*
  if (nullptr == m_RenderWindowViewDirectionController || nullptr == GetMultiWidget())
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
  */
}

//////////////////////////////////////////////////////////////////////////
// QmitkCustomMultiWidgetEditor
//////////////////////////////////////////////////////////////////////////
QmitkCustomMultiWidgetEditor::QmitkCustomMultiWidgetEditor()
  : m_Impl(new Impl())
{
  // nothing here
}

QmitkCustomMultiWidgetEditor::~QmitkCustomMultiWidgetEditor() {}

void QmitkCustomMultiWidgetEditor::OnViewDirectionChanged(ViewDirection viewDirection)
{
  m_Impl->m_RenderWindowViewDirectionController->SetViewDirectionOfRenderer(viewDirection);
}

//////////////////////////////////////////////////////////////////////////
// PRIVATE
//////////////////////////////////////////////////////////////////////////
void QmitkCustomMultiWidgetEditor::SetFocus()
{
  const auto& multiWidget = GetMultiWidget();
  if (nullptr != multiWidget)
  {
    multiWidget->setFocus();
  }
}

void QmitkCustomMultiWidgetEditor::CreateQtPartControl(QWidget* parent)
{
  auto multiWidget = GetMultiWidget();
  if (nullptr == multiWidget || nullptr == dynamic_cast<QmitkCustomMultiWidget*>(multiWidget))
  {
    // not multi widget set or multi widget is not of type 'QmitkCustomMultiWidget'
    QHBoxLayout* layout = new QHBoxLayout(parent);
    layout->setContentsMargins(0, 0, 0, 0);

    berry::IBerryPreferences* preferences = dynamic_cast<berry::IBerryPreferences*>(GetPreferences().GetPointer());
    mitk::BaseRenderer::RenderingMode::Type renderingMode = static_cast<mitk::BaseRenderer::RenderingMode::Type>(preferences->GetInt("Rendering Mode", 0));

    multiWidget = new QmitkCustomMultiWidget(parent, 0, 0, renderingMode);
    SetMultiWidget(multiWidget);
    // create left toolbar: interaction scheme toolbar to switch how the render window navigation behaves
    if (nullptr == m_Impl->m_InteractionSchemeToolBar)
    {
      m_Impl->m_InteractionSchemeToolBar = new QmitkInteractionSchemeToolBar(parent);
      layout->addWidget(m_Impl->m_InteractionSchemeToolBar);
    }
    m_Impl->m_InteractionSchemeToolBar->SetInteractionEventHandler(multiWidget->GetInteractionEventHandler());

    // add center widget: the custom multi widget
    layout->addWidget(multiWidget);

    multiWidget->SetDataStorage(GetDataStorage());
    dynamic_cast<QmitkCustomMultiWidget*>(multiWidget)->InitializeMultiWidget();

    // create right toolbar: configuration toolbar to change the render window widget layout
    if (nullptr == m_Impl->m_ConfigurationToolBar)
    {
      m_Impl->m_ConfigurationToolBar = new QmitkMultiWidgetConfigurationToolBar(multiWidget);
      layout->addWidget(m_Impl->m_ConfigurationToolBar);
    }

    connect(m_Impl->m_ConfigurationToolBar, &QmitkMultiWidgetConfigurationToolBar::LayoutSet, this, &QmitkCustomMultiWidgetEditor::OnLayoutChanged);
    connect(m_Impl->m_ConfigurationToolBar, &QmitkMultiWidgetConfigurationToolBar::Synchronized, this, &QmitkCustomMultiWidgetEditor::OnSynchronize);
    connect(m_Impl->m_ConfigurationToolBar, &QmitkMultiWidgetConfigurationToolBar::ViewDirectionChanged, this, &QmitkCustomMultiWidgetEditor::OnViewDirectionChanged);

    //m_Impl->m_MultiWidgetDecorationManager = std::make_unique<QmitkMultiWidgetDecorationManager>(multiWidget);

    m_Impl->m_RenderWindowViewDirectionController = std::make_unique<mitk::RenderWindowViewDirectionController>();
    m_Impl->m_RenderWindowViewDirectionController->SetDataStorage(GetDataStorage());
    m_Impl->SetControlledRenderer();

    OnPreferencesChanged(preferences);
  }
}

void QmitkCustomMultiWidgetEditor::OnPreferencesChanged(const berry::IBerryPreferences* preferences)
{
  const auto& multiWidget = GetMultiWidget();
  if (nullptr == multiWidget)
  {
    return;
  }

  if (multiWidget->GetRenderWindowWidgets().empty())
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
