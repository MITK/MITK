/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// render window manager plugin
#include "QmitkRenderWindowManagerView.h"

// mitk core
#include <mitkBaseRenderer.h>
#include <mitkDataNode.h>
#include <QmitkRenderWindow.h>

// mitk qt widgets
#include <QmitkAbstractMultiWidget.h>
#include <QmitkRenderWindowWidget.h>

// mitk gui qt common plugin
#include <QmitkAbstractMultiWidgetEditor.h>

const std::string QmitkRenderWindowManagerView::VIEW_ID = "org.mitk.views.renderwindowmanager";

void QmitkRenderWindowManagerView::RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart)
{
  if (m_RenderWindowPart != renderWindowPart)
  {
    m_RenderWindowPart = renderWindowPart;
    this->SetControlledRenderer();

    // if the render window part is an abstract multi widget editor we can receive the abstract multi widget and listen to the signal
    auto abstractMultiWidgetEditor = dynamic_cast<QmitkAbstractMultiWidgetEditor*>(m_RenderWindowPart);
    if (nullptr != abstractMultiWidgetEditor)
    {
      connect(abstractMultiWidgetEditor->GetMultiWidget(), &QmitkAbstractMultiWidget::ActiveRenderWindowChanged,
              this, &QmitkRenderWindowManagerView::RenderWindowChanged);
    }
  }
}

void QmitkRenderWindowManagerView::RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart)
{
  if (m_RenderWindowPart == renderWindowPart)
  {
    // if the render window part is an abstract multi widget editor we need to disconnect the signal before release the render window part
    auto abstractMultiWidgetEditor = dynamic_cast<QmitkAbstractMultiWidgetEditor*>(m_RenderWindowPart);
    if (nullptr != abstractMultiWidgetEditor)
    {
      disconnect(abstractMultiWidgetEditor->GetMultiWidget(), &QmitkAbstractMultiWidget::ActiveRenderWindowChanged,
                 this, &QmitkRenderWindowManagerView::RenderWindowChanged);
    }

    m_RenderWindowPart = nullptr;
    this->SetControlledRenderer();
  }
}

void QmitkRenderWindowManagerView::RenderWindowPartInputChanged(mitk::IRenderWindowPart* renderWindowPart)
{
  if (m_RenderWindowPart == renderWindowPart)
  {
    this->SetControlledRenderer();
  }
}

void QmitkRenderWindowManagerView::CreateQtPartControl(QWidget* parent)
{
  m_Parent = parent;
  // create GUI widgets
  m_Controls.setupUi(parent);

  // add custom render window manager UI widget to the 'renderWindowManagerTab'
  m_RenderWindowInspector = new QmitkRenderWindowDataStorageInspector(parent);
  m_RenderWindowInspector->SetDataStorage(GetDataStorage());
  m_RenderWindowInspector->setObjectName(QStringLiteral("m_RenderWindowManipulatorWidget"));
  m_Controls.verticalLayout->addWidget(m_RenderWindowInspector);

  // data node context menu and menu actions
  m_InspectorView = m_RenderWindowInspector->GetView();
  m_DataNodeContextMenu = new QmitkDataNodeContextMenu(GetSite(), m_InspectorView);
  m_DataNodeContextMenu->SetDataStorage(GetDataStorage());
  //m_DataNodeContextMenu->SetSurfaceDecimation(m_SurfaceDecimation);

  // connect objects
  connect(m_Controls.comboBoxRenderWindowSelection, static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged),
          this, &QmitkRenderWindowManagerView::OnRenderWindowSelectionChanged);
  connect(m_InspectorView, &QAbstractItemView::customContextMenuRequested,
          m_DataNodeContextMenu, &QmitkDataNodeContextMenu::OnContextMenuRequested);

  auto renderWindowPart = GetRenderWindowPart();
  if (nullptr != renderWindowPart)
  {
    this->RenderWindowPartActivated(renderWindowPart);
  }

  // also sets the controlled renderer
  this->SetControlledRenderer();
}

void QmitkRenderWindowManagerView::SetControlledRenderer()
{
  QHash<QString, QmitkRenderWindow*> renderWindows;
  if (nullptr != m_RenderWindowPart)
  {
    renderWindows = m_RenderWindowPart->GetQmitkRenderWindows();
  }

  mitk::RenderWindowLayerUtilities::RendererVector controlledRenderer;
  QStringList rendererNames;
  m_Controls.comboBoxRenderWindowSelection->clear();
  mitk::BaseRenderer* baseRenderer = nullptr;
  for (const auto& renderWindow : renderWindows.values())
  {
    baseRenderer = mitk::BaseRenderer::GetInstance(renderWindow->GetVtkRenderWindow());
    if (nullptr != baseRenderer)
    {
      controlledRenderer.push_back(baseRenderer);
      rendererNames.append(baseRenderer->GetName());
    }
  }

  m_RenderWindowInspector->SetControlledRenderer(controlledRenderer);
  rendererNames.sort();
  m_Controls.comboBoxRenderWindowSelection->addItems(rendererNames);
}

void QmitkRenderWindowManagerView::OnRenderWindowSelectionChanged(const QString& renderWindowId)
{
  m_RenderWindowInspector->SetActiveRenderWindow(renderWindowId);
  mitk::BaseRenderer* selectedRenderer = mitk::BaseRenderer::GetByName(renderWindowId.toStdString());
  if (nullptr != selectedRenderer)
  {
    m_DataNodeContextMenu->SetBaseRenderer(selectedRenderer);
  }

  // if the render window part is an abstract multi widget editor we can set the active render window
  auto abstractMultiWidgetEditor = dynamic_cast<QmitkAbstractMultiWidgetEditor*>(m_RenderWindowPart);
  if (nullptr != abstractMultiWidgetEditor)
  {

    auto renderWindowWidget = abstractMultiWidgetEditor->GetMultiWidget()->GetRenderWindowWidget(renderWindowId);
    abstractMultiWidgetEditor->GetMultiWidget()->SetActiveRenderWindowWidget(renderWindowWidget);
  }
}

void QmitkRenderWindowManagerView::RenderWindowChanged()
{
  auto abstractMultiWidget = dynamic_cast<QmitkAbstractMultiWidget*>(sender());
  if (nullptr != abstractMultiWidget)
  {
    auto activeRenderWindowWidget = abstractMultiWidget->GetActiveRenderWindowWidget();
    if (nullptr != activeRenderWindowWidget)
    {
      m_Controls.comboBoxRenderWindowSelection->setCurrentText(activeRenderWindowWidget->GetWidgetName());
    }
  }
}

QItemSelectionModel* QmitkRenderWindowManagerView::GetDataNodeSelectionModel() const
{
  return m_InspectorView->selectionModel();
}
