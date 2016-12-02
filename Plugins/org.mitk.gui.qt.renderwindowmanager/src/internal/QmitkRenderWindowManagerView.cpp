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

// mitk
#include <mitkBaseRenderer.h>

// qt
#include <QMessageBox>
#include <QSignalMapper>
#include <QStringList>

const std::string QmitkRenderWindowManagerView::VIEW_ID = "org.mitk.views.renderwindowmanager";

void QmitkRenderWindowManagerView::SetFocus()
{
  m_Controls.radioButtonAxial->setFocus();
}

void QmitkRenderWindowManagerView::CreateQtPartControl(QWidget* parent)
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Parent = parent;
  m_Controls.setupUi(m_Parent);

  // initialize the render window layer controller and set the controller renderer (in constructor) and the data storage 
  m_RenderWindowLayerController = new mitk::RenderWindowLayerController();
  m_RenderWindowLayerController->SetDataStorage(GetDataStorage());
  mitk::RenderWindowLayerController::RendererVector controlledRenderer = m_RenderWindowLayerController->GetControlledRenderer();
  for (const auto& renderer : controlledRenderer)
  {
    m_Controls.comboBoxRenderWindowSelection->addItem(renderer->GetName());
  }

  m_CurrentRendererName = m_Controls.comboBoxRenderWindowSelection->itemText(0);

  // create a new model
  m_RenderWindowModel.insert(m_CurrentRendererName, new QmitkRenderWindowDataModel(this));
  m_Controls.renderWindowTableView->setModel(m_RenderWindowModel.value(m_CurrentRendererName));
  m_Controls.renderWindowTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_Controls.renderWindowTableView->horizontalHeader()->setStretchLastSection(true);

  m_RenderWindowModel.value(m_CurrentRendererName)->SetDataStorage(GetDataStorage());
  //m_VisibilityDelegate = new QmitkVisibilityDelegate(this);
  //m_Controls.renderWindowTableView->setItemDelegateForColumn(0, m_VisibilityDelegate);

  mitk::BaseRenderer* renderer = mitk::BaseRenderer::GetByName(m_CurrentRendererName.toStdString());
  mitk::RenderWindowLayerController::LayerStack layerStack = m_RenderWindowLayerController->GetLayerStack(renderer, true);
  m_RenderWindowModel.value(m_CurrentRendererName)->UpdateTableForRenderWindow(layerStack);

  m_Controls.radioButtonAxial->setChecked(true);

  m_AddDataNodeWidget = new QmitkLayerManagerAddNodeWidget(m_Parent);
  m_AddDataNodeWidget->hide();
  m_AddDataNodeWidget->SetDataStorage(GetDataStorage());

  SetUpConnections();
}

void QmitkRenderWindowManagerView::SetUpConnections(void)
{
  connect(m_Controls.comboBoxRenderWindowSelection, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(OnRenderWindowSelectionChanged(const QString&)));

  QSignalMapper* udSignalMapper = new QSignalMapper(this);
  udSignalMapper->setMapping(m_Controls.pushButtonMoveUp, QString("up"));
  udSignalMapper->setMapping(m_Controls.pushButtonMoveDown, QString("down"));

  connect(m_Controls.pushButtonMoveUp, SIGNAL(clicked()), udSignalMapper, SLOT(map()));
  connect(m_Controls.pushButtonMoveDown, SIGNAL(clicked()), udSignalMapper, SLOT(map()));
  connect(udSignalMapper, SIGNAL(mapped(const QString&)), this, SLOT(MoveDataNode(const QString&)));

  connect(m_Controls.pushButtonAddNode, SIGNAL(clicked()), this, SLOT(ShowAddDataNodeWidget()));
  connect(m_Controls.pushButtonRemoveNode, SIGNAL(clicked()), this, SLOT(RemoveDataNode()));
  connect(m_Controls.pushButtonSetAsBaseNode, SIGNAL(clicked()), this, SLOT(SetAsBaseDataNode()));

  //connect(m_Controls.radioButtonAxial, SIGNAL(clicked()), this, SLOT(DoImageProcessing()));
  //connect(m_Controls.radioButtonCoronal, SIGNAL(clicked()), this, SLOT(DoImageProcessing()));
  //connect(m_Controls.radioButtonSagittal, SIGNAL(clicked()), this, SLOT(DoImageProcessing()));

  connect(m_AddDataNodeWidget, SIGNAL(NodeToAddSelected(mitk::DataNode*)), this, SLOT(AddDataNode(mitk::DataNode*)));
}

void QmitkRenderWindowManagerView::OnRenderWindowSelectionChanged(const QString &renderWindowId)
{
  m_CurrentRendererName = renderWindowId;
  if (!m_RenderWindowModel.contains(m_CurrentRendererName))
  {
    m_RenderWindowModel.insert(m_CurrentRendererName, new QmitkRenderWindowDataModel(this));
    m_RenderWindowModel.value(m_CurrentRendererName)->SetDataStorage(GetDataStorage());
  }

  m_Controls.renderWindowTableView->setModel(m_RenderWindowModel.value(m_CurrentRendererName));

  mitk::BaseRenderer* renderer = mitk::BaseRenderer::GetByName(m_CurrentRendererName.toStdString());
  mitk::RenderWindowLayerController::LayerStack layerStack = m_RenderWindowLayerController->GetLayerStack(renderer, true);

  m_RenderWindowModel.value(m_CurrentRendererName)->UpdateTableForRenderWindow(layerStack);
}

void QmitkRenderWindowManagerView::ShowAddDataNodeWidget()
{
  m_AddDataNodeWidget->ListDataNodes();
  m_AddDataNodeWidget->show();
}

void QmitkRenderWindowManagerView::AddDataNode(mitk::DataNode* dataNode)
{
  mitk::BaseRenderer* selectedRenderer = mitk::BaseRenderer::GetByName(m_CurrentRendererName.toStdString());
  m_RenderWindowLayerController->InsertLayerNode(dataNode, -1, selectedRenderer);

  m_RenderWindowModel.value(m_CurrentRendererName)->UpdateTableForRenderWindow(m_RenderWindowLayerController->GetLayerStack(selectedRenderer, true));
}

void QmitkRenderWindowManagerView::RemoveDataNode()
{
  // TODO: not yet implemented
}

void QmitkRenderWindowManagerView::SetAsBaseDataNode()
{
  // TODO: not yet implemented
}

void QmitkRenderWindowManagerView::MoveDataNode(const QString &direction)
{
  QModelIndex selectedIndex = m_Controls.renderWindowTableView->currentIndex();
  if (selectedIndex.isValid())
  {
    QVariant rowData = m_RenderWindowModel.value(m_CurrentRendererName)->data(selectedIndex, Qt::DisplayRole);

    mitk::DataNode* dataNode = GetDataStorage()->GetNamedNode(rowData.toString().toStdString());
    mitk::BaseRenderer* selectedRenderer = mitk::BaseRenderer::GetByName(m_CurrentRendererName.toStdString());
    if ("up" == direction)
    {
      m_RenderWindowLayerController->MoveNodeUp(dataNode, selectedRenderer);
    }
    else
    {
      m_RenderWindowLayerController->MoveNodeDown(dataNode, selectedRenderer);

    }

    m_RenderWindowModel.value(m_CurrentRendererName)->UpdateTableForRenderWindow(m_RenderWindowLayerController->GetLayerStack(selectedRenderer, true));
  }
}
