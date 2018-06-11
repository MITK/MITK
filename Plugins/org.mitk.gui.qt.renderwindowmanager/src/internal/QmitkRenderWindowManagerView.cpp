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

// org.mitk.gui.qt.common
#include <QmitkNodeSelectionDialog.h>

const std::string QmitkRenderWindowManagerView::VIEW_ID = "org.mitk.views.renderwindowmanager";

void QmitkRenderWindowManagerView::SetFocus()
{
  // nothing here
}

void QmitkRenderWindowManagerView::CreateQtPartControl(QWidget* parent)
{
  m_Parent = parent;
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

  SetUpConnections();

  OnRenderWindowSelectionChanged(m_Controls.comboBoxRenderWindowSelection->itemText(0));
}

void QmitkRenderWindowManagerView::SetUpConnections()
{
  connect(m_Controls.comboBoxRenderWindowSelection, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(OnRenderWindowSelectionChanged(const QString&)));
  connect(m_RenderWindowManipulatorWidget, SIGNAL(AddLayerButtonClicked()), this, SLOT(OnAddLayerButtonClicked()));

  m_ModelViewSelectionConnector = std::make_unique<QmitkModelViewSelectionConnector>();
  try
  {
    m_ModelViewSelectionConnector->SetView(m_RenderWindowManipulatorWidget->GetLayerStackTableView());
  }
  catch (mitk::Exception& e)
  {
    mitkReThrow(e) << "Cannot connect the model-view pair signals and slots.";
  }
  m_SelectionServiceConnector = std::make_unique<QmitkSelectionServiceConnector>();
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
  QmitkNodeSelectionDialog* dialog = new QmitkNodeSelectionDialog(m_Parent, "Select nodes to add to the render window", "");
  dialog->SetDataStorage(GetDataStorage());
  dialog->SetSelectOnlyVisibleNodes(true);
  dialog->SetSelectionMode(QAbstractItemView::MultiSelection);

  if (QDialog::Accepted == dialog->exec())
  {
    auto nodes = dialog->GetSelectedNodes();
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

  delete dialog;
}

void QmitkRenderWindowManagerView::SetSelectionProvider()
{
  m_SelectionProvider = QmitkDataNodeSelectionProvider::Pointer(new QmitkDataNodeSelectionProvider);
  m_SelectionProvider->SetItemSelectionModel(m_RenderWindowManipulatorWidget->GetLayerStackTableView()->selectionModel());
  GetSite()->SetSelectionProvider(berry::ISelectionProvider::Pointer(m_SelectionProvider));

  // This function is called during the creation of the GUI. It is overridden in this class to create a custom selection provider.
  // This view is used as a selection provider (not used as a selection listener)
  m_SelectionServiceConnector->SetAsSelectionProvider(m_SelectionProvider.GetPointer());
  connect(m_ModelViewSelectionConnector.get(), SIGNAL(CurrentSelectionChanged(QList<mitk::DataNode::Pointer>)), m_SelectionServiceConnector.get(), SLOT(ChangeServiceSelection(QList<mitk::DataNode::Pointer>)));
}

void QmitkRenderWindowManagerView::NodeAdded(const mitk::DataNode* node)
{
  bool global = false;
  node->GetBoolProperty("globalObject_RWM", global);
  if (global)
  {
    // initially insert new point set node into the node list of all render windows
    // the node object of a new point set won't be visible due to its "helper object" property set to true
    m_RenderWindowManipulatorWidget->AddLayerToAllRenderer(const_cast<mitk::DataNode*>(node));
  }
  else
  {
    // initially set new node as invisible in all render windows
    // this way, each single renderer overwrites the common renderer and the node is invisible
    // until it is inserted into the node list of a render windows
    m_RenderWindowManipulatorWidget->HideDataNodeInAllRenderer(node);
  }
}
