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
#include "QmitkDataStorageRenderWindowListModel.h"

// qt widgets module
#include "QmitkCustomVariants.h"
#include "QmitkEnums.h"
#include "QmitkNodeDescriptorManager.h"

QmitkDataStorageRenderWindowListModel::QmitkDataStorageRenderWindowListModel(QObject* parent /*= nullptr*/)
  : QmitkAbstractDataStorageModel(parent)
{
  m_RenderWindowLayerController = std::make_unique<mitk::RenderWindowLayerController>();
}

QmitkDataStorageRenderWindowListModel::~QmitkDataStorageRenderWindowListModel()
{
  // nothing here
}

void QmitkDataStorageRenderWindowListModel::DataStorageChanged()
{
  m_RenderWindowLayerController->SetDataStorage(m_DataStorage.Lock());
  UpdateModelData();
}

void QmitkDataStorageRenderWindowListModel::NodePredicateChanged()
{
  UpdateModelData();
}

void QmitkDataStorageRenderWindowListModel::NodeAdded(const mitk::DataNode* node)
{
  // add a node to each render window specific list (or to a global list initially)
  AddDataNodeToAllRenderer(const_cast<mitk::DataNode*>(node));
  UpdateModelData();
}

void QmitkDataStorageRenderWindowListModel::NodeChanged(const mitk::DataNode* /*node*/)
{
  // nothing here, since the "'NodeChanged'-event is currently sent far too often
  //UpdateModelData();
}

void QmitkDataStorageRenderWindowListModel::NodeRemoved(const mitk::DataNode* /*node*/)
{
  // update model data to create a new list without the removed data node
  UpdateModelData();
}

QModelIndex QmitkDataStorageRenderWindowListModel::index(int row, int column, const QModelIndex& parent) const
{
  bool hasIndex = this->hasIndex(row, column, parent);
  if (hasIndex)
  {
    return this->createIndex(row, column);
  }

  return QModelIndex();
}

QModelIndex QmitkDataStorageRenderWindowListModel::parent(const QModelIndex& /*child*/) const
{
  return QModelIndex();
}

int QmitkDataStorageRenderWindowListModel::rowCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
  if (parent.isValid())
  {
    return 0;
  }

  return static_cast<int>(m_LayerStack.size());
}

int QmitkDataStorageRenderWindowListModel::columnCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
  if (parent.isValid())
  {
    return 0;
  }

  return 1;
}

QVariant QmitkDataStorageRenderWindowListModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid() || this != index.model())
  {
    return QVariant();
  }

  if (index.row() < 0 || index.row() >= static_cast<int>(m_LayerStack.size()))
  {
    return QVariant();
  }

  mitk::RenderWindowLayerUtilities::LayerStack::const_iterator layerStackIt = m_LayerStack.begin();
  std::advance(layerStackIt, index.row());
  mitk::DataNode* dataNode = layerStackIt->second;

  if (Qt::CheckStateRole == role)
  {
    bool visibility = false;
    dataNode->GetVisibility(visibility, m_BaseRenderer);
    if (visibility)
    {
      return Qt::Checked;
    }
    else
    {
      return Qt::Unchecked;
    }
  }
  else if (Qt::DisplayRole == role)
  {
    return QVariant(QString::fromStdString(dataNode->GetName()));
  }
  else if (Qt::ToolTipRole == role)
  {
    return QVariant("Name of the data node.");
  }
  else if (Qt::DecorationRole == role)
  {
    QmitkNodeDescriptor* nodeDescriptor = QmitkNodeDescriptorManager::GetInstance()->GetDescriptor(dataNode);
    return nodeDescriptor->GetIcon(dataNode);
  }
  else if (Qt::UserRole == role || QmitkDataNodeRawPointerRole == role)
  {
    // user role always returns a reference to the data node,
    // which can be used to modify the data node in the data storage
    return QVariant::fromValue<mitk::DataNode*>(dataNode);
  }
  else if (QmitkDataNodeRole == role)
  {
    return QVariant::fromValue<mitk::DataNode::Pointer>(mitk::DataNode::Pointer(dataNode));
  }

  return QVariant();
}

bool QmitkDataStorageRenderWindowListModel::setData(const QModelIndex& index, const QVariant& value, int role /*= Qt::EditRole*/)
{
  if (!index.isValid() || this != index.model())
  {
    return false;
  }

  if (index.row() < 0 || index.row() >= static_cast<int>(m_LayerStack.size()))
  {
    return false;
  }

  mitk::RenderWindowLayerUtilities::LayerStack::const_iterator layerStackIt = m_LayerStack.begin();
  std::advance(layerStackIt, index.row());
  mitk::DataNode* dataNode = layerStackIt->second;
  if (Qt::CheckStateRole == role)
  {
    Qt::CheckState newCheckState = static_cast<Qt::CheckState>(value.toInt());
    bool isVisible = newCheckState;
    dataNode->SetVisibility(isVisible, m_BaseRenderer);

    emit dataChanged(index, index);
    mitk::RenderingManager::GetInstance()->RequestUpdate(m_BaseRenderer->GetRenderWindow());
    return true;
  }
  return false;
}

Qt::ItemFlags QmitkDataStorageRenderWindowListModel::flags(const QModelIndex &index) const
{
  if (!index.isValid() || this != index.model())
  {
    return Qt::NoItemFlags;
  }

  return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
}

void QmitkDataStorageRenderWindowListModel::SetControlledRenderer(mitk::RenderWindowLayerUtilities::RendererVector controlledRenderer)
{
  m_RenderWindowLayerController->SetControlledRenderer(controlledRenderer);
}

void QmitkDataStorageRenderWindowListModel::SetCurrentRenderer(mitk::BaseRenderer* baseRenderer)
{
  if (m_BaseRenderer != baseRenderer)
  {
    m_BaseRenderer = baseRenderer;
    UpdateModelData();
  }
}

void QmitkDataStorageRenderWindowListModel::AddDataNodeToAllRenderer(mitk::DataNode* dataNode)
{
  m_RenderWindowLayerController->InsertLayerNode(dataNode);
}

void QmitkDataStorageRenderWindowListModel::UpdateModelData()
{
  if (!m_DataStorage.IsExpired())
  {
    auto dataStorage = m_DataStorage.Lock();
    if (m_BaseRenderer.IsNotNull())
    {
    // update the model, so that it will be filled with the nodes of the new data storage
      beginResetModel();
      // get the current layer stack of the given base renderer
      m_LayerStack = mitk::RenderWindowLayerUtilities::GetLayerStack(dataStorage, m_BaseRenderer, true);
      endResetModel();
    }
  }
}
