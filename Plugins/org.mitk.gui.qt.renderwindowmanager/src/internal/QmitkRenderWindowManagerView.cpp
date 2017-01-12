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

  // initialize the render window layer controller and the render window view direction controller
  // and set the controller renderer (in constructor) and the data storage 
  m_RenderWindowLayerController = std::make_unique<mitk::RenderWindowLayerController>();
  m_RenderWindowViewDirectionController = std::make_unique<mitk::RenderWindowViewDirectionController>();
  m_RenderWindowLayerController->SetDataStorage(GetDataStorage());
  m_RenderWindowViewDirectionController->SetDataStorage(GetDataStorage());
  mitk::RenderWindowLayerController::RendererVector controlledRenderer = m_RenderWindowLayerController->GetControlledRenderer();
  for (const auto& renderer : controlledRenderer)
  {
    m_Controls.comboBoxRenderWindowSelection->addItem(renderer->GetName());
  }

  // create a new model
  m_RenderWindowDataModel = std::make_unique<QmitkRenderWindowDataModel>(this);
  m_RenderWindowDataModel->SetDataStorage(GetDataStorage());
  m_RenderWindowDataModel->SetCurrentRenderer(m_Controls.comboBoxRenderWindowSelection->itemText(0).toStdString());

  m_Controls.renderWindowTableView->setModel(m_RenderWindowDataModel.get());
  m_Controls.renderWindowTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_Controls.renderWindowTableView->horizontalHeader()->setStretchLastSection(true);
  m_Controls.renderWindowTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_Controls.renderWindowTableView->setSelectionMode(QAbstractItemView::SingleSelection);
  //m_VisibilityDelegate = new QmitkVisibilityDelegate(this);
  //m_Controls.renderWindowTableView->setItemDelegateForColumn(0, m_VisibilityDelegate);

  m_AddLayerWidget = new QmitkLayerManagerAddLayerWidget(m_Parent);
  m_AddLayerWidget->hide();
  m_AddLayerWidget->SetDataStorage(GetDataStorage());

  SetUpConnections();
}

void QmitkRenderWindowManagerView::SetUpConnections(void)
{
  connect(m_Controls.comboBoxRenderWindowSelection, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(OnRenderWindowSelectionChanged(const QString&)));

  connect(m_Controls.pushButtonAddLayer, SIGNAL(clicked()), this, SLOT(ShowAddLayerWidget()));
  connect(m_Controls.pushButtonRemoveLayer, SIGNAL(clicked()), this, SLOT(RemoveLayer()));
  connect(m_Controls.pushButtonSetAsBaseLayer, SIGNAL(clicked()), this, SLOT(SetAsBaseLayer()));

  QSignalMapper* udSignalMapper = new QSignalMapper(this);
  udSignalMapper->setMapping(m_Controls.pushButtonMoveUp, QString("up"));
  udSignalMapper->setMapping(m_Controls.pushButtonMoveDown, QString("down"));
  connect(udSignalMapper, SIGNAL(mapped(const QString&)), this, SLOT(MoveLayer(const QString&)));
  connect(m_Controls.pushButtonMoveUp, SIGNAL(clicked()), udSignalMapper, SLOT(map()));
  connect(m_Controls.pushButtonMoveDown, SIGNAL(clicked()), udSignalMapper, SLOT(map()));

  connect(m_Controls.pushButtonResetRenderer, SIGNAL(clicked()), this, SLOT(ResetRenderer()));
  connect(m_Controls.pushButtonClearRenderer, SIGNAL(clicked()), this, SLOT(ClearRenderer()));

  QSignalMapper* changeViewDirectionSignalMapper = new QSignalMapper(this);
  changeViewDirectionSignalMapper->setMapping(m_Controls.radioButtonAxial, QString("axial"));
  changeViewDirectionSignalMapper->setMapping(m_Controls.radioButtonCoronal, QString("coronal"));
  changeViewDirectionSignalMapper->setMapping(m_Controls.radioButtonSagittal, QString("sagittal"));
  connect(changeViewDirectionSignalMapper, SIGNAL(mapped(const QString&)), this, SLOT(ChangeViewDirection(const QString&)));
  connect(m_Controls.radioButtonAxial, SIGNAL(clicked()), changeViewDirectionSignalMapper, SLOT(map()));
  connect(m_Controls.radioButtonCoronal, SIGNAL(clicked()), changeViewDirectionSignalMapper, SLOT(map()));
  connect(m_Controls.radioButtonSagittal, SIGNAL(clicked()), changeViewDirectionSignalMapper, SLOT(map()));

  connect(m_AddLayerWidget, SIGNAL(LayerToAddSelected(mitk::DataNode*)), this, SLOT(AddLayer(mitk::DataNode*)));
}

void QmitkRenderWindowManagerView::OnRenderWindowSelectionChanged(const QString &renderWindowId)
{
  std::string currentRendererName = renderWindowId.toStdString();
  m_RenderWindowDataModel->SetCurrentRenderer(currentRendererName);

  mitk::BaseRenderer* selectedRenderer = mitk::BaseRenderer::GetByName(currentRendererName);
  mitk::SliceNavigationController::ViewDirection viewDirection = selectedRenderer->GetSliceNavigationController()->GetDefaultViewDirection();
  switch (viewDirection)
  {
  case mitk::SliceNavigationController::Axial:
    m_Controls.radioButtonAxial->setChecked(true);
    break;
  case mitk::SliceNavigationController::Frontal:
    m_Controls.radioButtonCoronal->setChecked(true);
    break;
  case mitk::SliceNavigationController::Sagittal:
    m_Controls.radioButtonSagittal->setChecked(true);
    break;
  default:
    break;
  }
}

void QmitkRenderWindowManagerView::ShowAddLayerWidget()
{
  m_AddLayerWidget->ListLayer();
  m_AddLayerWidget->show();
}

void QmitkRenderWindowManagerView::AddLayer(mitk::DataNode* dataNode)
{
  const mitk::BaseRenderer* selectedRenderer = mitk::BaseRenderer::GetByName(m_Controls.comboBoxRenderWindowSelection->currentText().toStdString());
  m_RenderWindowLayerController->InsertLayerNode(dataNode, -1, selectedRenderer);
}

void QmitkRenderWindowManagerView::RemoveLayer()
{
  QModelIndex selectedIndex = m_Controls.renderWindowTableView->currentIndex();
  if (selectedIndex.isValid())
  {
    QVariant rowData = m_RenderWindowDataModel->data(selectedIndex, Qt::UserRole);
    mitk::DataNode* dataNode = GetDataStorage()->GetNamedNode(rowData.toString().toStdString());
    const mitk::BaseRenderer* selectedRenderer = mitk::BaseRenderer::GetByName(m_Controls.comboBoxRenderWindowSelection->currentText().toStdString());

    m_RenderWindowLayerController->RemoveLayerNode(dataNode, selectedRenderer);
    m_Controls.renderWindowTableView->clearSelection();
  }
}

void QmitkRenderWindowManagerView::SetAsBaseLayer()
{
  QModelIndex selectedIndex = m_Controls.renderWindowTableView->currentIndex();
  if (selectedIndex.isValid())
  {
    QVariant rowData = m_RenderWindowDataModel->data(selectedIndex, Qt::UserRole);
    mitk::DataNode* dataNode = GetDataStorage()->GetNamedNode(rowData.toString().toStdString());
    const mitk::BaseRenderer* selectedRenderer = mitk::BaseRenderer::GetByName(m_Controls.comboBoxRenderWindowSelection->currentText().toStdString());

    m_RenderWindowLayerController->SetBaseDataNode(dataNode, selectedRenderer);
    m_Controls.renderWindowTableView->clearSelection();
  }
}

void QmitkRenderWindowManagerView::MoveLayer(const QString &direction)
{
  QModelIndex selectedIndex = m_Controls.renderWindowTableView->currentIndex();
  if (selectedIndex.isValid())
  {
    QVariant rowData = m_RenderWindowDataModel->data(selectedIndex, Qt::UserRole);
    mitk::DataNode* dataNode = GetDataStorage()->GetNamedNode(rowData.toString().toStdString());
    const mitk::BaseRenderer* selectedRenderer = mitk::BaseRenderer::GetByName(m_Controls.comboBoxRenderWindowSelection->currentText().toStdString());

    if ("up" == direction)
    {
      m_RenderWindowLayerController->MoveNodeUp(dataNode, selectedRenderer);
    }
    else
    {
      m_RenderWindowLayerController->MoveNodeDown(dataNode, selectedRenderer);
    }
    m_Controls.renderWindowTableView->clearSelection();
  }
}

void QmitkRenderWindowManagerView::ResetRenderer()
{
  const mitk::BaseRenderer* selectedRenderer = mitk::BaseRenderer::GetByName(m_Controls.comboBoxRenderWindowSelection->currentText().toStdString());

  m_RenderWindowLayerController->ResetRenderer(true, selectedRenderer);
  m_Controls.renderWindowTableView->clearSelection();
}

void QmitkRenderWindowManagerView::ClearRenderer()
{
  const mitk::BaseRenderer* selectedRenderer = mitk::BaseRenderer::GetByName(m_Controls.comboBoxRenderWindowSelection->currentText().toStdString());

  m_RenderWindowLayerController->ResetRenderer(false, selectedRenderer);
  m_Controls.renderWindowTableView->clearSelection();
}

void QmitkRenderWindowManagerView::ChangeViewDirection(const QString &viewDirection)
{
  mitk::BaseRenderer* selectedRenderer = mitk::BaseRenderer::GetByName(m_Controls.comboBoxRenderWindowSelection->currentText().toStdString());

  m_RenderWindowViewDirectionController->SetViewDirectionOfRenderer(viewDirection.toStdString(), selectedRenderer);
}

void QmitkRenderWindowManagerView::NodeAdded(const mitk::DataNode* node)
{
  // initially set new node as invisible in all render windows
  // this way, each single renderer overwrites the common renderer and the node is invisible
  // until it is inserted in the node list of a render window
  m_RenderWindowLayerController->HideDataNodeInAllRenderer(node);
}
