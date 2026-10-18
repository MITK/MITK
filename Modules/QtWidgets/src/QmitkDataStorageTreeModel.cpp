/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateData.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateOr.h>
#include <mitkNodePredicateProperty.h>
#include <mitkPlanarFigure.h>
#include <mitkProperties.h>
#include <mitkRenderingManager.h>
#include <mitkStringProperty.h>

#include <mitkPropertyNameHelper.h>

#include <QmitkDataStorageTreeModel.h>
#include <QmitkDataStorageTreeModelInternalItem.h>
#include <QmitkNodeDescriptorManager.h>
#include <QmitkCustomVariants.h>
#include <QmitkEnums.h>
#include <QmitkMimeTypes.h>

#include <QFile>
#include <QIcon>
#include <QMimeData>
#include <QTextStream>

#include <algorithm>
#include <iterator>
#include <map>

#include <mitkCoreServices.h>

QmitkDataStorageTreeModel::QmitkDataStorageTreeModel(mitk::DataStorage *_DataStorage,
                                                     bool _PlaceNewNodesOnTop,
                                                     QObject *parent)
  : QAbstractItemModel(parent),
    m_DataStorage(nullptr),
    m_PlaceNewNodesOnTop(_PlaceNewNodesOnTop),
    m_Root(nullptr),
    m_AllowHierarchyChange(false)
{
  this->SetDataStorage(_DataStorage);
}

QmitkDataStorageTreeModel::~QmitkDataStorageTreeModel()
{
  // set data storage to 0 = remove all listeners
  this->SetDataStorage(nullptr);
  m_Root->Delete();
  m_Root = nullptr;
}

mitk::DataNode::Pointer QmitkDataStorageTreeModel::GetNode(const QModelIndex &index) const
{
  return this->TreeItemFromIndex(index)->GetDataNode();
}

const mitk::DataStorage::Pointer QmitkDataStorageTreeModel::GetDataStorage() const
{
  return m_DataStorage.Lock();
}

QModelIndex QmitkDataStorageTreeModel::index(int row, int column, const QModelIndex &parent) const
{
  TreeItem *parentItem;

  if (!parent.isValid())
    parentItem = m_Root;
  else
    parentItem = static_cast<TreeItem *>(parent.internalPointer());

  TreeItem *childItem = parentItem->GetChild(row);
  if (childItem)
    return createIndex(row, column, childItem);
  else
    return QModelIndex();
}

int QmitkDataStorageTreeModel::rowCount(const QModelIndex &parent) const
{
  TreeItem *parentTreeItem = this->TreeItemFromIndex(parent);
  return parentTreeItem->GetChildCount();
}

Qt::ItemFlags QmitkDataStorageTreeModel::flags(const QModelIndex &index) const
{
  if (index.isValid())
  {
    return Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable |
           Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
  }
  else
  {
    return Qt::ItemIsDropEnabled;
  }
}

int QmitkDataStorageTreeModel::columnCount(const QModelIndex & /* parent = QModelIndex() */) const
{
  return 1;
}

QModelIndex QmitkDataStorageTreeModel::parent(const QModelIndex &index) const
{
  if (!index.isValid())
    return QModelIndex();

  TreeItem *childItem = this->TreeItemFromIndex(index);
  TreeItem *parentItem = childItem->GetParent();

  if (parentItem == m_Root)
    return QModelIndex();

  return this->createIndex(parentItem->GetIndex(), 0, parentItem);
}

QmitkDataStorageTreeModel::TreeItem *QmitkDataStorageTreeModel::TreeItemFromIndex(const QModelIndex &index) const
{
  if (index.isValid())
    return static_cast<TreeItem *>(index.internalPointer());
  else
    return m_Root;
}
Qt::DropActions QmitkDataStorageTreeModel::supportedDropActions() const
{
  return Qt::CopyAction | Qt::MoveAction;
}

Qt::DropActions QmitkDataStorageTreeModel::supportedDragActions() const
{
  return Qt::CopyAction | Qt::MoveAction;
}

bool QmitkDataStorageTreeModel::dropMimeData(
  const QMimeData *data, Qt::DropAction action, int row, int /*column*/, const QModelIndex &parent)
{
  // Early exit, returning true, but not actually doing anything (ignoring data).
  if (action == Qt::IgnoreAction)
  {
    return true;
  }

  // Note, we are returning true if we handled it, and false otherwise
  bool returnValue = false;

  if (data->hasFormat("application/x-qabstractitemmodeldatalist"))
  {
    returnValue = true;

    // First we extract a Qlist of TreeItem* pointers.
    QList<TreeItem *> listOfItemsToDrop = ToTreeItemPtrList(data);
    if (listOfItemsToDrop.empty())
    {
      return false;
    }

    // Retrieve the TreeItem* where we are dropping stuff, and its parent.
    TreeItem *dropItem = this->TreeItemFromIndex(parent);
    TreeItem *parentItem = dropItem->GetParent();

    // If item was dropped onto empty space, we select the root node
    if (dropItem == m_Root)
    {
      parentItem = m_Root;
    }

    // Dragging and Dropping is only allowed within the same parent, so use the first item in list to validate.
    // (otherwise, you could have a derived image such as a segmentation, and assign it to another image).
    // NOTE: We are assuming the input list is valid... i.e. when it was dragged, all the items had the same parent.

    // Determine whether or not the drag and drop operation is a valid one.
    // Examples of invalid operations include:
    //  - dragging nodes with different parents
    //  - dragging nodes from one parent to another parent, if m_AllowHierarchyChange is false
    //  - dragging a node on one of its child nodes (only relevant if m_AllowHierarchyChange is true)

    bool isValidDragAndDropOperation(true);

    // different parents
    {
      TreeItem *firstParent = listOfItemsToDrop[0]->GetParent();
      QList<TreeItem *>::iterator diIter;
      for (diIter = listOfItemsToDrop.begin() + 1; diIter != listOfItemsToDrop.end(); diIter++)
      {
        if (firstParent != (*diIter)->GetParent())
        {
          isValidDragAndDropOperation = false;
          break;
        }
      }
    }

    // dragging from one parent to another
    if ((!m_AllowHierarchyChange) && isValidDragAndDropOperation)
    {
      if (row == -1) // drag onto a node
      {
        isValidDragAndDropOperation = listOfItemsToDrop[0]->GetParent() == parentItem;
      }
      else // drag between nodes
      {
        isValidDragAndDropOperation = listOfItemsToDrop[0]->GetParent() == dropItem;
      }
    }

    // dragging on a child node of one the dragged nodes
    {
      QList<TreeItem *>::iterator diIter;
      for (diIter = listOfItemsToDrop.begin(); diIter != listOfItemsToDrop.end(); diIter++)
      {
        TreeItem *tempItem = dropItem;

        while (tempItem != m_Root)
        {
          tempItem = tempItem->GetParent();
          if (tempItem == *diIter)
          {
            isValidDragAndDropOperation = false;
          }
        }
      }
    }

    // dragging on one of the dragged nodes
    if (listOfItemsToDrop.contains(dropItem))
      isValidDragAndDropOperation = false;

    auto dataStorage = m_DataStorage.Lock();

    // Dragging on a node derived from one of the dragged nodes, also through a
    // source the tree does not show, as the data storage would refuse it.
    if (m_AllowHierarchyChange && isValidDragAndDropOperation && dropItem != m_Root && dataStorage.IsNotNull())
    {
      for (TreeItem *const itemToDrop : listOfItemsToDrop)
      {
        const auto derivations = dataStorage->GetDerivations(itemToDrop->GetDataNode(), nullptr, false);
        if (std::find(derivations->begin(), derivations->end(), dropItem->GetDataNode()) != derivations->end())
          isValidDragAndDropOperation = false;
      }
    }

    if (!isValidDragAndDropOperation)
      return isValidDragAndDropOperation;

    TreeItem *const insertTargetItem = m_AllowHierarchyChange ? dropItem : (row == -1 ? parentItem : dropItem);

    // The drop position is computed among the children of the insert target
    // that stay where they are, as if the dragged items were taken out first.
    std::vector<TreeItem *> remainingChildren;
    for (TreeItem *child : insertTargetItem->GetChildren())
    {
      if (!listOfItemsToDrop.contains(child))
        remainingChildren.push_back(child);
    }

    const int remainingCount = static_cast<int>(remainingChildren.size());

    // row = -1 dropped on an item, row != -1 dropped in between two items.
    int dropIndex = 0;
    if (row != -1)
    {
      // The drop row counts the dragged items above it, which do not stay.
      int itemsAboveDropRow = 0;
      for (TreeItem *const itemToDrop : listOfItemsToDrop)
      {
        if ((itemToDrop->GetIndex() < row) && (itemToDrop->GetParent() == dropItem))
          ++itemsAboveDropRow;
      }

      dropIndex = std::min(row - itemsAboveDropRow, remainingCount);
    }
    else if (m_AllowHierarchyChange)
    {
      // Reparent onto dropItem: append as new children.
      dropIndex = remainingCount;
    }
    else
    {
      // Sibling reorder onto dropItem: take its position among its siblings.
      const auto dropItemIt = std::find(remainingChildren.begin(), remainingChildren.end(), dropItem);
      dropIndex = static_cast<int>(std::distance(remainingChildren.begin(), dropItemIt));
    }

    // A drop onto empty space appends to the top-level items.
    if (row == -1 && dropItem == m_Root)
      dropIndex = remainingCount;

    dropIndex = std::max(0, dropIndex);

    // Each dragged item goes before the child that follows the dropped items,
    // so they keep their order among each other.
    TreeItem *const nextItem = dropIndex < remainingCount ? remainingChildren[dropIndex] : nullptr;

    for (TreeItem *itemToDrop : listOfItemsToDrop)
    {
      // A reorder keeps all sources of a node, also those the tree does not show.
      const bool changesParent = itemToDrop->GetParent() != insertTargetItem;

      this->MoveTreeItem(itemToDrop, insertTargetItem, nextItem);

      if (m_AllowHierarchyChange && changesParent && dataStorage.IsNotNull())
      {
        // The root item holds a placeholder node that is not in the data storage.
        auto sources = mitk::DataStorage::SetOfObjects::New();
        if (dropItem != m_Root)
          sources->push_back(dropItem->GetDataNode());

        // Removing the node and adding it again would detach the nodes derived
        // from it, which the tree keeps below it. The item is in place already,
        // so UpdateNodeParent() leaves it there.
        dataStorage->SetSources(itemToDrop->GetDataNode(), sources);
      }
    }

    // Change Layers to match.
    this->AdjustLayerProperty();
  }
  else if (data->hasFormat("application/x-mitk-datanodes"))
  {
    returnValue = true;

    int numberOfNodesDropped = 0;

    QList<mitk::DataNode *> dataNodeList = QmitkMimeTypes::ToDataNodePtrList(data);
    mitk::DataNode *node = nullptr;
    foreach (node, dataNodeList)
    {
      auto datastorage = m_DataStorage.Lock();
      if (node && datastorage.IsNotNull() && !datastorage->Exists(node))
      {
        m_DataStorage.Lock()->Add(node);
        mitk::BaseData::Pointer basedata = node->GetData();

        if (basedata.IsNotNull())
        {
          mitk::RenderingManager::GetInstance()->InitializeViews(basedata->GetTimeGeometry());

          numberOfNodesDropped++;
        }
      }
    }
    // Only do a rendering update, if we actually dropped anything.
    if (numberOfNodesDropped > 0)
    {
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
  }

  return returnValue;
}

QStringList QmitkDataStorageTreeModel::mimeTypes() const
{
  QStringList types = QAbstractItemModel::mimeTypes();
  types << "application/x-qabstractitemmodeldatalist";
  types << "application/x-mitk-datanodes";
  return types;
}

QMimeData *QmitkDataStorageTreeModel::mimeData(const QModelIndexList &indexes) const
{
  return mimeDataFromModelIndexList(indexes);
}

QMimeData *QmitkDataStorageTreeModel::mimeDataFromModelIndexList(const QModelIndexList &indexes)
{
  QMimeData *ret = new QMimeData;

  QString treeItemAddresses("");
  QString dataNodeAddresses("");

  QByteArray baTreeItemPtrs;
  QByteArray baDataNodePtrs;

  QDataStream dsTreeItemPtrs(&baTreeItemPtrs, QIODevice::WriteOnly);
  QDataStream dsDataNodePtrs(&baDataNodePtrs, QIODevice::WriteOnly);

  for (int i = 0; i < indexes.size(); i++)
  {
    TreeItem *treeItem = static_cast<TreeItem *>(indexes.at(i).internalPointer());

    dsTreeItemPtrs << reinterpret_cast<quintptr>(treeItem);
    dsDataNodePtrs << reinterpret_cast<quintptr>(treeItem->GetDataNode().GetPointer());

    // --------------- deprecated -----------------
    unsigned long long treeItemAddress = reinterpret_cast<unsigned long long>(treeItem);
    unsigned long long dataNodeAddress = reinterpret_cast<unsigned long long>(treeItem->GetDataNode().GetPointer());
    QTextStream(&treeItemAddresses) << treeItemAddress;
    QTextStream(&dataNodeAddresses) << dataNodeAddress;

    if (i != indexes.size() - 1)
    {
      QTextStream(&treeItemAddresses) << ",";
      QTextStream(&dataNodeAddresses) << ",";
    }
    // -------------- end deprecated -------------
  }

  // ------------------ deprecated -----------------
  ret->setData("application/x-qabstractitemmodeldatalist", QByteArray(treeItemAddresses.toLatin1()));
  ret->setData("application/x-mitk-datanodes", QByteArray(dataNodeAddresses.toLatin1()));
  // --------------- end deprecated -----------------

  ret->setData(QmitkMimeTypes::DataStorageTreeItemPtrs, baTreeItemPtrs);
  ret->setData(QmitkMimeTypes::DataNodePtrs, baDataNodePtrs);

  return ret;
}

QVariant QmitkDataStorageTreeModel::data(const QModelIndex &index, int role) const
{
  mitk::DataNode *dataNode = this->TreeItemFromIndex(index)->GetDataNode();

  // get name of treeItem (may also be edited)
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
  else if (role == Qt::CheckStateRole)
  {
    return dataNode->IsVisible(nullptr);
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

bool QmitkDataStorageTreeModel::DicomPropertiesExists(const mitk::DataNode &node) const
{
  bool propertiesExists = false;

  mitk::BaseProperty *seriesDescription_deprecated = (node.GetProperty("dicom.series.SeriesDescription"));
  mitk::BaseProperty *studyDescription_deprecated = (node.GetProperty("dicom.study.StudyDescription"));
  mitk::BaseProperty *patientsName_deprecated = (node.GetProperty("dicom.patient.PatientsName"));
  mitk::BaseProperty *seriesDescription =
    (node.GetProperty(mitk::GeneratePropertyNameForDICOMTag(0x0008, 0x103e).c_str()));
  mitk::BaseProperty *studyDescription =
    (node.GetProperty(mitk::GeneratePropertyNameForDICOMTag(0x0008, 0x1030).c_str()));
  mitk::BaseProperty *patientsName = (node.GetProperty(mitk::GeneratePropertyNameForDICOMTag(0x0010, 0x0010).c_str()));

  if (patientsName != nullptr && studyDescription != nullptr && seriesDescription != nullptr)
  {
    if ((!patientsName->GetValueAsString().empty()) && (!studyDescription->GetValueAsString().empty()) &&
        (!seriesDescription->GetValueAsString().empty()))
    {
      propertiesExists = true;
    }
  }

  /** Code covers the deprecated property naming for backwards compatibility */
  if (patientsName_deprecated != nullptr && studyDescription_deprecated != nullptr && seriesDescription_deprecated != nullptr)
  {
    if ((!patientsName_deprecated->GetValueAsString().empty()) &&
        (!studyDescription_deprecated->GetValueAsString().empty()) &&
        (!seriesDescription_deprecated->GetValueAsString().empty()))
    {
      propertiesExists = true;
    }
  }

  return propertiesExists;
}

QVariant QmitkDataStorageTreeModel::headerData(int /*section*/, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole && m_Root)
    return QString::fromStdString(m_Root->GetDataNode()->GetName());

  return QVariant();
}

void QmitkDataStorageTreeModel::SetDataStorage(mitk::DataStorage *_DataStorage)
{
  if (m_DataStorage != _DataStorage) // dont take the same again
  {
    auto dataStorage = m_DataStorage.Lock();
    if (dataStorage.IsNotNull())
    {
      // remove Listener for the data storage itself
      dataStorage->RemoveObserver(m_DataStorageDeletedTag);

      // remove listeners for the nodes
      dataStorage->AddNodeEvent.RemoveListener(
        mitk::MessageDelegate1<QmitkDataStorageTreeModel, const mitk::DataNode *>(this,
                                                                                  &QmitkDataStorageTreeModel::AddNode));

      dataStorage->ChangedNodeEvent.RemoveListener(
        mitk::MessageDelegate1<QmitkDataStorageTreeModel, const mitk::DataNode *>(
          this, &QmitkDataStorageTreeModel::SetNodeModified));

      dataStorage->RemoveNodeEvent.RemoveListener(
        mitk::MessageDelegate1<QmitkDataStorageTreeModel, const mitk::DataNode *>(
          this, &QmitkDataStorageTreeModel::RemoveNode));

      dataStorage->SourcesChangedNodeEvent.RemoveListener(
        mitk::MessageDelegate1<QmitkDataStorageTreeModel, const mitk::DataNode *>(
          this, &QmitkDataStorageTreeModel::UpdateNodeParent));
    }

    this->beginResetModel();

    // take over the new data storage
    m_DataStorage = _DataStorage;

    // delete the old root (if necessary, create new)
    if (m_Root)
      m_Root->Delete();
    mitk::DataNode::Pointer rootDataNode = mitk::DataNode::New();
    rootDataNode->SetName("Data Manager");
    m_Root = new TreeItem(rootDataNode, nullptr);

    dataStorage = m_DataStorage.Lock();
    if (dataStorage.IsNotNull())
    {
      // add Listener for the data storage itself
      auto command = itk::SimpleMemberCommand<QmitkDataStorageTreeModel>::New();
      command->SetCallbackFunction(this, &QmitkDataStorageTreeModel::SetDataStorageDeleted);
      m_DataStorageDeletedTag = dataStorage->AddObserver(itk::DeleteEvent(), command);

      // add listeners for the nodes
      dataStorage->AddNodeEvent.AddListener(mitk::MessageDelegate1<QmitkDataStorageTreeModel, const mitk::DataNode *>(
        this, &QmitkDataStorageTreeModel::AddNode));

      dataStorage->ChangedNodeEvent.AddListener(
        mitk::MessageDelegate1<QmitkDataStorageTreeModel, const mitk::DataNode *>(
          this, &QmitkDataStorageTreeModel::SetNodeModified));

      dataStorage->RemoveNodeEvent.AddListener(
        mitk::MessageDelegate1<QmitkDataStorageTreeModel, const mitk::DataNode *>(
          this, &QmitkDataStorageTreeModel::RemoveNode));

      dataStorage->SourcesChangedNodeEvent.AddListener(
        mitk::MessageDelegate1<QmitkDataStorageTreeModel, const mitk::DataNode *>(
          this, &QmitkDataStorageTreeModel::UpdateNodeParent));

      // finally add all nodes to the model
      this->Update();
    }

    this->endResetModel();
  }
}

void QmitkDataStorageTreeModel::SetDataStorageDeleted()
{
  this->SetDataStorage(nullptr);
}

void QmitkDataStorageTreeModel::AddNodeInternal(const mitk::DataNode *node)
{
  auto dataStorage = m_DataStorage.Lock();

  if (node == nullptr || dataStorage.IsNull() || !dataStorage->Exists(node) || m_Root->Find(node) != nullptr)
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
      this->AddNode(parentDataNode);
      parentTreeItem = m_Root->Find(parentDataNode);
      if (!parentTreeItem)
        return;
    }

    // get the index of this parent with the help of the grand parent
    index = this->createIndex(parentTreeItem->GetIndex(), 0, parentTreeItem);
  }

  // add node
  const int row = this->GetNewChildRow(parentTreeItem, node);
  this->beginInsertRows(index, row, row);
  parentTreeItem->InsertChild(new TreeItem(const_cast<mitk::DataNode *>(node)), row);
  this->endInsertRows();

  if(m_PlaceNewNodesOnTop)
  {
    this->AdjustLayerProperty();
  }
}

int QmitkDataStorageTreeModel::GetNewChildRow(TreeItem *parentItem, const mitk::DataNode *node) const
{
  if (m_PlaceNewNodesOnTop)
    return 0;

  int nodeLayer = -1;
  node->GetIntProperty("layer", nodeLayer);

  int row = 0;
  for (TreeItem *siblingTreeItem : parentItem->GetChildren())
  {
    int siblingLayer = -1;
    if (mitk::DataNode *siblingNode = siblingTreeItem->GetDataNode())
      siblingNode->GetIntProperty("layer", siblingLayer);

    if (nodeLayer > siblingLayer)
      break;

    ++row;
  }

  return row;
}

void QmitkDataStorageTreeModel::AddNode(const mitk::DataNode *node)
{
  auto dataStorage = m_DataStorage.Lock();

  if (node == nullptr || dataStorage.IsNull() || !dataStorage->Exists(node) ||
      m_Root->Find(node) != nullptr)
    return;

  this->AddNodeInternal(node);
}

void QmitkDataStorageTreeModel::SetPlaceNewNodesOnTop(bool _PlaceNewNodesOnTop)
{
  m_PlaceNewNodesOnTop = _PlaceNewNodesOnTop;
}

void QmitkDataStorageTreeModel::RemoveNodeInternal(const mitk::DataNode *node)
{
  if (!m_Root)
    return;

  TreeItem *treeItem = m_Root->Find(node);
  if (!treeItem)
    return; // return because there is no treeitem containing this node

  TreeItem *parentTreeItem = treeItem->GetParent();

  // The children take over the sources of the node in the data storage, so
  // they move to the item of its first source.
  for (TreeItem *child : treeItem->GetChildren())
    this->MoveTreeItem(child, parentTreeItem);

  this->beginRemoveRows(this->IndexFromTreeItem(parentTreeItem), treeItem->GetIndex(), treeItem->GetIndex());
  delete treeItem;
  this->endRemoveRows();

  this->AdjustLayerProperty();
}

void QmitkDataStorageTreeModel::RemoveNode(const mitk::DataNode *node)
{
  if (node == nullptr)
    return;

  this->RemoveNodeInternal(node);
}

void QmitkDataStorageTreeModel::UpdateNodeParent(const mitk::DataNode *node)
{
  if (node == nullptr || m_Root == nullptr)
    return;

  TreeItem *treeItem = m_Root->Find(node);
  if (treeItem == nullptr)
    return;

  TreeItem *parentTreeItem = m_Root;

  if (mitk::DataNode *parentDataNode = this->GetParentNode(node); parentDataNode != nullptr)
  {
    parentTreeItem = m_Root->Find(parentDataNode);
    if (parentTreeItem == nullptr)
    {
      this->AddNode(parentDataNode);
      parentTreeItem = m_Root->Find(parentDataNode);
      if (parentTreeItem == nullptr)
        return;
    }
  }

  if (parentTreeItem == treeItem->GetParent())
    return;

  // The item goes where a new node would, so a node that code gives a new
  // parent is drawn as if it had been added there.
  const int row = this->GetNewChildRow(parentTreeItem, node);
  const auto children = parentTreeItem->GetChildren();
  this->MoveTreeItem(treeItem, parentTreeItem, row < static_cast<int>(children.size()) ? children[row] : nullptr);
  this->AdjustLayerProperty();
}

void QmitkDataStorageTreeModel::MoveTreeItem(TreeItem *item, TreeItem *newParentItem, TreeItem *nextItem)
{
  TreeItem *oldParentItem = item->GetParent();
  const int oldRow = item->GetIndex();

  // Rows before the move, as beginMoveRows() expects them.
  const int newRow = nextItem != nullptr ? nextItem->GetIndex() : newParentItem->GetChildCount();

  // beginMoveRows() refuses to move an item to where it already is.
  if (oldParentItem == newParentItem && (newRow == oldRow || newRow == oldRow + 1))
    return;

  if (!this->beginMoveRows(
        this->IndexFromTreeItem(oldParentItem), oldRow, oldRow, this->IndexFromTreeItem(newParentItem), newRow))
  {
    return;
  }

  oldParentItem->RemoveChild(item);
  newParentItem->InsertChild(item, oldParentItem == newParentItem && oldRow < newRow ? newRow - 1 : newRow);

  this->endMoveRows();
}

void QmitkDataStorageTreeModel::SetNodeModified(const mitk::DataNode *node)
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

mitk::DataNode *QmitkDataStorageTreeModel::GetParentNode(const mitk::DataNode *node) const
{
  mitk::DataNode *dataNode = nullptr;

  mitk::DataStorage::SetOfObjects::ConstPointer _Sources = m_DataStorage.Lock()->GetSources(node);

  if (_Sources->Size() > 0)
    dataNode = _Sources->front();

  return dataNode;
}

bool QmitkDataStorageTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  mitk::DataNode *dataNode = this->TreeItemFromIndex(index)->GetDataNode();
  if (!dataNode)
    return false;

  if (role == Qt::EditRole && !value.toString().isEmpty())
  {
    dataNode->SetName(value.toString().toStdString());

    mitk::PlanarFigure *planarFigure = dynamic_cast<mitk::PlanarFigure *>(dataNode->GetData());

    if (planarFigure != nullptr)
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
  else if (role == Qt::CheckStateRole)
  {
    // Please note: value.toInt() returns 2, independentely from the actual checkstate of the index element.
    // Therefore the checkstate is being estimated again here.

    QVariant qcheckstate = index.data(Qt::CheckStateRole);
    int checkstate = qcheckstate.toInt();
    bool isVisible = bool(checkstate);
    dataNode->SetVisibility(!isVisible);
    emit nodeVisibilityChanged();
  }
  // inform listeners about changes
  emit dataChanged(index, index);
  return true;
}

bool QmitkDataStorageTreeModel::setHeaderData(int /*section*/,
                                              Qt::Orientation /*orientation*/,
                                              const QVariant & /* value */,
                                              int /*role = Qt::EditRole*/)
{
  return false;
}

void QmitkDataStorageTreeModel::AdjustLayerProperty()
{
  /// transform the tree into an array and set the layer property descending
  std::vector<TreeItem *> vec;
  this->TreeToVector(m_Root, vec);

  int i = vec.size() - 1;
  for (std::vector<TreeItem *>::const_iterator it = vec.begin(); it != vec.end(); ++it)
  {
    mitk::DataNode::Pointer dataNode = (*it)->GetDataNode();
    bool fixedLayer = false;

    if (!(dataNode->GetBoolProperty("fixedLayer", fixedLayer) && fixedLayer))
      dataNode->SetIntProperty("layer", i);

    --i;
  }
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkDataStorageTreeModel::TreeToVector(TreeItem *parent, std::vector<TreeItem *> &vec) const
{
  TreeItem *current;
  for (int i = 0; i < parent->GetChildCount(); ++i)
  {
    current = parent->GetChild(i);
    this->TreeToVector(current, vec);
    vec.push_back(current);
  }
}

QModelIndex QmitkDataStorageTreeModel::IndexFromTreeItem(TreeItem *item) const
{
  if (item == m_Root)
    return QModelIndex();
  else
    return this->createIndex(item->GetIndex(), 0, item);
}

QList<mitk::DataNode::Pointer> QmitkDataStorageTreeModel::GetNodeSet() const
{
  QList<mitk::DataNode::Pointer> res;
  if (m_Root)
    this->TreeToNodeSet(m_Root, res);

  return res;
}

void QmitkDataStorageTreeModel::TreeToNodeSet(TreeItem *parent, QList<mitk::DataNode::Pointer> &vec) const
{
  TreeItem *current;
  for (int i = 0; i < parent->GetChildCount(); ++i)
  {
    current = parent->GetChild(i);
    vec.push_back(current->GetDataNode());
    this->TreeToNodeSet(current, vec);
  }
}

QModelIndex QmitkDataStorageTreeModel::GetIndex(const mitk::DataNode *node) const
{
  if (m_Root)
  {
    TreeItem *item = m_Root->Find(node);
    if (item)
      return this->IndexFromTreeItem(item);
  }
  return QModelIndex();
}

QList<QmitkDataStorageTreeModel::TreeItem *> QmitkDataStorageTreeModel::ToTreeItemPtrList(const QMimeData *mimeData)
{
  if (mimeData == nullptr || !mimeData->hasFormat(QmitkMimeTypes::DataStorageTreeItemPtrs))
  {
    return QList<TreeItem *>();
  }
  return ToTreeItemPtrList(mimeData->data(QmitkMimeTypes::DataStorageTreeItemPtrs));
}

QList<QmitkDataStorageTreeModel::TreeItem *> QmitkDataStorageTreeModel::ToTreeItemPtrList(const QByteArray &ba)
{
  QList<TreeItem *> result;
  QDataStream ds(ba);
  while (!ds.atEnd())
  {
    quintptr treeItemPtr;
    ds >> treeItemPtr;
    result.push_back(reinterpret_cast<TreeItem *>(treeItemPtr));
  }
  return result;
}

void QmitkDataStorageTreeModel::Update()
{
  auto datastorage = m_DataStorage.Lock();
  if (datastorage.IsNotNull())
  {
    mitk::DataStorage::SetOfObjects::ConstPointer _NodeSet = datastorage->GetAll();

    /// Regardless the value of this preference, the new nodes must not be inserted
    /// at the top now, but at the position according to their layer.
    bool newNodesWereToBePlacedOnTop = m_PlaceNewNodesOnTop;
    m_PlaceNewNodesOnTop = false;

    for (const auto& node : *_NodeSet)
    {
      this->AddNodeInternal(node);
    }

    m_PlaceNewNodesOnTop = newNodesWereToBePlacedOnTop;

    /// Adjust the layers to ensure that derived nodes are above their sources.
    this->AdjustLayerProperty();
  }
}

void QmitkDataStorageTreeModel::SetAllowHierarchyChange(bool allowHierarchyChange)
{
  m_AllowHierarchyChange = allowHierarchyChange;
}
