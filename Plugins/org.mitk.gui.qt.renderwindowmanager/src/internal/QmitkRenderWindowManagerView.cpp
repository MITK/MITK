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

// blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// mitk core
#include <mitkBaseRenderer.h>
#include <mitkDataNode.h>

const std::string QmitkRenderWindowManagerView::VIEW_ID = "org.mitk.views.renderwindowmanager";

void QmitkRenderWindowManagerView::SetFocus()
{
  // nothing here
}

void QmitkRenderWindowManagerView::CreateQtPartControl(QWidget* parent)
{
  // create GUI widgets
  m_Controls.setupUi(parent);
  // add custom render window manager UI widget to the 'renderWindowManagerTab'
  m_RenderWindowManipulatorWidget = new QmitkRenderWindowManipulatorWidget(GetDataStorage(), parent);
  m_RenderWindowManipulatorWidget->setObjectName(QStringLiteral("m_RenderWindowManipulatorWidget"));
  m_Controls.verticalLayout->addWidget(m_RenderWindowManipulatorWidget);

  SetControlledRenderer();

  for (const auto& renderer : m_ControlledRenderer)
  {
    m_Controls.comboBoxRenderWindowSelection->addItem(renderer->GetName());
  }

  OnRenderWindowSelectionChanged(m_Controls.comboBoxRenderWindowSelection->itemText(0));

  connect(m_Controls.comboBoxRenderWindowSelection, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(OnRenderWindowSelectionChanged(const QString&)));
  connect(m_RenderWindowManipulatorWidget, SIGNAL(AddLayerButtonClicked()), this, SLOT(OnAddLayerButtonClicked()));
}

void QmitkRenderWindowManagerView::SetControlledRenderer()
{
  const mitk::RenderingManager::RenderWindowVector allRegisteredRenderWindows = mitk::RenderingManager::GetInstance()->GetAllRegisteredRenderWindows();
  mitk::BaseRenderer* baseRenderer = nullptr;
  for (const auto &renderWindow : allRegisteredRenderWindows)
  {
    baseRenderer = mitk::BaseRenderer::GetInstance(renderWindow);
    if (nullptr != baseRenderer)
    {
      m_ControlledRenderer.push_back(baseRenderer);
    }
  }

  m_RenderWindowManipulatorWidget->SetControlledRenderer(m_ControlledRenderer);
}

void QmitkRenderWindowManagerView::OnRenderWindowSelectionChanged(const QString &renderWindowId)
{
  m_RenderWindowManipulatorWidget->SetActiveRenderWindow(renderWindowId);
}

void QmitkRenderWindowManagerView::OnAddLayerButtonClicked()
{
  QList<mitk::DataNode::Pointer> nodes = GetDataManagerSelection();
  for (mitk::DataNode* dataNode : nodes)
  {
    if (nullptr != dataNode)
    {
      m_RenderWindowManipulatorWidget->AddLayer(dataNode);

      // get child nodes of the current node
      mitk::DataStorage::SetOfObjects::ConstPointer derivedNodes = GetDataStorage()->GetDerivations(dataNode, nullptr, false);
      for (mitk::DataStorage::SetOfObjects::ConstIterator it = derivedNodes->Begin(); it != derivedNodes->End(); ++it)
      {
        m_RenderWindowManipulatorWidget->AddLayer(it->Value());
      }
    }
  }
}

void QmitkRenderWindowManagerView::NodeAdded(const mitk::DataNode* node)
{
  // initially set new node as invisible in all render windows
  // this way, each single renderer overwrites the common render and the node is invisible
  // until it is inserted in the node list of a render window
  m_RenderWindowManipulatorWidget->HideDataNodeInAllRenderer(node);
}