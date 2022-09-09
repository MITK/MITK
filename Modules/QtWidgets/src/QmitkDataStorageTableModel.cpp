/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkDataStorageTableModel.h"

//# Own includes
#include "QmitkCustomVariants.h"
#include "QmitkEnums.h"
#include "mitkNodePredicateBase.h"
#include "mitkProperties.h"
#include "mitkRenderingManager.h"
#include <QmitkNodeDescriptorManager.h>

//# Toolkit includes
#include <QFile>
#include <QIcon>
#include <itkCommand.h>

//#CTORS/DTOR
QmitkDataStorageTableModel::QmitkDataStorageTableModel(mitk::DataStorage::Pointer _DataStorage,
                                                       mitk::NodePredicateBase *_Predicate,
                                                       QObject *parent)
  : QAbstractTableModel(parent),
    m_DataStorage(nullptr),
    m_Predicate(nullptr),
    m_BlockEvents(false),
    m_SortDescending(false)
{
  this->SetPredicate(_Predicate);
  this->SetDataStorage(_DataStorage);
}

QmitkDataStorageTableModel::~QmitkDataStorageTableModel()
{
  // set data storage 0 to remove event listeners
  this->SetDataStorage(nullptr);
}

//# Public GETTER
const mitk::DataStorage::Pointer QmitkDataStorageTableModel::GetDataStorage() const
{
  return m_DataStorage.Lock();
}

mitk::NodePredicateBase::Pointer QmitkDataStorageTableModel::GetPredicate() const
{
  return m_Predicate;
}

mitk::DataNode::Pointer QmitkDataStorageTableModel::GetNode(const QModelIndex &index) const
{
  mitk::DataNode::Pointer node;

  if (index.isValid())
  {
    node = m_NodeSet.at(index.row());
  }

  return node;
}

QVariant QmitkDataStorageTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  QVariant headerData;

  // show only horizontal header
  if (role == Qt::DisplayRole)
  {
    if (orientation == Qt::Horizontal)
    {
      // first column: "Name"
      if (section == 0)
        headerData = "Name";
      else if (section == 1)
        headerData = "Data Type";
      else if (section == 2)
        headerData = "Visibility";
    }
    else if (orientation == Qt::Vertical)
    {
      // show numbers for rows
      headerData = section + 1;
    }
  }

  return headerData;
}

Qt::ItemFlags QmitkDataStorageTableModel::flags(const QModelIndex &index) const
{
  Qt::ItemFlags flags = QAbstractItemModel::flags(index);

  // name & visibility is editable
  if (index.column() == 0)
  {
    flags |= Qt::ItemIsEditable;
  }
  else if (index.column() == 2)
  {
    flags |= Qt::ItemIsUserCheckable;
  }

  return flags;
}

int QmitkDataStorageTableModel::rowCount(const QModelIndex &) const
{
  return m_NodeSet.size();
}

int QmitkDataStorageTableModel::columnCount(const QModelIndex &) const
{
  // show name, type and visible columnn
  int columns = 3;
  return columns;
}

QVariant QmitkDataStorageTableModel::data(const QModelIndex &index, int role) const
{
  QVariant data;

  if (index.isValid() && !m_NodeSet.empty())
  {
    mitk::DataNode::Pointer node = m_NodeSet.at(index.row());

    std::string nodeName = node->GetName();
    if (nodeName.empty())
      nodeName = "unnamed";

    // get name
    if (index.column() == 0)
    {
      // get name of node (may also be edited)
      if (role == Qt::DisplayRole || role == Qt::EditRole)
      {
        data = QString::fromStdString(nodeName);
      }
      else if (role == QmitkDataNodeRole)
      {
        data = QVariant::fromValue(node);
      }
    }
    else if (index.column() == 1)
    {
      QmitkNodeDescriptor *nodeDescriptor = QmitkNodeDescriptorManager::GetInstance()->GetDescriptor(node);

      // get type property of mitk::BaseData
      if (role == Qt::DisplayRole)
      {
        data = nodeDescriptor->GetNameOfClass();
      }
      // show some nice icons for datatype
      else if (role == Qt::DecorationRole)
      {
        data = nodeDescriptor->GetIcon(node);
      }
    }
    else if (index.column() == 2)
    {
      // get visible property of mitk::BaseData
      bool visibility = false;

      if (node->GetVisibility(visibility, nullptr) && role == Qt::CheckStateRole)
      {
        data = (visibility ? Qt::Checked : Qt::Unchecked);
      } // node->GetVisibility(visibility, 0) && role == Qt::CheckStateRole

    } // index.column() == 2

  } // index.isValid() && !m_NodeSet.empty()
  return data;
}

//# Public SETTERS
void QmitkDataStorageTableModel::SetPredicate(mitk::NodePredicateBase *_Predicate)
{
  // ensure that a new predicate is set in order to avoid unnecessary changed events
  if (m_Predicate != _Predicate)
  {
    m_Predicate = _Predicate;
    this->Reset();
  }
}

void QmitkDataStorageTableModel::SetDataStorage(mitk::DataStorage::Pointer _DataStorage)
{
  // only proceed if we have a new datastorage
  if (m_DataStorage != _DataStorage)
  {
    auto dataStorage = m_DataStorage.Lock();

    // if a data storage was set before remove old event listeners
    if (dataStorage.IsNotNull())
    {
      dataStorage->AddNodeEvent.RemoveListener(
        mitk::MessageDelegate1<QmitkDataStorageTableModel, const mitk::DataNode *>(
          this, &QmitkDataStorageTableModel::AddNode));

      dataStorage->RemoveNodeEvent.RemoveListener(
        mitk::MessageDelegate1<QmitkDataStorageTableModel, const mitk::DataNode *>(
          this, &QmitkDataStorageTableModel::RemoveNode));
    }

    // set new data storage
    m_DataStorage = _DataStorage;
    dataStorage = m_DataStorage.Lock();

    // if new storage is not 0 subscribe for events
    if (dataStorage.IsNotNull())
    {
      // subscribe for node added/removed events
      dataStorage->AddNodeEvent.AddListener(
        mitk::MessageDelegate1<QmitkDataStorageTableModel, const mitk::DataNode *>(
          this, &QmitkDataStorageTableModel::AddNode));

      dataStorage->RemoveNodeEvent.AddListener(
        mitk::MessageDelegate1<QmitkDataStorageTableModel, const mitk::DataNode *>(
          this, &QmitkDataStorageTableModel::RemoveNode));
    }

    // Reset model (even if datastorage is 0->will be checked in Reset())
    this->Reset();
  }
}

void QmitkDataStorageTableModel::AddNode(const mitk::DataNode *node)
{
  // garantuee no recursions when a new node event is thrown
  if (!m_BlockEvents)
  {
    // if we have a predicate, check node against predicate first
    if (m_Predicate.IsNotNull() && !m_Predicate->CheckNode(node))
      return;

    // dont add nodes without data (formerly known as helper objects)
    if (node->GetData() == nullptr)
      return;

    // create listener commands to listen to changes in the name or the visibility of the node
    itk::MemberCommand<QmitkDataStorageTableModel>::Pointer propertyModifiedCommand =
      itk::MemberCommand<QmitkDataStorageTableModel>::New();
    propertyModifiedCommand->SetCallbackFunction(this, &QmitkDataStorageTableModel::PropertyModified);

    mitk::BaseProperty *tempProperty = nullptr;

    // add listener for properties
    tempProperty = node->GetProperty("visible");
    if (tempProperty)
      m_VisiblePropertyModifiedObserverTags[tempProperty] =
        tempProperty->AddObserver(itk::ModifiedEvent(), propertyModifiedCommand);

    tempProperty = node->GetProperty("name");
    if (tempProperty)
      m_NamePropertyModifiedObserverTags[tempProperty] =
        tempProperty->AddObserver(itk::ModifiedEvent(), propertyModifiedCommand);

    // emit beginInsertRows event
    beginInsertRows(QModelIndex(), m_NodeSet.size(), m_NodeSet.size());

    // add node
    m_NodeSet.push_back(const_cast<mitk::DataNode *>(node));

    // emit endInsertRows event
    endInsertRows();
  }
}

void QmitkDataStorageTableModel::RemoveNode(const mitk::DataNode *node)
{
  // garantuee no recursions when a new node event is thrown
  if (!m_BlockEvents)
  {
    // find corresponding node
    auto nodeIt = std::find(m_NodeSet.begin(), m_NodeSet.end(), node);

    if (nodeIt != m_NodeSet.end())
    {
      // now: remove listeners for name property ...
      mitk::BaseProperty *tempProperty = nullptr;

      tempProperty = (*nodeIt)->GetProperty("visible");
      if (tempProperty)
        tempProperty->RemoveObserver(m_VisiblePropertyModifiedObserverTags[tempProperty]);
      m_VisiblePropertyModifiedObserverTags.erase(tempProperty);

      // ... and visibility property
      tempProperty = (*nodeIt)->GetProperty("name");
      if (tempProperty)
        tempProperty->RemoveObserver(m_NamePropertyModifiedObserverTags[tempProperty]);
      m_NamePropertyModifiedObserverTags.erase(tempProperty);

      // get an index from iterator
      int row = std::distance(m_NodeSet.begin(), nodeIt);

      // emit beginRemoveRows event (QModelIndex is empty because we dont have a tree model)
      this->beginRemoveRows(QModelIndex(), row, row);

      // remove node
      m_NodeSet.erase(nodeIt);

      // emit endRemoveRows event
      endRemoveRows();
    }
  }
}

void QmitkDataStorageTableModel::PropertyModified(const itk::Object *caller, const itk::EventObject &)
{
  if (!m_BlockEvents)
  {
    // get modified property
    const mitk::BaseProperty *modifiedProperty = dynamic_cast<const mitk::BaseProperty *>(caller);

    if (modifiedProperty)
    {
      // find node that holds the modified property
      int row = -1;
      int column = -1;

      std::vector<mitk::DataNode *>::iterator it;
      mitk::BaseProperty *visibilityProperty = nullptr;
      mitk::BaseProperty *nameProperty = nullptr;

      // search for property that changed and emit datachanged on the corresponding ModelIndex
      for (it = m_NodeSet.begin(); it != m_NodeSet.end(); it++)
      {
        // check for the visible property or the name property
        visibilityProperty = (*it)->GetProperty("visible");
        if (modifiedProperty == visibilityProperty)
        {
          column = 2;
          break;
        }

        nameProperty = (*it)->GetProperty("name");
        if (modifiedProperty == nameProperty)
        {
          column = 0;
          break;
        }
      }

      // if we have the property we have a valid iterator
      if (it != m_NodeSet.end())
        row = std::distance(m_NodeSet.begin(), it);

      // now emit the dataChanged signal
      QModelIndex indexOfChangedProperty = index(row, column);
      emit dataChanged(indexOfChangedProperty, indexOfChangedProperty);
    }
  }
}

bool QmitkDataStorageTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  bool noErr = false;

  if (index.isValid() && (role == Qt::EditRole || role == Qt::CheckStateRole))
  {
    // any change events produced here should not be caught in this class
    // --> set m_BlockEvents to true
    m_BlockEvents = true;

    mitk::DataNode::Pointer node = m_NodeSet.at(index.row());

    if (index.column() == 0)
    {
      node->SetName(value.toString().toStdString());
    }
    else if (index.column() == 2)
    {
      node->SetBoolProperty("visible", (value.toInt() == Qt::Checked ? true : false));
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }

    // inform listeners about changes
    emit dataChanged(index, index);

    m_BlockEvents = false;
    noErr = true;
  }

  return noErr;
}

//#Protected SETTER
void QmitkDataStorageTableModel::Reset()
{
  mitk::DataStorage::SetOfObjects::ConstPointer _NodeSet;

  // remove all nodes now (dont use iterators because removing elements
  // would invalidate the iterator)
  // start at the last element: first in, last out
  unsigned int i = m_NodeSet.size();
  while (!m_NodeSet.empty())
  {
    --i;
    this->RemoveNode(m_NodeSet.at(i));
  }

  // normally now everything should be empty->just to be sure
  // erase all arrays again
  m_NamePropertyModifiedObserverTags.clear();
  m_VisiblePropertyModifiedObserverTags.clear();
  m_NodeSet.clear();

  auto dataStorage = m_DataStorage.Lock();

  // the whole reset depends on the fact if a data storage is set or not
  if (dataStorage.IsNotNull())
  {
    _NodeSet = m_Predicate.IsNotNull()
      ? dataStorage->GetSubset(m_Predicate)
      : dataStorage->GetAll();

    // finally add all nodes to the model
    for (auto it = _NodeSet->begin(); it != _NodeSet->end(); it++)
    {
      // save node
      this->AddNode(*it);
    }
  }
}

void QmitkDataStorageTableModel::sort(int column, Qt::SortOrder order /*= Qt::AscendingOrder */)
{
  bool sortDescending = (order == Qt::DescendingOrder) ? true : false;

  // do not sort twice !!! (dont know why, but qt calls this func twice. STUPID!)
  /*
    if(sortDescending != m_SortDescending)
    {*/

  // m_SortDescending = sortDescending;

  DataNodeCompareFunction::CompareCriteria _CompareCriteria = DataNodeCompareFunction::CompareByName;

  DataNodeCompareFunction::CompareOperator _CompareOperator =
    sortDescending ? DataNodeCompareFunction::Greater : DataNodeCompareFunction::Less;

  if (column == 1)
    _CompareCriteria = DataNodeCompareFunction::CompareByClassName;

  else if (column == 2)
    _CompareCriteria = DataNodeCompareFunction::CompareByVisibility;

  DataNodeCompareFunction compareFunc(_CompareCriteria, _CompareOperator);
  std::sort(m_NodeSet.begin(), m_NodeSet.end(), compareFunc);

  QAbstractTableModel::beginResetModel();
  QAbstractTableModel::endResetModel();
  //}
}

std::vector<mitk::DataNode *> QmitkDataStorageTableModel::GetNodeSet() const
{
  return m_NodeSet;
}

QmitkDataStorageTableModel::DataNodeCompareFunction::DataNodeCompareFunction(CompareCriteria _CompareCriteria,
                                                                             CompareOperator _CompareOperator)
  : m_CompareCriteria(_CompareCriteria), m_CompareOperator(_CompareOperator)
{
}

bool QmitkDataStorageTableModel::DataNodeCompareFunction::operator()(const mitk::DataNode::Pointer &_Left,
                                                                     const mitk::DataNode::Pointer &_Right) const
{
  switch (m_CompareCriteria)
  {
    case CompareByClassName:
      if (m_CompareOperator == Less)
        return (_Left->GetData()->GetNameOfClass() < _Right->GetData()->GetNameOfClass());
      else
        return (_Left->GetData()->GetNameOfClass() > _Right->GetData()->GetNameOfClass());
      break;

    case CompareByVisibility:
    {
      bool _LeftVisibility = false;
      bool _RightVisibility = false;
      _Left->GetVisibility(_LeftVisibility, nullptr);
      _Right->GetVisibility(_RightVisibility, nullptr);

      if (m_CompareOperator == Less)
        return (_LeftVisibility < _RightVisibility);
      else
        return (_LeftVisibility > _RightVisibility);
    }
    break;

    // CompareByName:
    default:
      if (m_CompareOperator == Less)
        return (_Left->GetName() < _Right->GetName());
      else
        return (_Left->GetName() > _Right->GetName());
      break;
  }
}
