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
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateData.h>
#include <mitkNodePredicateFirstLevel.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateOr.h>
#include <mitkNodePredicateProperty.h>
#include <mitkPlanarFigure.h>
#include <mitkProperties.h>
#include <mitkRenderingManager.h>
#include <mitkStringProperty.h>

#include <mitkPropertyNameHelper.h>

#include "QmitkDataStorageTreeModel.h"
#include "QmitkDataStorageTreeModelInternalItem.h"
#include "QmitkNodeDescriptorManager.h"
#include <QmitkCustomVariants.h>
#include <QmitkEnums.h>
#include <QmitkMimeTypes.h>

#include <QFile>
#include <QIcon>
#include <QMimeData>
#include <QTextStream>

#include <map>

#include <mitkCoreServices.h>

QmitkDataStorageTreeModel::QmitkDataStorageTreeModel(mitk::DataStorage *_DataStorage,
                                                     bool _PlaceNewNodesOnTop,
                                                     QObject *parent)
  : QAbstractItemModel(parent),
    m_DataStorage(nullptr),
    m_PlaceNewNodesOnTop(_PlaceNewNodesOnTop),
    m_Root(nullptr),
    m_BlockDataStorageEvents(false),
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

    if (!isValidDragAndDropOperation)
      return isValidDragAndDropOperation;

    if (listOfItemsToDrop[0] != dropItem && isValidDragAndDropOperation)
    {
      // Retrieve the index of where we are dropping stuff.
      QModelIndex parentModelIndex = this->IndexFromTreeItem(parentItem);

      int dragIndex = 0;

      // Iterate through the list of TreeItem (which may be at non-consecutive indexes).
      QList<TreeItem *>::iterator diIter;
      for (diIter = listOfItemsToDrop.begin(); diIter != listOfItemsToDrop.end(); diIter++)
      {
        TreeItem *itemToDrop = *diIter;

        // if the item is dragged down we have to compensate its final position for the
        // fact it is deleted lateron, this only applies if it is dragged within the same level
        if ((itemToDrop->GetIndex() < row) && (itemToDrop->GetParent() == dropItem))
        {
          dragIndex = 1;
        }

        // Here we assume that as you remove items, one at a time, that GetIndex() will be valid.
        this->beginRemoveRows(
          this->IndexFromTreeItem(itemToDrop->GetParent()), itemToDrop->GetIndex(), itemToDrop->GetIndex());
        itemToDrop->GetParent()->RemoveChild(itemToDrop);
        this->endRemoveRows();
      }

      // row = -1 dropped on an item, row != -1 dropped  in between two items
      // Select the target index position, or put it at the end of the list.
      int dropIndex = 0;
      if (row != -1)
      {
        if (dragIndex == 0)
          dropIndex = std::min(row, parentItem->GetChildCount() - 1);
        else
          dropIndex = std::min(row - 1, parentItem->GetChildCount() - 1);
      }
      else
      {
        dropIndex = dropItem->GetIndex();
      }

      QModelIndex dropItemModelIndex = this->IndexFromTreeItem(dropItem);
      if ((row == -1 && dropItemModelIndex.row() == -1) || dropItemModelIndex.row() > parentItem->GetChildCount())
        dropIndex = parentItem->GetChildCount() - 1;

      // Now insert items again at the drop item position

      if (m_AllowHierarchyChange)
      {
        this->beginInsertRows(dropItemModelIndex, dropIndex, dropIndex + listOfItemsToDrop.size() - 1);
      }
      else
      {
        this->beginInsertRows(parentModelIndex, dropIndex, dropIndex + listOfItemsToDrop.size() - 1);
      }

      for (diIter = listOfItemsToDrop.begin(); diIter != listOfItemsToDrop.end(); diIter++)
      {
        // dropped on node, behaviour depends on preference setting
        if (m_AllowHierarchyChange)
        {
          auto dataStorage = m_DataStorage.Lock();

          m_BlockDataStorageEvents = true;
          mitk::DataNode *droppedNode = (*diIter)->GetDataNode();
          mitk::DataNode *dropOntoNode = dropItem->GetDataNode();
          dataStorage->Remove(droppedNode);
          dataStorage->Add(droppedNode, dropOntoNode);
          m_BlockDataStorageEvents = false;

          dropItem->InsertChild((*diIter), dropIndex);
        }
        else
        {
          if (row == -1) // drag onto a node
          {
            parentItem->InsertChild((*diIter), dropIndex);
          }
          else // drag between nodes
          {
            dropItem->InsertChild((*diIter), dropIndex);
          }
        }

        dropIndex++;
      }
      this->endInsertRows();

      // Change Layers to match.
      this->AdjustLayerProperty();
    }
  }
  else if (data->hasFormat("application/x-mitk-datanodes"))
  {
    returnValue = true;

    int numberOfNodesDropped = 0;

    QList<mitk::DataNode *> dataNodeList = QmitkMimeTypes::ToDataNodePtrList(data);
    mitk::DataNode *node = nullptr;
    foreach (node, dataNodeList)
    {
      if (node && !m_DataStorage.IsExpired() && !m_DataStorage.Lock()->Exists(node))
      {
        m_DataStorage.Lock()->Add(node);
        mitk::BaseData::Pointer basedata = node->GetData();

        if (basedata.IsNotNull())
        {
          mitk::RenderingManager::GetInstance()->InitializeViews(
            basedata->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true);

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

  /** Code coveres the deprecated property naming for backwards compatibility */
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
    if (!m_DataStorage.IsExpired())
    {
      auto dataStorage = m_DataStorage.Lock();

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
    }

    // take over the new data storage
    m_DataStorage = _DataStorage;

    // delete the old root (if necessary, create new)
    if (m_Root)
      m_Root->Delete();
    mitk::DataNode::Pointer rootDataNode = mitk::DataNode::New();
    rootDataNode->SetName("Data Manager");
    m_Root = new TreeItem(rootDataNode, nullptr);
    this->beginResetModel();
    this->endResetModel();

    if (!m_DataStorage.IsExpired())
    {
      auto dataStorage = m_DataStorage.Lock();

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

      mitk::DataStorage::SetOfObjects::ConstPointer _NodeSet = dataStorage->GetSubset(m_Predicate);

      // finally add all nodes to the model
      this->Update();
    }
  }
}

void QmitkDataStorageTreeModel::SetDataStorageDeleted()
{
  this->SetDataStorage(nullptr);
}

void QmitkDataStorageTreeModel::AddNodeInternal(const mitk::DataNode *node)
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
      this->AddNode(parentDataNode);
      parentTreeItem = m_Root->Find(parentDataNode);
      if (!parentTreeItem)
        return;
    }

    // get the index of this parent with the help of the grand parent
    index = this->createIndex(parentTreeItem->GetIndex(), 0, parentTreeItem);
  }

  // add node
  if (m_PlaceNewNodesOnTop)
  {
    // emit beginInsertRows event
    beginInsertRows(index, 0, 0);
    parentTreeItem->InsertChild(new TreeItem(const_cast<mitk::DataNode *>(node)), 0);
  }
  else
  {
    int firstRowWithASiblingBelow = 0;
    int nodeLayer = -1;
    node->GetIntProperty("layer", nodeLayer);
    for (TreeItem* siblingTreeItem: parentTreeItem->GetChildren())
    {
      int siblingLayer = -1;
      if (mitk::DataNode* siblingNode = siblingTreeItem->GetDataNode())
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
    parentTreeItem->InsertChild(new TreeItem(const_cast<mitk::DataNode*>(node)), firstRowWithASiblingBelow);
  }

  // emit endInsertRows event
  endInsertRows();

  if(m_PlaceNewNodesOnTop)
  {
    this->AdjustLayerProperty();
  }
}

void QmitkDataStorageTreeModel::AddNode(const mitk::DataNode *node)
{
  if (node == nullptr || m_BlockDataStorageEvents || m_DataStorage.IsExpired() || !m_DataStorage.Lock()->Exists(node) ||
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
  QModelIndex parentIndex = this->IndexFromTreeItem(parentTreeItem);

  // emit beginRemoveRows event (QModelIndex is empty because we dont have a tree model)
  this->beginRemoveRows(parentIndex, treeItem->GetIndex(), treeItem->GetIndex());

  // remove node
  std::vector<TreeItem *> children = treeItem->GetChildren();
  delete treeItem;

  // emit endRemoveRows event
  endRemoveRows();

  // move all children of deleted node into its parent
  for (std::vector<TreeItem *>::iterator it = children.begin(); it != children.end(); it++)
  {
    // emit beginInsertRows event
    beginInsertRows(parentIndex, parentTreeItem->GetChildCount(), parentTreeItem->GetChildCount());

    // add nodes again
    parentTreeItem->AddChild(*it);

    // emit endInsertRows event
    endInsertRows();
  }

  this->AdjustLayerProperty();
}

void QmitkDataStorageTreeModel::RemoveNode(const mitk::DataNode *node)
{
  if (node == nullptr || m_BlockDataStorageEvents)
    return;

  this->RemoveNodeInternal(node);
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
    dataNode->SetStringProperty("name", value.toString().toStdString().c_str());

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
  if (!m_DataStorage.IsExpired())
  {
    mitk::DataStorage::SetOfObjects::ConstPointer _NodeSet = m_DataStorage.Lock()->GetAll();

    /// Regardless the value of this preference, the new nodes must not be inserted
    /// at the top now, but at the position according to their layer.
    bool newNodesWereToBePlacedOnTop = m_PlaceNewNodesOnTop;
    m_PlaceNewNodesOnTop = false;

    for (const auto& node: *_NodeSet)
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
