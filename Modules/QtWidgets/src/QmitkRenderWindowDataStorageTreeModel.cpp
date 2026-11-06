/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// render window manager UI module
#include <QmitkRenderWindowDataStorageTreeModel.h>

#include <QmitkDataStorageTreeModelInternalItem.h>
// qt widgets module
#include <QmitkCustomVariants.h>
#include <QmitkEnums.h>
#include <QmitkMimeTypes.h>
#include <QmitkNodeDescriptorManager.h>

#include <QIODevice>

QmitkRenderWindowDataStorageTreeModel::QmitkRenderWindowDataStorageTreeModel(QObject* parent /*= nullptr*/)
  : QmitkAbstractDataStorageModel(parent)
  , m_Root(nullptr)
{
  m_RenderWindowLayerController = std::make_unique<mitk::RenderWindowLayerController>();
  ResetTree();
}

void QmitkRenderWindowDataStorageTreeModel::DataStorageChanged()
{
  m_RenderWindowLayerController->SetDataStorage(m_DataStorage.Lock());
  ResetTree();
  UpdateModelData();
}

void QmitkRenderWindowDataStorageTreeModel::NodePredicateChanged()
{
  ResetTree();
  UpdateModelData();
}

void QmitkRenderWindowDataStorageTreeModel::NodeAdded(const mitk::DataNode* node)
{
  auto baseRenderer = m_BaseRenderer.Lock();

  if (baseRenderer.IsNull())
  {
    return;
  }

  if (m_NodePredicate.IsNotNull())
  {
    if (m_NodePredicate->CheckNode(node))
    {
      AddNodeInternal(node, baseRenderer);
    }
  }
}

void QmitkRenderWindowDataStorageTreeModel::NodeChanged(const mitk::DataNode* node)
{
  auto item = m_Root->Find(node);
  if (nullptr != item)
  {
    auto parentItem = item->GetParent();
    // as the root node should not be removed one should always have a parent item
    if (nullptr == parentItem)
    {
      return;
    }

    auto index = createIndex(item->GetIndex(), 0, item);
    emit dataChanged(index, index);
  }
}

void QmitkRenderWindowDataStorageTreeModel::NodeRemoved(const mitk::DataNode* node)
{
  RemoveNodeInternal(node);
}

void QmitkRenderWindowDataStorageTreeModel::NodeSourcesChanged(const mitk::DataNode* node)
{
  if (nullptr == node || nullptr == m_Root || m_BaseRenderer.IsExpired() || m_NodePredicate.IsNull() ||
      !m_NodePredicate->CheckNode(node))
  {
    return;
  }

  auto item = m_Root->Find(node);
  auto parentItem = m_Root;

  auto parentDataNode = this->GetParentNode(node);
  if (nullptr != parentDataNode)
  {
    parentItem = m_Root->Find(parentDataNode);
  }

  // A node is shown only below a shown parent, so a node that appears or
  // disappears with its new parent needs the tree built anew.
  if (nullptr == item || nullptr == parentItem)
  {
    this->ResetTree();
    this->UpdateModelData();
    return;
  }

  if (parentItem != item->GetParent())
  {
    this->MoveTreeItem(item, parentItem);
  }
}

void QmitkRenderWindowDataStorageTreeModel::MoveTreeItem(QmitkDataStorageTreeModelInternalItem* item, QmitkDataStorageTreeModelInternalItem* newParentItem)
{
  auto oldParentItem = item->GetParent();
  auto baseRenderer = m_BaseRenderer.Lock();
  const int row = this->GetLayerRow(newParentItem, item->GetDataNode(), baseRenderer);

  if (this->beginMoveRows(this->GetIndexByItem(oldParentItem), item->GetIndex(), item->GetIndex(), this->GetIndexByItem(newParentItem), row))
  {
    oldParentItem->RemoveChild(item);
    newParentItem->InsertChild(item, row);
    this->endMoveRows();
  }
}

QModelIndex QmitkRenderWindowDataStorageTreeModel::index(int row, int column, const QModelIndex& parent) const
{
  auto item = GetItemByIndex(parent);
  if (nullptr != item)
  {
    item = item->GetChild(row);
  }

  if (nullptr == item)
  {
    return QModelIndex();
  }

  return createIndex(row, column, item);
}

QModelIndex QmitkRenderWindowDataStorageTreeModel::parent(const QModelIndex& parent) const
{
  auto item = GetItemByIndex(parent);
  if (nullptr != item)
  {
    item = item->GetParent();
  }

  if(nullptr == item)
  {
    return QModelIndex();
  }

  if (item == m_Root)
  {
    return QModelIndex();
  }

  return createIndex(item->GetIndex(), 0, item);
}

int QmitkRenderWindowDataStorageTreeModel::rowCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
  auto item = GetItemByIndex(parent);
  if (nullptr == item)
  {
    return 0;
  }

  return item->GetChildCount();
}

int QmitkRenderWindowDataStorageTreeModel::columnCount(const QModelIndex&/* parent = QModelIndex()*/) const
{
  if (0 == m_Root->GetChildCount())
  {
    // no items stored, no need to display columns
    return 0;
  }

  return 1;
}

QVariant QmitkRenderWindowDataStorageTreeModel::data(const QModelIndex& index, int role) const
{
  auto baseRenderer = m_BaseRenderer.Lock();

  if (baseRenderer.IsNull())
  {
    return QVariant();
  }

  if (!index.isValid() || this != index.model())
  {
    return QVariant();
  }

  auto item = GetItemByIndex(index);
  if (nullptr == item)
  {
    return QVariant();
  }

  auto dataNode = item->GetDataNode();
  if (nullptr == dataNode)
  {
    return QVariant();
  }

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

bool QmitkRenderWindowDataStorageTreeModel::setData(const QModelIndex& index, const QVariant& value, int role /*= Qt::EditRole*/)
{
  auto baseRenderer = m_BaseRenderer.Lock();

  if (baseRenderer.IsNull())
  {
    return false;
  }

  if (!index.isValid() || this != index.model())
  {
    return false;
  }

  auto item = GetItemByIndex(index);
  if (nullptr == item)
  {
    return false;
  }

  auto dataNode = item->GetDataNode();
  if (nullptr == dataNode)
  {
    return false;
  }

  if (Qt::EditRole == role && !value.toString().isEmpty())
  {
    dataNode->SetName(value.toString().toStdString().c_str());
    emit dataChanged(index, index);
    return true;
  }
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

Qt::ItemFlags QmitkRenderWindowDataStorageTreeModel::flags(const QModelIndex& index) const
{
  if (this != index.model())
  {
    return Qt::NoItemFlags;
  }

  if (!index.isValid())
  {
    return Qt::ItemIsDropEnabled;
  }

  auto item = GetItemByIndex(index);
  if (nullptr == item)
  {
    return Qt::NoItemFlags;
  }

  const auto dataNode = item->GetDataNode();
  if (m_NodePredicate.IsNull() || m_NodePredicate->CheckNode(dataNode))
  {
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
  }

  return Qt::NoItemFlags;
}

Qt::DropActions QmitkRenderWindowDataStorageTreeModel::supportedDropActions() const
{
  return Qt::CopyAction | Qt::MoveAction;
}

Qt::DropActions QmitkRenderWindowDataStorageTreeModel::supportedDragActions() const
{
  return Qt::CopyAction | Qt::MoveAction;
}

QStringList QmitkRenderWindowDataStorageTreeModel::mimeTypes() const
{
  QStringList types = QAbstractItemModel::mimeTypes();
  types << QmitkMimeTypes::DataNodePtrs;
  return types;
}

QMimeData* QmitkRenderWindowDataStorageTreeModel::mimeData(const QModelIndexList& indexes) const
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

bool QmitkRenderWindowDataStorageTreeModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int /*row*/, int /*column*/, const QModelIndex& parent)
{
  auto baseRenderer = m_BaseRenderer.Lock();

  if (baseRenderer.IsNull())
  {
    return false;
  }

  if (action == Qt::IgnoreAction)
  {
    return true;
  }

  if (!data->hasFormat(QmitkMimeTypes::DataNodePtrs))
  {
    return false;
  }

  if (!parent.isValid())
  {
    return false;
  }

  int layer = -1;
  auto dataNode = this->data(parent, QmitkDataNodeRawPointerRole).value<mitk::DataNode*>();
  if (nullptr != dataNode)
  {
    dataNode->GetIntProperty("layer", layer, baseRenderer);
  }

  auto dataNodeList = QmitkMimeTypes::ToDataNodePtrList(data);
  for (const auto& dataNode : std::as_const(dataNodeList))
  {
    m_RenderWindowLayerController->MoveNodeToPosition(dataNode, layer, baseRenderer);
  }

  ResetTree();
  UpdateModelData();
  AdjustLayerProperty();
  return true;
}

void QmitkRenderWindowDataStorageTreeModel::SetControlledRenderer(mitk::RenderWindowLayerUtilities::RendererVector controlledRenderer)
{
  ResetTree();
  auto dataStorage = m_DataStorage.Lock();

  if (dataStorage.IsNull())
  {
    return;
  }

  for (const auto& renderer : controlledRenderer)
  {
    if (nullptr == renderer)
    {
      continue;
    }

    auto allDataNodes = dataStorage->GetAll();
    for (const auto& dataNode : *allDataNodes)
    {
      // add the node to each render window
      mitk::RenderWindowLayerUtilities::SetRenderWindowProperties(dataNode, renderer);
    }
  }
}

void QmitkRenderWindowDataStorageTreeModel::SetCurrentRenderer(mitk::BaseRenderer* baseRenderer)
{
  if (m_BaseRenderer == baseRenderer)
  {
    return;
  }

  // base renderer changed
  // reset tree to build a new renderer-specific item hierarchy
  m_BaseRenderer = baseRenderer;
  ResetTree();
  UpdateModelData();
}

mitk::BaseRenderer::Pointer QmitkRenderWindowDataStorageTreeModel::GetCurrentRenderer() const
{
  return m_BaseRenderer.Lock();
}

void QmitkRenderWindowDataStorageTreeModel::ResetTree()
{
  beginResetModel();
  if (nullptr != m_Root)
  {
    m_Root->Delete();
  }

  mitk::DataNode::Pointer rootDataNode = mitk::DataNode::New();
  rootDataNode->SetName("Data Storage");
  m_Root = new QmitkDataStorageTreeModelInternalItem(rootDataNode);
  endResetModel();
}

void QmitkRenderWindowDataStorageTreeModel::UpdateModelData()
{
  auto dataStorage = m_DataStorage.Lock();

  if (dataStorage.IsNotNull())
  {
    auto baseRenderer = m_BaseRenderer.Lock();

    if (baseRenderer.IsNotNull())
    {
      auto allDataNodes = dataStorage->GetAll();
      for (const auto& dataNode : *allDataNodes)
      {
        // add the node to each render window
        mitk::RenderWindowLayerUtilities::SetRenderWindowProperties(dataNode, baseRenderer);
      }

      if (m_NodePredicate.IsNotNull())
      {
        auto filteredDataNodes = dataStorage->GetSubset(m_NodePredicate);
        for (const auto& dataNode : *filteredDataNodes)
        {
          AddNodeInternal(dataNode, baseRenderer);
        }
      }
    }
  }
}

void QmitkRenderWindowDataStorageTreeModel::AdjustLayerProperty()
{
  auto baseRenderer = m_BaseRenderer.Lock();

  if (baseRenderer.IsNull())
  {
    return;
  }

  std::vector<QmitkDataStorageTreeModelInternalItem*> treeAsVector;
  TreeToVector(m_Root, treeAsVector);

  int i = treeAsVector.size() - 1;
  for (auto it = treeAsVector.begin(); it != treeAsVector.end(); ++it)
  {
    auto dataNode = (*it)->GetDataNode();
    dataNode->SetIntProperty("layer", i, baseRenderer);
    --i;
  }
}

void QmitkRenderWindowDataStorageTreeModel::TreeToVector(QmitkDataStorageTreeModelInternalItem* parent, std::vector<QmitkDataStorageTreeModelInternalItem*>& treeAsVector) const
{
  QmitkDataStorageTreeModelInternalItem* item;
  for (int i = 0; i < parent->GetChildCount(); ++i)
  {
    item = parent->GetChild(i);
    TreeToVector(item, treeAsVector);
    treeAsVector.push_back(item);
  }
}

void QmitkRenderWindowDataStorageTreeModel::AddNodeInternal(const mitk::DataNode* dataNode, const mitk::BaseRenderer* renderer)
{
  if (nullptr == dataNode
   || m_DataStorage.IsExpired()
   || nullptr != m_Root->Find(dataNode))
  {
    return;
  }

  // find out if we have a root node
  auto parentItem = m_Root;
  QModelIndex index;
  auto parentDataNode = GetParentNode(dataNode);

  if (nullptr != parentDataNode) // no top level data node
  {
    parentItem = m_Root->Find(parentDataNode);
    if (nullptr == parentItem)
    {
      // parent node not contained in the tree; add it
      NodeAdded(parentDataNode);
      parentItem = m_Root->Find(parentDataNode);
      if (nullptr == parentItem)
      {
        // could not find and add the parent tree; abort
        return;
      }
    }

    // get the index of this parent with the help of the grand parent
    index = createIndex(parentItem->GetIndex(), 0, parentItem);
  }

  const int row = this->GetLayerRow(parentItem, dataNode, renderer);

  beginInsertRows(index, row, row);
  auto newNode = new QmitkDataStorageTreeModelInternalItem(const_cast<mitk::DataNode*>(dataNode));
  parentItem->InsertChild(newNode, row);
  endInsertRows();
}

int QmitkRenderWindowDataStorageTreeModel::GetLayerRow(QmitkDataStorageTreeModelInternalItem* parentItem, const mitk::DataNode* dataNode, const mitk::BaseRenderer* renderer) const
{
  int nodeLayer = -1;
  dataNode->GetIntProperty("layer", nodeLayer, renderer);

  int row = 0;
  for (const auto& siblingItem : parentItem->GetChildren())
  {
    int siblingLayer = -1;
    auto siblingNode = siblingItem->GetDataNode();
    if (nullptr != siblingNode)
    {
      siblingNode->GetIntProperty("layer", siblingLayer, renderer);
    }
    if (nodeLayer > siblingLayer)
    {
      break;
    }
    ++row;
  }

  return row;
}

void QmitkRenderWindowDataStorageTreeModel::RemoveNodeInternal(const mitk::DataNode* dataNode)
{
  if (nullptr == dataNode
   || nullptr == m_Root)
  {
    return;
  }

  auto item = m_Root->Find(dataNode);
  if (nullptr == item)
  {
    return;
  }

  auto parentItem = item->GetParent();
  auto parentIndex = GetIndexByItem(parentItem);

  // The children take over the sources of the node in the data storage, so
  // they move to the item of its first source. A rebuild would add the node
  // again, as it is still in the data storage while this event is handled.
  // Moves keep the expanded state and the selection of the children in views.
  for (auto* child : item->GetChildren())
  {
    this->MoveTreeItem(child, parentItem);
  }

  beginRemoveRows(parentIndex, item->GetIndex(), item->GetIndex());
  parentItem->RemoveChild(item);
  delete item;
  endRemoveRows();
}

mitk::DataNode* QmitkRenderWindowDataStorageTreeModel::GetParentNode(const mitk::DataNode* node) const
{
  mitk::DataNode* dataNode = nullptr;
  auto dataStorage = m_DataStorage.Lock();

  if (dataStorage.IsNull())
  {
    return dataNode;
  }

  auto sources = dataStorage->GetSources(node);
  if (sources->empty())
  {
    return dataNode;
  }

  return sources->front();
}

QmitkDataStorageTreeModelInternalItem* QmitkRenderWindowDataStorageTreeModel::GetItemByIndex(const QModelIndex& index) const
{
  if (index.isValid())
  {
    return static_cast<QmitkDataStorageTreeModelInternalItem*>(index.internalPointer());
  }

  return m_Root;
}

QModelIndex QmitkRenderWindowDataStorageTreeModel::GetIndexByItem(QmitkDataStorageTreeModelInternalItem* item) const
{
  if (item == m_Root)
  {
    return QModelIndex();
  }

  return createIndex(item->GetIndex(), 0, item);
}
