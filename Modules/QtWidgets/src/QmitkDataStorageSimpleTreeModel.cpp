/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkDataStorageSimpleTreeModel.h>
#include <QmitkDataStorageTreeModelInternalItem.h>
#include <QmitkNodeDescriptorManager.h>

// qt widgets module
#include "QmitkCustomVariants.h"
#include "QmitkEnums.h"

QmitkDataStorageSimpleTreeModel::QmitkDataStorageSimpleTreeModel(QObject *parent)
  : QmitkAbstractDataStorageModel(parent)
  , m_Root(nullptr)
{
  ResetTree();
}

QmitkDataStorageSimpleTreeModel::~QmitkDataStorageSimpleTreeModel()
{
  m_Root->Delete();
  m_Root = nullptr;
}

void QmitkDataStorageSimpleTreeModel::ResetTree()
{
  mitk::DataNode::Pointer rootDataNode = mitk::DataNode::New();
  rootDataNode->SetName("Data Storage");
  m_Root = new TreeItem(rootDataNode, nullptr);
}

void QmitkDataStorageSimpleTreeModel::DataStorageChanged()
{
  if (m_Root)
  {
    m_Root->Delete();
  }

  beginResetModel();
  ResetTree();
  UpdateModelData();
  endResetModel();
}

void QmitkDataStorageSimpleTreeModel::NodePredicateChanged()
{
  beginResetModel();
  ResetTree();
  UpdateModelData();
  endResetModel();
}

void QmitkDataStorageSimpleTreeModel::NodeAdded(const mitk::DataNode *node)
{
  if (node == nullptr || m_DataStorage.IsExpired() || !m_DataStorage.Lock()->Exists(node) ||
      m_Root->Find(node) != nullptr)
    return;

  this->AddNodeInternal(node);
}

void QmitkDataStorageSimpleTreeModel::NodeChanged(const mitk::DataNode *node)
{
  TreeItem *treeItem = m_Root->Find(node);
  if (treeItem)
  {
    TreeItem *parentTreeItem = treeItem->GetParent();
    // as the root node should not be removed one should always have a parent item
    if (!parentTreeItem)
      return;
    QModelIndex index = this->createIndex(treeItem->GetIndex(), 0, treeItem);

    // now emit the dataChanged signal
    emit dataChanged(index, index);
  }
}

void QmitkDataStorageSimpleTreeModel::NodeRemoved(const mitk::DataNode *node)
{
  if (node == nullptr || !m_Root)
    return;

  TreeItem *treeItem = m_Root->Find(node);
  if (!treeItem)
    return; // return because there is no treeitem containing this node

  TreeItem *parentTreeItem = treeItem->GetParent();
  QModelIndex parentIndex = this->IndexFromTreeItem(parentTreeItem);

  // emit beginRemoveRows event (QModelIndex is empty because we dont have a tree model)
  this->beginRemoveRows(parentIndex, treeItem->GetIndex(), treeItem->GetIndex());

  // remove node
  std::vector<TreeItem *> children = treeItem->GetChildren();
  m_TreeItems.remove(treeItem);
  delete treeItem; //delete in tree

  if (!children.empty())
  {
    //if not empty we have to rebuild the whole representation,
    //because the children could be now top level, or at another
    //source/parent.
    this->UpdateModelData();
  }
}

QModelIndex QmitkDataStorageSimpleTreeModel::index(int row, int column, const QModelIndex &parent) const
{
  TreeItem *parentItem;

  if (!parent.isValid() || parent.model() != this)
    parentItem = m_Root;
  else
    parentItem = static_cast<TreeItem *>(parent.internalPointer());

  if (parentItem)
  {
    TreeItem *childItem = parentItem->GetChild(row);
    if (childItem)
      return createIndex(row, column, childItem);
  }

  return QModelIndex();
}

QModelIndex QmitkDataStorageSimpleTreeModel::parent(const QModelIndex &child) const
{
  if (!child.isValid() || !m_Root || child.model() != this)
    return QModelIndex();

  TreeItem *childItem = this->TreeItemFromIndex(child);

  if (!childItem)
    return QModelIndex();

  TreeItem *parentItem = childItem->GetParent();

  if (parentItem == m_Root)
    return QModelIndex();

  return this->createIndex(parentItem->GetIndex(), 0, parentItem);
}

QmitkDataStorageSimpleTreeModel::TreeItem *QmitkDataStorageSimpleTreeModel::TreeItemFromIndex(
  const QModelIndex &index) const
{
  if (index.isValid() && index.model() == this)
  {
    auto item = static_cast<TreeItem *>(index.internalPointer());
    auto finding = std::find(std::begin(m_TreeItems), std::end(m_TreeItems), item);
    if (finding == std::end(m_TreeItems))
    {
      return nullptr;
    }
    return item;
  }
  else
    return m_Root;
}

int QmitkDataStorageSimpleTreeModel::rowCount(const QModelIndex &parent) const
{
  TreeItem *parentTreeItem = this->TreeItemFromIndex(parent);
  if (parentTreeItem)
    return parentTreeItem->GetChildCount();
  else
    return 0;
}

int QmitkDataStorageSimpleTreeModel::columnCount(const QModelIndex &/*parent*/) const
{
  return 1;
}

QVariant QmitkDataStorageSimpleTreeModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid() || index.model() != this)
  {
    return QVariant();
  }

  auto treeItem = this->TreeItemFromIndex(index);
  if (!treeItem)
    return QVariant();

  mitk::DataNode *dataNode = treeItem->GetDataNode();

  QString nodeName = QString::fromStdString(dataNode->GetName());
  if (nodeName.isEmpty())
  {
    nodeName = "unnamed";
  }

  if (role == Qt::DisplayRole)
    return nodeName;
  else if (role == Qt::ToolTipRole)
    return nodeName;
  else if (role == Qt::DecorationRole)
  {
    QmitkNodeDescriptor *nodeDescriptor = QmitkNodeDescriptorManager::GetInstance()->GetDescriptor(dataNode);
    return nodeDescriptor->GetIcon(dataNode);
  }
  else if (role == QmitkDataNodeRole)
  {
    return QVariant::fromValue<mitk::DataNode::Pointer>(mitk::DataNode::Pointer(dataNode));
  }
  else if (role == QmitkDataNodeRawPointerRole)
  {
    return QVariant::fromValue<mitk::DataNode *>(dataNode);
  }

  return QVariant();
}

bool QmitkDataStorageSimpleTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  if (!index.isValid() || index.model() != this)
    return false;

  auto treeItem = this->TreeItemFromIndex(index);
  if (!treeItem)
    return false;

  mitk::DataNode *dataNode = treeItem->GetDataNode();
  if (!dataNode)
    return false;

  if (role == Qt::EditRole && !value.toString().isEmpty())
  {
    dataNode->SetName(value.toString().toStdString().c_str());
  }
  else if (role == Qt::CheckStateRole)
  {
    // Please note: value.toInt() returns 2, independentely from the actual checkstate of the index element.
    // Therefore the checkstate is being estimated again here.

    QVariant qcheckstate = index.data(Qt::CheckStateRole);
    int checkstate = qcheckstate.toInt();
    bool isVisible = bool(checkstate);
    dataNode->SetVisibility(!isVisible);
  }
  // inform listeners about changes
  emit dataChanged(index, index);
  return true;
}

QVariant QmitkDataStorageSimpleTreeModel::headerData(int /*section*/, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole && m_Root)
    return QString::fromStdString(m_Root->GetDataNode()->GetName());

  return QVariant();
}

Qt::ItemFlags QmitkDataStorageSimpleTreeModel::flags(const QModelIndex &index) const
{
  if (index.isValid() && index.model() == this)
  {
    auto treeItem = this->TreeItemFromIndex(index);
    if (!treeItem)
      return Qt::NoItemFlags;

    const auto dataNode = treeItem->GetDataNode();
    if (m_NodePredicate.IsNull() || m_NodePredicate->CheckNode(dataNode))
    {
      return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
    }
    else
    {
      return Qt::NoItemFlags;
    }

  }

  return Qt::NoItemFlags;
}

mitk::DataNode *QmitkDataStorageSimpleTreeModel::GetParentNode(const mitk::DataNode *node) const
{
  mitk::DataNode *dataNode = nullptr;

  mitk::DataStorage::SetOfObjects::ConstPointer _Sources = m_DataStorage.Lock()->GetSources(node);

  if (_Sources->Size() > 0)
    dataNode = _Sources->front();

  return dataNode;
}

void QmitkDataStorageSimpleTreeModel::AddNodeInternal(const mitk::DataNode *node)
{
  if (node == nullptr || m_DataStorage.IsExpired() || !m_DataStorage.Lock()->Exists(node) || m_Root->Find(node) != nullptr)
    return;

  // find out if we have a root node
  TreeItem *parentTreeItem = m_Root;
  QModelIndex index;
  mitk::DataNode *parentDataNode = this->GetParentNode(node);

  if (parentDataNode) // no top level data node
  {
    parentTreeItem = m_Root->Find(parentDataNode); // find the corresponding tree item
    if (!parentTreeItem)
    {
      this->NodeAdded(parentDataNode);
      parentTreeItem = m_Root->Find(parentDataNode);
      if (!parentTreeItem)
        return;
    }

    // get the index of this parent with the help of the grand parent
    index = this->createIndex(parentTreeItem->GetIndex(), 0, parentTreeItem);
  }

  int firstRowWithASiblingBelow = 0;
  int nodeLayer = -1;
  node->GetIntProperty("layer", nodeLayer);
  for (TreeItem *siblingTreeItem : parentTreeItem->GetChildren())
  {
    int siblingLayer = -1;
    if (mitk::DataNode *siblingNode = siblingTreeItem->GetDataNode())
    {
      siblingNode->GetIntProperty("layer", siblingLayer);
    }
    if (nodeLayer > siblingLayer)
    {
      break;
    }
    ++firstRowWithASiblingBelow;
  }
  beginInsertRows(index, firstRowWithASiblingBelow, firstRowWithASiblingBelow);
  auto newNode = new TreeItem(const_cast<mitk::DataNode *>(node));
  parentTreeItem->InsertChild(newNode, firstRowWithASiblingBelow);
  m_TreeItems.push_back(newNode);

  endInsertRows();
}

QModelIndex QmitkDataStorageSimpleTreeModel::IndexFromTreeItem(TreeItem *item) const
{
  if (item == m_Root)
    return QModelIndex();
  else
    return this->createIndex(item->GetIndex(), 0, item);
}

void QmitkDataStorageSimpleTreeModel::UpdateModelData()
{
  if (!m_DataStorage.IsExpired())
  {
    auto nodeset = m_DataStorage.Lock()->GetAll();
    if (m_NodePredicate != nullptr)
    {
      nodeset = m_DataStorage.Lock()->GetSubset(m_NodePredicate);
    }

    for (const auto& node : *nodeset)
    {
      this->AddNodeInternal(node);
    }
  }
}
