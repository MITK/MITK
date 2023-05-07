/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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

  m_StorageModel = std::make_unique<QmitkRenderWindowDataStorageTreeModel>(this);

  m_Controls.renderWindowTreeView->setModel(m_StorageModel.get());
  m_Controls.renderWindowTreeView->setHeaderHidden(true);
  m_Controls.renderWindowTreeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_Controls.renderWindowTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_Controls.renderWindowTreeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
  m_Controls.renderWindowTreeView->setAlternatingRowColors(true);
  m_Controls.renderWindowTreeView->setDragEnabled(true);
  m_Controls.renderWindowTreeView->setDropIndicatorShown(true);
  m_Controls.renderWindowTreeView->setAcceptDrops(true);
  m_Controls.renderWindowTreeView->setContextMenuPolicy(Qt::CustomContextMenu);

  SetUpConnections();
}

QAbstractItemView* QmitkRenderWindowDataStorageInspector::GetView()
{
  return m_Controls.renderWindowTreeView;
}

const QAbstractItemView* QmitkRenderWindowDataStorageInspector::GetView() const
{
  return m_Controls.renderWindowTreeView;
}

void QmitkRenderWindowDataStorageInspector::SetSelectionMode(SelectionMode mode)
{
  m_Controls.renderWindowTreeView->setSelectionMode(mode);
}

QmitkRenderWindowDataStorageInspector::SelectionMode QmitkRenderWindowDataStorageInspector::GetSelectionMode() const
{
  return m_Controls.renderWindowTreeView->selectionMode();
}

void QmitkRenderWindowDataStorageInspector::Initialize()
{
  auto dataStorage = m_DataStorage.Lock();

  if (dataStorage.IsNull())
    return;

  m_StorageModel->SetDataStorage(dataStorage);
  m_StorageModel->SetNodePredicate(m_NodePredicate);

  m_RenderWindowLayerController->SetDataStorage(dataStorage);
  m_RenderWindowViewDirectionController->SetDataStorage(dataStorage);

  m_Connector->SetView(m_Controls.renderWindowTreeView);
}

void QmitkRenderWindowDataStorageInspector::SetUpConnections()
{
  connect(m_StorageModel.get(), &QAbstractItemModel::rowsInserted, this, &QmitkRenderWindowDataStorageInspector::ModelRowsInserted);

  QSignalMapper* changeViewDirectionSignalMapper = new QSignalMapper(this);
  changeViewDirectionSignalMapper->setMapping(m_Controls.radioButtonAxial, QString("axial"));
  changeViewDirectionSignalMapper->setMapping(m_Controls.radioButtonCoronal, QString("coronal"));
  changeViewDirectionSignalMapper->setMapping(m_Controls.radioButtonSagittal, QString("sagittal"));
  changeViewDirectionSignalMapper->setMapping(m_Controls.radioButton3D, QString("3D"));
  connect(changeViewDirectionSignalMapper, static_cast<void(QSignalMapper::*)(const QString&)>(&QSignalMapper::mapped), this, &QmitkRenderWindowDataStorageInspector::ChangeViewDirection);

  connect(m_Controls.radioButtonAxial, &QRadioButton::clicked, changeViewDirectionSignalMapper, static_cast<void(QSignalMapper::*)()>(&QSignalMapper::map));
  connect(m_Controls.radioButtonCoronal, &QRadioButton::clicked, changeViewDirectionSignalMapper, static_cast<void(QSignalMapper::*)()>(&QSignalMapper::map));
  connect(m_Controls.radioButtonSagittal, &QRadioButton::clicked, changeViewDirectionSignalMapper, static_cast<void(QSignalMapper::*)()>(&QSignalMapper::map));
  connect(m_Controls.radioButton3D, &QRadioButton::clicked, changeViewDirectionSignalMapper, static_cast<void(QSignalMapper::*)()>(&QSignalMapper::map));
}

void QmitkRenderWindowDataStorageInspector::SetControlledRenderer(mitk::RenderWindowLayerUtilities::RendererVector controlledRenderer)
{
  m_StorageModel->SetControlledRenderer(controlledRenderer);
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
  mitk::AnatomicalPlane viewDirection = selectedRenderer->GetSliceNavigationController()->GetDefaultViewDirection();
  switch (viewDirection)
  {
  case mitk::AnatomicalPlane::Axial:
    m_Controls.radioButtonAxial->setChecked(true);
    break;
  case mitk::AnatomicalPlane::Coronal:
    m_Controls.radioButtonCoronal->setChecked(true);
    break;
  case mitk::AnatomicalPlane::Sagittal:
    m_Controls.radioButtonSagittal->setChecked(true);
    break;
  default:
    break;
  }
}

void QmitkRenderWindowDataStorageInspector::ModelRowsInserted(const QModelIndex& parent, int /*start*/, int /*end*/)
{
  m_Controls.renderWindowTreeView->setExpanded(parent, true);
}

void QmitkRenderWindowDataStorageInspector::ChangeViewDirection(const QString& viewDirection)
{
  m_RenderWindowViewDirectionController->SetViewDirectionOfRenderer(viewDirection.toStdString(), m_StorageModel->GetCurrentRenderer());
}
