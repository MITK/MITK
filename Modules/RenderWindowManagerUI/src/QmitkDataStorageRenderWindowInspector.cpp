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

// render window manager UI module
#include "QmitkDataStorageRenderWindowInspector.h"

#include "QmitkCustomVariants.h"

// mitk core
#include <mitkBaseRenderer.h>

// qt
#include <QSignalMapper>

QmitkDataStorageRenderWindowInspector::QmitkDataStorageRenderWindowInspector(QWidget* parent /*=nullptr*/)
  : QmitkAbstractDataStorageInspector(parent)
{
  m_Controls.setupUi(this);

  // initialize the render window layer controller and the render window view direction controller
  // and set the controller renderer (in constructor) and the data storage
  m_RenderWindowLayerController = std::make_unique<mitk::RenderWindowLayerController>();
  m_RenderWindowViewDirectionController = std::make_unique<mitk::RenderWindowViewDirectionController>();


  // create a new model
  m_StorageModel = std::make_unique<QmitkDataStorageRenderWindowListModel>(this);

  m_Controls.renderWindowListView->setModel(m_StorageModel.get());
  m_Controls.renderWindowListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_Controls.renderWindowListView->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_Controls.renderWindowListView->setSelectionMode(QAbstractItemView::SingleSelection);
  m_Controls.renderWindowListView->setAlternatingRowColors(true);

  SetUpConnections();
}

QAbstractItemView* QmitkDataStorageRenderWindowInspector::GetView()
{
  return m_Controls.renderWindowListView;
};

const QAbstractItemView* QmitkDataStorageRenderWindowInspector::GetView() const
{
  return m_Controls.renderWindowListView;
};

void QmitkDataStorageRenderWindowInspector::SetSelectionMode(SelectionMode mode)
{
  m_Controls.renderWindowListView->setSelectionMode(mode);
}

QmitkDataStorageRenderWindowInspector::SelectionMode QmitkDataStorageRenderWindowInspector::GetSelectionMode() const
{
  return m_Controls.renderWindowListView->selectionMode();
}

void QmitkDataStorageRenderWindowInspector::Initialize()
{
  m_StorageModel->SetDataStorage(m_DataStorage.Lock());
  m_StorageModel->SetNodePredicate(m_NodePredicate);

  m_RenderWindowLayerController->SetDataStorage(m_DataStorage.Lock());
  m_RenderWindowViewDirectionController->SetDataStorage(m_DataStorage.Lock());

  m_Connector->SetView(m_Controls.renderWindowListView);
}

void QmitkDataStorageRenderWindowInspector::SetUpConnections()
{
  // signal to signal connection
  connect(m_Controls.pushButtonAddLayer, SIGNAL(clicked()), this, SIGNAL(AddLayerButtonClicked()));
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
}

void QmitkDataStorageRenderWindowInspector::SetControlledRenderer(mitk::RenderWindowLayerUtilities::RendererVector controlledRenderer)
{
  m_RenderWindowLayerController->SetControlledRenderer(controlledRenderer);
  m_RenderWindowViewDirectionController->SetControlledRenderer(controlledRenderer);
}

void QmitkDataStorageRenderWindowInspector::SetActiveRenderWindow(const QString& renderWindowId)
{
  mitk::BaseRenderer* selectedRenderer = mitk::BaseRenderer::GetByName(renderWindowId.toStdString());
  if (nullptr == selectedRenderer)
  {
    return;
  }

  m_StorageModel->SetCurrentRenderer(selectedRenderer);
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

void QmitkDataStorageRenderWindowInspector::AddLayer(mitk::DataNode* dataNode)
{
  m_RenderWindowLayerController->InsertLayerNode(dataNode, -1, m_StorageModel->GetCurrentRenderer());
}

void QmitkDataStorageRenderWindowInspector::AddLayerToAllRenderer(mitk::DataNode* dataNode)
{
  m_RenderWindowLayerController->InsertLayerNode(dataNode, -1, nullptr);
}

void QmitkDataStorageRenderWindowInspector::HideDataNodeInAllRenderer(const mitk::DataNode* dataNode)
{
  m_RenderWindowLayerController->HideDataNodeInAllRenderer(dataNode);
}

void QmitkDataStorageRenderWindowInspector::RemoveLayer()
{
  QModelIndex selectedIndex = m_Controls.renderWindowListView->currentIndex();
  if (selectedIndex.isValid())
  {

    QVariant qvariantDataNode = m_StorageModel->data(selectedIndex, Qt::UserRole);
    if (qvariantDataNode.canConvert<mitk::DataNode*>())
    {
      mitk::DataNode* dataNode = qvariantDataNode.value<mitk::DataNode*>();
      m_RenderWindowLayerController->RemoveLayerNode(dataNode, m_StorageModel->GetCurrentRenderer());
      m_Controls.renderWindowListView->clearSelection();
    }
  }
}

void QmitkDataStorageRenderWindowInspector::SetAsBaseLayer()
{
  QModelIndex selectedIndex = m_Controls.renderWindowListView->currentIndex();
  if (selectedIndex.isValid())
  {
    QVariant qvariantDataNode = m_StorageModel->data(selectedIndex, Qt::UserRole);
    if (qvariantDataNode.canConvert<mitk::DataNode*>())
    {
      mitk::DataNode* dataNode = qvariantDataNode.value<mitk::DataNode*>();
      m_RenderWindowLayerController->SetBaseDataNode(dataNode, m_StorageModel->GetCurrentRenderer());
      m_Controls.renderWindowListView->clearSelection();
    }
  }
}

void QmitkDataStorageRenderWindowInspector::MoveLayer(const QString& direction)
{
  /*
  QModelIndex selectedIndex = m_Controls.renderWindowListView->currentIndex();
  if (selectedIndex.isValid())
  {
    QVariant qvariantDataNode = m_RenderWindowDataModel->data(selectedIndex, Qt::UserRole);
    if (qvariantDataNode.canConvert<mitk::DataNode*>())
    {
      mitk::DataNode* dataNode = qvariantDataNode.value<mitk::DataNode*>();
      const mitk::BaseRenderer* selectedRenderer = m_RenderWindowDataModel->GetCurrentRenderer();

      bool success = false;
      if ("up" == direction)
      {
        success = m_RenderWindowLayerController->MoveNodeUp(dataNode, selectedRenderer);
        if (success)
        {
          // node has been successfully moved up
          m_Controls.renderWindowListView->selectRow(selectedIndex.row() - 1);
        }
      }
      else
      {
        success = m_RenderWindowLayerController->MoveNodeDown(dataNode, selectedRenderer);
        if (success)
        {
          // node has been successfully moved down
          m_Controls.renderWindowListView->selectRow(selectedIndex.row() + 1);
        }
      }
    }
  }
  */
}

void QmitkDataStorageRenderWindowInspector::ResetRenderer()
{
  m_RenderWindowLayerController->ResetRenderer(true, m_StorageModel->GetCurrentRenderer());
  m_Controls.renderWindowListView->clearSelection();
}

void QmitkDataStorageRenderWindowInspector::ClearRenderer()
{
  m_RenderWindowLayerController->ResetRenderer(false, m_StorageModel->GetCurrentRenderer());
  m_Controls.renderWindowListView->clearSelection();
}

void QmitkDataStorageRenderWindowInspector::ChangeViewDirection(const QString& viewDirection)
{
  m_RenderWindowViewDirectionController->SetViewDirectionOfRenderer(viewDirection.toStdString(), m_StorageModel->GetCurrentRenderer());
}
