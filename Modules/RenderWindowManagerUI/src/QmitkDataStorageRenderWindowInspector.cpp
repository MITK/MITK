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

  // initialize the render window view direction controller
  // and set the controlled renderer (in constructor) and the data storage
  m_RenderWindowViewDirectionController = std::make_unique<mitk::RenderWindowViewDirectionController>();

  // create a new model
  m_StorageModel = std::make_unique<QmitkDataStorageRenderWindowListModel>(this);

  m_Controls.renderWindowListView->setModel(m_StorageModel.get());
  m_Controls.renderWindowListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_Controls.renderWindowListView->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_Controls.renderWindowListView->setSelectionMode(QAbstractItemView::ExtendedSelection);
  m_Controls.renderWindowListView->setAlternatingRowColors(true);
  m_Controls.renderWindowListView->setDragEnabled(true);
  m_Controls.renderWindowListView->setDropIndicatorShown(true);
  m_Controls.renderWindowListView->setAcceptDrops(true);

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

  m_RenderWindowViewDirectionController->SetDataStorage(m_DataStorage.Lock());

  m_Connector->SetView(m_Controls.renderWindowListView);
}

void QmitkDataStorageRenderWindowInspector::SetUpConnections()
{
  connect(m_Controls.pushButtonSetAsBaseLayer, SIGNAL(clicked()), this, SLOT(SetAsBaseLayer()));

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
  m_StorageModel->SetControlledRenderer(controlledRenderer);
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

void QmitkDataStorageRenderWindowInspector::SetAsBaseLayer()
{
  /*
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
  */
}

void QmitkDataStorageRenderWindowInspector::ResetRenderer()
{
  //m_RenderWindowLayerController->ResetRenderer(true, m_StorageModel->GetCurrentRenderer());
  m_Controls.renderWindowListView->clearSelection();
}

void QmitkDataStorageRenderWindowInspector::ClearRenderer()
{
  //m_RenderWindowLayerController->ResetRenderer(false, m_StorageModel->GetCurrentRenderer());
  m_Controls.renderWindowListView->clearSelection();
}

void QmitkDataStorageRenderWindowInspector::ChangeViewDirection(const QString& viewDirection)
{
  m_RenderWindowViewDirectionController->SetViewDirectionOfRenderer(viewDirection.toStdString(), m_StorageModel->GetCurrentRenderer());
}
