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
#include "QmitkRenderWindowDataStorageInspector.h"

#include "QmitkCustomVariants.h"

// mitk core
#include <mitkBaseRenderer.h>

// qt
#include <QSignalMapper>

QmitkRenderWindowDataStorageInspector::QmitkRenderWindowDataStorageInspector(QWidget* parent /*=nullptr*/)
  : QmitkAbstractDataStorageInspector(parent)
{
  m_Controls.setupUi(this);

  // initialize the render window layer controller and the render window view direction controller
  m_RenderWindowLayerController = std::make_unique<mitk::RenderWindowLayerController>();
  m_RenderWindowViewDirectionController = std::make_unique<mitk::RenderWindowViewDirectionController>();

  m_StorageModel = std::make_unique<QmitkRenderWindowDataStorageListModel>(this);

  m_Controls.renderWindowListView->setModel(m_StorageModel.get());
  m_Controls.renderWindowListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_Controls.renderWindowListView->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_Controls.renderWindowListView->setSelectionMode(QAbstractItemView::ExtendedSelection);
  m_Controls.renderWindowListView->setAlternatingRowColors(true);
  m_Controls.renderWindowListView->setDragEnabled(true);
  m_Controls.renderWindowListView->setDropIndicatorShown(true);
  m_Controls.renderWindowListView->setAcceptDrops(true);
  m_Controls.renderWindowListView->setContextMenuPolicy(Qt::CustomContextMenu);

  SetUpConnections();
}

QAbstractItemView* QmitkRenderWindowDataStorageInspector::GetView()
{
  return m_Controls.renderWindowListView;
}

const QAbstractItemView* QmitkRenderWindowDataStorageInspector::GetView() const
{
  return m_Controls.renderWindowListView;
}

void QmitkRenderWindowDataStorageInspector::SetSelectionMode(SelectionMode mode)
{
  m_Controls.renderWindowListView->setSelectionMode(mode);
}

QmitkRenderWindowDataStorageInspector::SelectionMode QmitkRenderWindowDataStorageInspector::GetSelectionMode() const
{
  return m_Controls.renderWindowListView->selectionMode();
}

void QmitkRenderWindowDataStorageInspector::Initialize()
{
  if (m_DataStorage.IsExpired())
  {
    return;
  }

  auto dataStorage = m_DataStorage.Lock();

  m_StorageModel->SetDataStorage(dataStorage);
  m_StorageModel->SetNodePredicate(m_NodePredicate);

  m_RenderWindowLayerController->SetDataStorage(dataStorage);
  m_RenderWindowViewDirectionController->SetDataStorage(dataStorage);

  m_Connector->SetView(m_Controls.renderWindowListView);
}

void QmitkRenderWindowDataStorageInspector::SetUpConnections()
{
  connect(m_Controls.pushButtonSetAsBaseLayer, &QPushButton::clicked, this, &QmitkRenderWindowDataStorageInspector::SetAsBaseLayer);
  connect(m_Controls.pushButtonResetRenderer, &QPushButton::clicked, this, &QmitkRenderWindowDataStorageInspector::ResetRenderer);

  QSignalMapper* changeViewDirectionSignalMapper = new QSignalMapper(this);
  changeViewDirectionSignalMapper->setMapping(m_Controls.radioButtonAxial, QString("axial"));
  changeViewDirectionSignalMapper->setMapping(m_Controls.radioButtonCoronal, QString("coronal"));
  changeViewDirectionSignalMapper->setMapping(m_Controls.radioButtonSagittal, QString("sagittal"));
  changeViewDirectionSignalMapper->setMapping(m_Controls.radioButton3D, QString("3D"));
  connect(changeViewDirectionSignalMapper, static_cast<void(QSignalMapper::*)(const QString&)>(&QSignalMapper::mapped), this, &QmitkRenderWindowDataStorageInspector::ChangeViewDirection);

  connect(m_Controls.radioButtonAxial, &QPushButton::clicked, changeViewDirectionSignalMapper, static_cast<void(QSignalMapper::*)()>(&QSignalMapper::map));
  connect(m_Controls.radioButtonCoronal, &QPushButton::clicked, changeViewDirectionSignalMapper, static_cast<void(QSignalMapper::*)()>(&QSignalMapper::map));
  connect(m_Controls.radioButtonSagittal, &QPushButton::clicked, changeViewDirectionSignalMapper, static_cast<void(QSignalMapper::*)()>(&QSignalMapper::map));
  connect(m_Controls.radioButton3D, &QPushButton::clicked, changeViewDirectionSignalMapper, static_cast<void(QSignalMapper::*)()>(&QSignalMapper::map));
}

void QmitkRenderWindowDataStorageInspector::SetControlledRenderer(mitk::RenderWindowLayerUtilities::RendererVector controlledRenderer)
{
  m_StorageModel->SetControlledRenderer(controlledRenderer);
  m_RenderWindowLayerController->SetControlledRenderer(controlledRenderer);
  m_RenderWindowViewDirectionController->SetControlledRenderer(controlledRenderer);
}

void QmitkRenderWindowDataStorageInspector::SetActiveRenderWindow(const QString& renderWindowId)
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

void QmitkRenderWindowDataStorageInspector::SetAsBaseLayer()
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

void QmitkRenderWindowDataStorageInspector::ResetRenderer()
{
  m_RenderWindowLayerController->ResetRenderer(true, m_StorageModel->GetCurrentRenderer());
  m_Controls.renderWindowListView->clearSelection();
}

void QmitkRenderWindowDataStorageInspector::ChangeViewDirection(const QString& viewDirection)
{
  m_RenderWindowViewDirectionController->SetViewDirectionOfRenderer(viewDirection.toStdString(), m_StorageModel->GetCurrentRenderer());
}
