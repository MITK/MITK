/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

// render window manager plugin
#include "QmitkRenderWindowManagerView.h"

// mitk core
#include <mitkBaseRenderer.h>
#include <mitkDataNode.h>

// qt
#include <QGlobal.h>

const std::string QmitkRenderWindowManagerView::VIEW_ID = "org.mitk.views.renderwindowmanager";

void QmitkRenderWindowManagerView::RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart)
{
  if (m_RenderWindowPart != renderWindowPart)
  {
    m_RenderWindowPart = renderWindowPart;
  }

  SetControlledRenderer();
}

void QmitkRenderWindowManagerView::RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart)
{
  if (m_RenderWindowPart == renderWindowPart)
  {
    m_RenderWindowPart = nullptr;
  }

  SetControlledRenderer();
}

void QmitkRenderWindowManagerView::RenderWindowPartInputChanged(mitk::IRenderWindowPart* renderWindowPart)
{
  if (m_RenderWindowPart == renderWindowPart)
  {
    SetControlledRenderer();
  }
}

void QmitkRenderWindowManagerView::SetFocus()
{
  // nothing here
}

void QmitkRenderWindowManagerView::CreateQtPartControl(QWidget* parent)
{
  // create GUI widgets
  m_Controls.setupUi(parent);

  // add custom render window manager UI widget to the 'renderWindowManagerTab'
  m_RenderWindowInspector = new QmitkDataStorageRenderWindowInspector(parent);
  m_RenderWindowInspector->SetDataStorage(GetDataStorage());
  m_RenderWindowInspector->setObjectName(QStringLiteral("m_RenderWindowManipulatorWidget"));
  m_Controls.verticalLayout->addWidget(m_RenderWindowInspector);

  // data node context menu and menu actions
  m_InspectorView = m_RenderWindowInspector->GetView();
  m_DataNodeContextMenu = new QmitkDataNodeContextMenu(GetSite(), m_InspectorView);
  m_DataNodeContextMenu->SetDataStorage(GetDataStorage());
  //m_DataNodeContextMenu->SetSurfaceDecimation(m_SurfaceDecimation);

   // connect objects
  connect(m_Controls.comboBoxRenderWindowSelection, QOverload<const QString&>::of(&QComboBox::currentIndexChanged), this, &QmitkRenderWindowManagerView::OnRenderWindowSelectionChanged);
  connect(m_InspectorView, &QAbstractItemView::customContextMenuRequested, m_DataNodeContextMenu, &QmitkDataNodeContextMenu::OnContextMenuRequested);

  m_RenderWindowPart = GetRenderWindowPart();
  // also sets the controlled renderer
  RenderWindowPartActivated(m_RenderWindowPart);
}

void QmitkRenderWindowManagerView::SetControlledRenderer()
{
  const mitk::RenderingManager::RenderWindowVector allRegisteredRenderWindows = mitk::RenderingManager::GetInstance()->GetAllRegisteredRenderWindows();
  mitk::BaseRenderer* baseRenderer = nullptr;
  mitk::RenderWindowLayerUtilities::RendererVector controlledRenderer;
  QStringList rendererNames;
  for (const auto& renderWindow : allRegisteredRenderWindows)
  {
    baseRenderer = mitk::BaseRenderer::GetInstance(renderWindow);
    if (nullptr != baseRenderer)
    {
      controlledRenderer.push_back(baseRenderer);
      rendererNames.append(baseRenderer->GetName());
    }
  }

  m_RenderWindowInspector->SetControlledRenderer(controlledRenderer);

  m_Controls.comboBoxRenderWindowSelection->clear();
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
}

QItemSelectionModel* QmitkRenderWindowManagerView::GetDataNodeSelectionModel() const
{
  return m_InspectorView->selectionModel();
}
