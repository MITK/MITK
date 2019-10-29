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
#include "QmitkRenderWindowDataStorageListModel.h"

// qt widgets module
#include "QmitkCustomVariants.h"
#include "QmitkEnums.h"
#include "QmitkMimeTypes.h"
#include "QmitkNodeDescriptorManager.h"

QmitkRenderWindowDataStorageListModel::QmitkRenderWindowDataStorageListModel(QObject* parent /*= nullptr*/)
  : QmitkAbstractDataStorageModel(parent)
{
  m_RenderWindowLayerController = std::make_unique<mitk::RenderWindowLayerController>();
}

void QmitkRenderWindowDataStorageListModel::DataStorageChanged()
{
  m_RenderWindowLayerController->SetDataStorage(m_DataStorage.Lock());
  UpdateModelData();
}

void QmitkRenderWindowDataStorageListModel::NodePredicateChanged()
{
  UpdateModelData();
}

void QmitkRenderWindowDataStorageListModel::NodeAdded(const mitk::DataNode* node)
{
  // add a node to each render window specific list (or to a global list initially)
  AddDataNodeToAllRenderer(const_cast<mitk::DataNode*>(node));
  UpdateModelData();
}

void QmitkRenderWindowDataStorageListModel::NodeChanged(const mitk::DataNode* /*node*/)
{
  // nothing here, since the "'NodeChanged'-event is currently sent far too often
}

void QmitkRenderWindowDataStorageListModel::NodeRemoved(const mitk::DataNode* /*node*/)
{
  // update model data to create a new list without the removed data node
  UpdateModelData();
}

QModelIndex QmitkRenderWindowDataStorageListModel::index(int row, int column, const QModelIndex& parent) const
{
  bool hasIndex = this->hasIndex(row, column, parent);
  if (hasIndex)
  {
    return this->createIndex(row, column);
  }

  return QModelIndex();
}

QModelIndex QmitkRenderWindowDataStorageListModel::parent(const QModelIndex& /*child*/) const
{
  return QModelIndex();
}

int QmitkRenderWindowDataStorageListModel::rowCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
  if (parent.isValid())
  {
    return 0;
  }

  return static_cast<int>(m_LayerStack.size());
}

int QmitkRenderWindowDataStorageListModel::columnCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
  if (parent.isValid())
  {
    return 0;
  }

  return 1;
}

QVariant QmitkRenderWindowDataStorageListModel::data(const QModelIndex& index, int role) const
{
  if (m_BaseRenderer.IsExpired())
  {
    return QVariant();
  }

  auto baseRenderer = m_BaseRenderer.Lock();

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
    dataNode->GetVisibility(visibility, baseRenderer);
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

bool QmitkRenderWindowDataStorageListModel::setData(const QModelIndex& index, const QVariant& value, int role /*= Qt::EditRole*/)
{
  if (m_BaseRenderer.IsExpired())
  {
    return false;
  }

  auto baseRenderer = m_BaseRenderer.Lock();

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
    dataNode->SetVisibility(isVisible, baseRenderer);

    emit dataChanged(index, index);
    mitk::RenderingManager::GetInstance()->RequestUpdate(baseRenderer->GetRenderWindow());
    return true;
  }
  return false;
}

Qt::ItemFlags QmitkRenderWindowDataStorageListModel::flags(const QModelIndex &index) const
{
  if (this != index.model())
  {
    return Qt::NoItemFlags;
  }

  if (!index.isValid())
  {
    return Qt::ItemIsDropEnabled;
  }

  return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

Qt::DropActions QmitkRenderWindowDataStorageListModel::supportedDropActions() const
{
  return Qt::CopyAction | Qt::MoveAction;
}

Qt::DropActions QmitkRenderWindowDataStorageListModel::supportedDragActions() const
{
  return Qt::CopyAction | Qt::MoveAction;
}

QStringList QmitkRenderWindowDataStorageListModel::mimeTypes() const
{
  QStringList types = QAbstractItemModel::mimeTypes();
  types << QmitkMimeTypes::DataNodePtrs;
  return types;
}

QMimeData* QmitkRenderWindowDataStorageListModel::mimeData(const QModelIndexList& indexes) const
{
  QMimeData* mimeData = new QMimeData();
  QByteArray encodedData;

  QDataStream stream(&encodedData, QIODevice::WriteOnly);

  for (const auto& index : indexes)
  {
    if (index.isValid())
    {
      auto dataNode = data(index, QmitkDataNodeRawPointerRole).value<mitk::DataNode*>();
      stream << reinterpret_cast<quintptr>(dataNode);
    }
  }

  mimeData->setData(QmitkMimeTypes::DataNodePtrs, encodedData);
  return mimeData;
}

bool QmitkRenderWindowDataStorageListModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int /*row*/, int column, const QModelIndex& parent)
{
  if (m_BaseRenderer.IsExpired())
  {
    return false;
  }

  auto baseRenderer = m_BaseRenderer.Lock();

  if (action == Qt::IgnoreAction)
  {
    return true;
  }

  if (!data->hasFormat(QmitkMimeTypes::DataNodePtrs))
  {
    return false;
  }

  if (column > 0)
  {
    return false;
  }

  if (parent.isValid())
  {
    int layer = -1;
    auto dataNode = this->data(parent, QmitkDataNodeRawPointerRole).value<mitk::DataNode*>();
    if (nullptr != dataNode)
    {
      dataNode->GetIntProperty("layer", layer, baseRenderer);
    }

    auto dataNodeList = QmitkMimeTypes::ToDataNodePtrList(data);
    for (const auto& dataNode : dataNodeList)
    {
      m_RenderWindowLayerController->MoveNodeToPosition(dataNode, layer, baseRenderer);
    }

    UpdateModelData();
    return true;
  }

  return false;
}

void QmitkRenderWindowDataStorageListModel::SetControlledRenderer(mitk::RenderWindowLayerUtilities::RendererVector controlledRenderer)
{
  m_RenderWindowLayerController->SetControlledRenderer(controlledRenderer);

  if (!m_DataStorage.IsExpired())
  {
    auto dataStorage = m_DataStorage.Lock();
    mitk::DataStorage::SetOfObjects::ConstPointer allDataNodes = dataStorage->GetAll();
    for (mitk::DataStorage::SetOfObjects::ConstIterator it = allDataNodes->Begin(); it != allDataNodes->End(); ++it)
    {
      mitk::DataNode::Pointer dataNode = it->Value();
      if (dataNode.IsNull())
      {
        continue;
      }

      AddDataNodeToAllRenderer(dataNode);
    }
  }
}

void QmitkRenderWindowDataStorageListModel::SetCurrentRenderer(mitk::BaseRenderer* baseRenderer)
{
  if (m_BaseRenderer == baseRenderer)
  {
    return;
  }

  m_BaseRenderer = baseRenderer;
  if (!m_BaseRenderer.IsExpired())
  {
    UpdateModelData();
  }
}

mitk::BaseRenderer* QmitkRenderWindowDataStorageListModel::GetCurrentRenderer() const
{
  if (m_BaseRenderer.IsExpired())
  {
    return nullptr;
  }

  return m_BaseRenderer.Lock().GetPointer();
}

void QmitkRenderWindowDataStorageListModel::AddDataNodeToAllRenderer(mitk::DataNode* dataNode)
{
  m_RenderWindowLayerController->InsertLayerNode(dataNode);
}

void QmitkRenderWindowDataStorageListModel::UpdateModelData()
{
  if (!m_DataStorage.IsExpired())
  {
    auto dataStorage = m_DataStorage.Lock();
    if (!m_BaseRenderer.IsExpired())
    {
      auto baseRenderer = m_BaseRenderer.Lock();
      // update the model, so that it will be filled with the nodes of the new data storage
      beginResetModel();
      // get the current layer stack of the given base renderer
      m_LayerStack = mitk::RenderWindowLayerUtilities::GetLayerStack(dataStorage, baseRenderer, true);
      endResetModel();
    }
  }
}
