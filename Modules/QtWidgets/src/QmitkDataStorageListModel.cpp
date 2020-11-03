/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkDataStorageListModel.h"

//# Own includes
// mitk
#include "mitkStringProperty.h"

//# Toolkit includes
// itk
#include "itkCommand.h"

QmitkDataStorageListModel::QmitkDataStorageListModel(mitk::DataStorage *dataStorage,
                                                     mitk::NodePredicateBase::Pointer pred,
                                                     QObject *parent)
  : QAbstractListModel(parent)
  , m_NodePredicate(nullptr)
  , m_DataStorage(nullptr)
  , m_BlockEvents(false)
{
  this->SetPredicate(pred);
  this->SetDataStorage(dataStorage);
}

QmitkDataStorageListModel::~QmitkDataStorageListModel()
{
  // set data storage to nullptr so that the event listener gets removed
  this->SetDataStorage(nullptr);
}

void QmitkDataStorageListModel::SetDataStorage(mitk::DataStorage::Pointer dataStorage)
{
  if (m_DataStorage == dataStorage)
  {
    return;
  }

  // remove old listeners
  if (m_DataStorage != nullptr)
  {
    m_DataStorage->AddNodeEvent.RemoveListener(
      mitk::MessageDelegate1<QmitkDataStorageListModel, const mitk::DataNode *>(
        this, &QmitkDataStorageListModel::OnDataStorageNodeAdded));

    m_DataStorage->RemoveNodeEvent.RemoveListener(
      mitk::MessageDelegate1<QmitkDataStorageListModel, const mitk::DataNode *>(
        this, &QmitkDataStorageListModel::OnDataStorageNodeRemoved));

    m_DataStorage->RemoveObserver(m_DataStorageDeleteObserverTag);
    m_DataStorageDeleteObserverTag = 0;
  }

  m_DataStorage = dataStorage;

  if (m_DataStorage != nullptr)
  {
    // subscribe for node added/removed events
    m_DataStorage->AddNodeEvent.AddListener(mitk::MessageDelegate1<QmitkDataStorageListModel, const mitk::DataNode *>(
      this, &QmitkDataStorageListModel::OnDataStorageNodeAdded));

    m_DataStorage->RemoveNodeEvent.AddListener(
      mitk::MessageDelegate1<QmitkDataStorageListModel, const mitk::DataNode *>(
        this, &QmitkDataStorageListModel::OnDataStorageNodeRemoved));

    // add ITK delete listener on data storage
    itk::MemberCommand<QmitkDataStorageListModel>::Pointer deleteCommand =
      itk::MemberCommand<QmitkDataStorageListModel>::New();
    deleteCommand->SetCallbackFunction(this, &QmitkDataStorageListModel::OnDataStorageDeleted);
    m_DataStorageDeleteObserverTag = m_DataStorage->AddObserver(itk::DeleteEvent(), deleteCommand);
  }

  // reset/rebuild model
  reset();
}

Qt::ItemFlags QmitkDataStorageListModel::flags(const QModelIndex &) const
{
  return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant QmitkDataStorageListModel::data(const QModelIndex &index, int role) const
{
  if (role == Qt::DisplayRole && index.isValid())
  {
    const mitk::DataNode *node = std::get<NODE>(m_NodesAndObserverTags.at(index.row()));
    return QVariant(QString::fromStdString(node->GetName()));
  }
  else
  {
    return QVariant();
  }
}

QVariant QmitkDataStorageListModel::headerData(int /*section*/, Qt::Orientation /*orientation*/, int /*role*/) const
{
  return QVariant(tr("Nodes"));
}

int QmitkDataStorageListModel::rowCount(const QModelIndex & /*parent*/) const
{
  return m_NodesAndObserverTags.size();
}

std::vector<mitk::DataNode *> QmitkDataStorageListModel::GetDataNodes() const
{
  auto size = m_NodesAndObserverTags.size();
  std::vector<mitk::DataNode *> result(size);
  for (std::size_t i = 0; i < size; ++i)
  {
    result[i] = std::get<NODE>(m_NodesAndObserverTags[i]);
  }
  return result;
}

mitk::DataStorage *QmitkDataStorageListModel::GetDataStorage() const
{
  return m_DataStorage;
}

void QmitkDataStorageListModel::SetPredicate(mitk::NodePredicateBase *pred)
{
  m_NodePredicate = pred;

  // in a prior implementation the call to beginResetModel() been after reset().
  // Should this actually be the better order of calls, please document!
  QAbstractListModel::beginResetModel();
  reset();
  QAbstractListModel::endResetModel();
}

mitk::NodePredicateBase *QmitkDataStorageListModel::GetPredicate() const
{
  return m_NodePredicate;
}

void QmitkDataStorageListModel::reset()
{
  mitk::DataStorage::SetOfObjects::ConstPointer modelNodes;

  if (m_DataStorage != nullptr)
  {
    if (m_NodePredicate != nullptr)
    {
      modelNodes = m_DataStorage->GetSubset(m_NodePredicate);
    }
    else
    {
      modelNodes = m_DataStorage->GetAll();
    }
  }

  ClearInternalNodeList();

  // add all filtered nodes to our list
  if (modelNodes != nullptr)
  {
    for (auto &node : *modelNodes)
    {
      AddNodeToInternalList(node);
    }
  }
}

void QmitkDataStorageListModel::AddNodeToInternalList(mitk::DataNode *node)
{
  if (m_DataStorage != nullptr)
  {
    itk::MemberCommand<QmitkDataStorageListModel>::Pointer nodeModifiedCommand;
    // add modified observer
    nodeModifiedCommand = itk::MemberCommand<QmitkDataStorageListModel>::New();
    nodeModifiedCommand->SetCallbackFunction(this, &QmitkDataStorageListModel::OnDataNodeModified);
    unsigned long nodeObserverTag = node->AddObserver(itk::ModifiedEvent(), nodeModifiedCommand);

    itk::MemberCommand<QmitkDataStorageListModel>::Pointer dataModifiedCommand;
    unsigned long dataObserverTag = 0;
    // add modified observer
    if (node->GetData() != nullptr)
    {
      dataModifiedCommand = itk::MemberCommand<QmitkDataStorageListModel>::New();
      dataModifiedCommand->SetCallbackFunction(this, &QmitkDataStorageListModel::OnDataModified);
      dataObserverTag = node->GetData()->AddObserver(itk::ModifiedEvent(), dataModifiedCommand);
    }

    m_NodesAndObserverTags.push_back(std::make_tuple(node, nodeObserverTag, dataObserverTag));
  }
}

void QmitkDataStorageListModel::ClearInternalNodeList()
{
  for (auto &iter : m_NodesAndObserverTags)
  {
    auto node = std::get<NODE>(iter);
    if (node != nullptr)
    {
      node->RemoveObserver(std::get<NODE_OBSERVER>(iter));
      if (node->GetData() != nullptr)
      {
        node->GetData()->RemoveObserver(std::get<DATA_OBSERVER>(iter));
      }
    }
  }
  m_NodesAndObserverTags.clear();
}

void QmitkDataStorageListModel::RemoveNodeFromInternalList(mitk::DataNode *node)
{
  for (auto iter = m_NodesAndObserverTags.begin(); iter != m_NodesAndObserverTags.end(); ++iter)
  {
    if (std::get<NODE>(*iter) == node)
    {
      node->RemoveObserver(std::get<NODE_OBSERVER>(*iter));
      if (node->GetData() != nullptr)
      {
        node->GetData()->RemoveObserver(std::get<DATA_OBSERVER>(*iter));
      }
      m_NodesAndObserverTags.erase(iter); // invalidate iter
      break;
    }
  }
}

void QmitkDataStorageListModel::OnDataStorageNodeAdded(const mitk::DataNode *node)
{
  // guarantee no recursions when a new node event is thrown
  if (!m_BlockEvents)
  {
    m_BlockEvents = true;

    // check if node should be added to the model
    bool addNode = true;
    if (m_NodePredicate && !m_NodePredicate->CheckNode(node))
      addNode = false;

    if (addNode)
    {
      int newIndex = m_NodesAndObserverTags.size();
      beginInsertRows(QModelIndex(), newIndex, newIndex);
      AddNodeToInternalList(const_cast<mitk::DataNode *>(node));
      endInsertRows();
    }

    m_BlockEvents = false;
  }
}

void QmitkDataStorageListModel::OnDataStorageNodeRemoved(const mitk::DataNode *node)
{
  // guarantee no recursions when a new node event is thrown
  if (!m_BlockEvents)
  {
    m_BlockEvents = true;

    int row = 0;
    for (auto iter = m_NodesAndObserverTags.begin(); iter != m_NodesAndObserverTags.end(); ++iter, ++row)
    {
      if (std::get<NODE>(*iter) == node)
      {
        // node found, remove it
        beginRemoveRows(QModelIndex(), row, row);
        RemoveNodeFromInternalList(std::get<NODE>(*iter));
        endRemoveRows();
        break;
      }
    }
  }

  m_BlockEvents = false;
}

void QmitkDataStorageListModel::OnDataNodeModified(const itk::Object *caller, const itk::EventObject & /*event*/)
{
  if (m_BlockEvents)
    return;

  const mitk::DataNode *modifiedNode = dynamic_cast<const mitk::DataNode *>(caller);
  if (modifiedNode)
  {
    QModelIndex changedIndex = getIndex(modifiedNode);
    if (changedIndex.isValid())
    {
      emit dataChanged(changedIndex, changedIndex);
    }
  }
}

void QmitkDataStorageListModel::OnDataModified(const itk::Object *caller, const itk::EventObject &event)
{
  OnDataNodeModified(caller, event); // until different implementation
}

void QmitkDataStorageListModel::OnDataStorageDeleted(const itk::Object *, const itk::EventObject &)
{
  if (m_BlockEvents)
    return;

  this->SetDataStorage(nullptr);
}

mitk::DataNode::Pointer QmitkDataStorageListModel::getNode(const QModelIndex &index) const
{
  if (index.isValid())
  {
    return std::get<NODE>(m_NodesAndObserverTags.at(index.row()));
  }
  else
  {
    return nullptr;
  }
}

QModelIndex QmitkDataStorageListModel::getIndex(const mitk::DataNode *node) const
{
  int row = 0;
  for (auto iter = m_NodesAndObserverTags.begin(); iter != m_NodesAndObserverTags.end(); ++iter, ++row)
  {
    if (std::get<NODE>(*iter) == node)
    {
      return index(row);
    }
  }
  return QModelIndex();
}
