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
#include "QmitkRenderWindowManipulatorWidget.h"

// qt widgets module
#include "QmitkCustomVariants.h"
#include "QmitkEnums.h"

// mitk core
#include <mitkBaseRenderer.h>

// qt
#include <QSignalMapper>

QmitkRenderWindowManipulatorWidget::QmitkRenderWindowManipulatorWidget(mitk::DataStorage::Pointer dataStorage, QWidget* parent /*=nullptr*/)
  : QWidget(parent)
  , m_DataStorage(dataStorage)
  , m_StorageModel(nullptr)
{
  Init();
}

void QmitkRenderWindowManipulatorWidget::Init()
{
  m_Controls.setupUi(this);

  // initialize the render window layer controller and the render window view direction controller
  // and set the controller renderer (in constructor) and the data storage
  m_RenderWindowLayerController = std::make_unique<mitk::RenderWindowLayerController>();
  m_RenderWindowViewDirectionController = std::make_unique<mitk::RenderWindowViewDirectionController>();
  m_RenderWindowLayerController->SetDataStorage(m_DataStorage);
  m_RenderWindowViewDirectionController->SetDataStorage(m_DataStorage);

  m_Controls.layerStackTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_Controls.layerStackTableView->horizontalHeader()->setStretchLastSection(true);
  m_Controls.layerStackTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_Controls.layerStackTableView->setSelectionMode(QAbstractItemView::SingleSelection);

  m_StorageModel = new QmitkDataStorageLayerStackModel(this);
  m_StorageModel->SetDataStorage(m_DataStorage);

  m_Controls.layerStackTableView->setModel(m_StorageModel);

  SetUpConnections();
}

void QmitkRenderWindowManipulatorWidget::SetUpConnections()
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

QTableView* QmitkRenderWindowManipulatorWidget::GetLayerStackTableView()
{
  return m_Controls.layerStackTableView;
}

void QmitkRenderWindowManipulatorWidget::SetControlledRenderer(RenderWindowLayerUtilities::RendererVector controlledRenderer)
{
  m_RenderWindowLayerController->SetControlledRenderer(controlledRenderer);
  m_RenderWindowViewDirectionController->SetControlledRenderer(controlledRenderer);
}

void QmitkRenderWindowManipulatorWidget::SetActiveRenderWindow(const QString &renderWindowId)
{
  if (nullptr == m_StorageModel)
  {
    return;
  }

  std::string currentRendererName = renderWindowId.toStdString();
  m_StorageModel->SetCurrentRenderer(currentRendererName);

  mitk::BaseRenderer* selectedRenderer = mitk::BaseRenderer::GetByName(currentRendererName);
  if (nullptr == selectedRenderer)
  {
    return;
  }

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

void QmitkRenderWindowManipulatorWidget::AddLayer(mitk::DataNode* dataNode)
{
  if (nullptr == m_StorageModel)
  {
    return;
  }

  m_RenderWindowLayerController->InsertLayerNode(dataNode, -1, m_StorageModel->GetCurrentRenderer());
}

void QmitkRenderWindowManipulatorWidget::AddLayerToAllRenderer(mitk::DataNode* dataNode)
{
  m_RenderWindowLayerController->InsertLayerNode(dataNode, -1, nullptr);
}

void QmitkRenderWindowManipulatorWidget::HideDataNodeInAllRenderer(const mitk::DataNode* dataNode)
{
  m_RenderWindowLayerController->HideDataNodeInAllRenderer(dataNode);
}

void QmitkRenderWindowManipulatorWidget::RemoveLayer()
{
  if (nullptr == m_StorageModel)
  {
    return;
  }

  QModelIndex selectedIndex = m_Controls.layerStackTableView->currentIndex();
  if (selectedIndex.isValid())
  {

    QVariant qvariantDataNode = m_StorageModel->data(selectedIndex, QmitkDataNodeRawPointerRole);
    if (qvariantDataNode.canConvert<mitk::DataNode*>())
    {
      mitk::DataNode* dataNode = qvariantDataNode.value<mitk::DataNode*>();
      m_RenderWindowLayerController->RemoveLayerNode(dataNode, m_StorageModel->GetCurrentRenderer());
      m_Controls.layerStackTableView->clearSelection();
    }
  }
}

void QmitkRenderWindowManipulatorWidget::SetAsBaseLayer()
{
  if (nullptr == m_StorageModel)
  {
    return;
  }

  QModelIndex selectedIndex = m_Controls.layerStackTableView->currentIndex();
  if (selectedIndex.isValid())
  {
    QVariant qvariantDataNode = m_StorageModel->data(selectedIndex, QmitkDataNodeRawPointerRole);
    if (qvariantDataNode.canConvert<mitk::DataNode*>())
    {
      mitk::DataNode* dataNode = qvariantDataNode.value<mitk::DataNode*>();
      m_RenderWindowLayerController->SetBaseDataNode(dataNode, m_StorageModel->GetCurrentRenderer());
      m_Controls.layerStackTableView->clearSelection();
    }
  }
}

void QmitkRenderWindowManipulatorWidget::MoveLayer(const QString &direction)
{
  if (nullptr == m_StorageModel)
  {
    return;
  }

  QModelIndex selectedIndex = m_Controls.layerStackTableView->currentIndex();
  if (selectedIndex.isValid())
  {
    QVariant qvariantDataNode = m_StorageModel->data(selectedIndex, QmitkDataNodeRawPointerRole);
    if (qvariantDataNode.canConvert<mitk::DataNode*>())
    {
      mitk::DataNode* dataNode = qvariantDataNode.value<mitk::DataNode*>();
      const mitk::BaseRenderer* selectedRenderer = m_StorageModel->GetCurrentRenderer();

      bool success = false;
      if ("up" == direction)
      {
        success = m_RenderWindowLayerController->MoveNodeUp(dataNode, selectedRenderer);
        if (success)
        {
          // node has been successfully moved up
          QTableView* tableView = dynamic_cast<QTableView*>(m_Controls.layerStackTableView);
          if (nullptr != tableView)
          {
            tableView->selectRow(selectedIndex.row() - 1);
          }
        }
      }
      else
      {
        success = m_RenderWindowLayerController->MoveNodeDown(dataNode, selectedRenderer);
        if (success)
        {
          // node has been successfully moved down
          QTableView* tableView = dynamic_cast<QTableView*>(m_Controls.layerStackTableView);
          if (nullptr != tableView)
          {
            tableView->selectRow(selectedIndex.row() + 1);
          }
        }
      }
    }
  }
}

void QmitkRenderWindowManipulatorWidget::ResetRenderer()
{
  if (nullptr == m_StorageModel)
  {
    return;
  }

  m_RenderWindowLayerController->ResetRenderer(true, m_StorageModel->GetCurrentRenderer());
  m_Controls.layerStackTableView->clearSelection();
}

void QmitkRenderWindowManipulatorWidget::ClearRenderer()
{
  if (nullptr == m_StorageModel)
  {
    return;
  }

  m_RenderWindowLayerController->ResetRenderer(false, m_StorageModel->GetCurrentRenderer());
  m_Controls.layerStackTableView->clearSelection();
}

void QmitkRenderWindowManipulatorWidget::ChangeViewDirection(const QString &viewDirection)
{
  if (nullptr == m_StorageModel)
  {
    return;

  }

  m_RenderWindowViewDirectionController->SetViewDirectionOfRenderer(viewDirection.toStdString(), m_StorageModel->GetCurrentRenderer());
}
