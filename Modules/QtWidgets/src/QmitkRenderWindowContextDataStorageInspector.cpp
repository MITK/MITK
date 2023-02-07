/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// render window manager UI module
#include "QmitkRenderWindowContextDataStorageInspector.h"

#include <QmitkCustomVariants.h>
#include <QmitkEnums.h>

#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateProperty.h>

// qt
#include <QMenu>
#include <QSignalMapper>

QmitkRenderWindowContextDataStorageInspector::QmitkRenderWindowContextDataStorageInspector(
  QWidget* parent /* =nullptr */,
  mitk::BaseRenderer* renderer /* = nullptr */)
  : QmitkAbstractDataStorageInspector(parent)
{
  m_Controls.setupUi(this);

  mitk::RenderWindowLayerUtilities::RendererVector controlledRenderer{ renderer };
  m_RenderWindowLayerController = std::make_unique<mitk::RenderWindowLayerController>();

  m_StorageModel = std::make_unique<QmitkRenderWindowDataStorageTreeModel>(this);
  m_StorageModel->SetControlledRenderer(controlledRenderer);

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

  connect(m_Controls.renderWindowTreeView, &QTreeView::customContextMenuRequested,
          this, &QmitkRenderWindowContextDataStorageInspector::OnContextMenuRequested);

  SetUpConnections();

  if (nullptr == renderer)
  {
    return;
  }

  m_StorageModel->SetCurrentRenderer(renderer);
}

QAbstractItemView* QmitkRenderWindowContextDataStorageInspector::GetView()
{
  return m_Controls.renderWindowTreeView;
}

const QAbstractItemView* QmitkRenderWindowContextDataStorageInspector::GetView() const
{
  return m_Controls.renderWindowTreeView;
}

void QmitkRenderWindowContextDataStorageInspector::SetSelectionMode(SelectionMode mode)
{
  m_Controls.renderWindowTreeView->setSelectionMode(mode);
}

QmitkRenderWindowContextDataStorageInspector::SelectionMode QmitkRenderWindowContextDataStorageInspector::GetSelectionMode() const
{
  return m_Controls.renderWindowTreeView->selectionMode();
}

QItemSelectionModel* QmitkRenderWindowContextDataStorageInspector::GetDataNodeSelectionModel() const
{
  return m_Controls.renderWindowTreeView->selectionModel();
}

void QmitkRenderWindowContextDataStorageInspector::Initialize()
{
  auto dataStorage = m_DataStorage.Lock();

  if (dataStorage.IsNull())
    return;

  m_StorageModel->SetDataStorage(dataStorage);

  mitk::NodePredicateAnd::Pointer noHelperObjects = mitk::NodePredicateAnd::New();
  noHelperObjects->AddPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object")));
  noHelperObjects->AddPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("hidden object")));
  m_StorageModel->SetNodePredicate(noHelperObjects);

  m_RenderWindowLayerController->SetDataStorage(dataStorage);

  m_Connector->SetView(m_Controls.renderWindowTreeView);
}

void QmitkRenderWindowContextDataStorageInspector::SetUpConnections()
{
  connect(m_StorageModel.get(), &QAbstractItemModel::rowsInserted, this, &QmitkRenderWindowContextDataStorageInspector::ModelRowsInserted);
}

void QmitkRenderWindowContextDataStorageInspector::ModelRowsInserted(const QModelIndex& parent, int /*start*/, int /*end*/)
{
  m_Controls.renderWindowTreeView->setExpanded(parent, true);
}

void QmitkRenderWindowContextDataStorageInspector::OnContextMenuRequested(const QPoint& pos)
{
  QMenu contextMenu;
  contextMenu.addAction(tr("Reinit with node"), this, &QmitkRenderWindowContextDataStorageInspector::OnReinit);
  contextMenu.addAction(tr("Reset to node geometry"), this, &QmitkRenderWindowContextDataStorageInspector::OnReset);

  contextMenu.exec(this->mapToGlobal(pos));
}

void QmitkRenderWindowContextDataStorageInspector::OnReinit()
{
  auto nodes = this->GetSelectedNodes();
  emit ReinitAction(nodes);
}

void QmitkRenderWindowContextDataStorageInspector::OnReset()
{
  auto nodes = this->GetSelectedNodes();
  emit ResetAction(nodes);
}
