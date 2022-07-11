/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkAbstractDataStorageModel.h>

QmitkAbstractDataStorageModel::QmitkAbstractDataStorageModel(QObject* parent/* = nullptr*/)
  : QAbstractItemModel(parent)
  , m_DataStorage(nullptr)
  , m_NodePredicate(nullptr)
{
  // nothing here
}

QmitkAbstractDataStorageModel::QmitkAbstractDataStorageModel(mitk::DataStorage* dataStorage, QObject* parent/* = nullptr*/)
  : QAbstractItemModel(parent)
  , m_DataStorage(nullptr)
  , m_NodePredicate(nullptr)
{
  SetDataStorage(dataStorage);
}

QmitkAbstractDataStorageModel::~QmitkAbstractDataStorageModel()
{
  auto dataStorage = m_DataStorage.Lock();

  if (dataStorage.IsNotNull())
  {
    // remove Listener for the data storage itself
    dataStorage->RemoveObserver(m_DataStorageDeletedTag);

    // remove listener from data storage
    dataStorage->AddNodeEvent.RemoveListener(
      mitk::MessageDelegate1<QmitkAbstractDataStorageModel, const mitk::DataNode*>(this, &QmitkAbstractDataStorageModel::NodeAdded));
    dataStorage->RemoveNodeEvent.RemoveListener(
      mitk::MessageDelegate1<QmitkAbstractDataStorageModel, const mitk::DataNode*>(this, &QmitkAbstractDataStorageModel::NodeRemoved));
    dataStorage->ChangedNodeEvent.RemoveListener(
      mitk::MessageDelegate1<QmitkAbstractDataStorageModel, const mitk::DataNode*>(this, &QmitkAbstractDataStorageModel::NodeChanged));
  }
}

void QmitkAbstractDataStorageModel::SetDataStorage(mitk::DataStorage* dataStorage)
{
  if (m_DataStorage == dataStorage)
  {
    return;
  }

  auto lockedDataStorage = m_DataStorage.Lock();

  if (lockedDataStorage.IsNotNull())
  {
    // remove Listener for the data storage itself
    lockedDataStorage->RemoveObserver(m_DataStorageDeletedTag);

    // remove listener from old data storage
    lockedDataStorage->AddNodeEvent.RemoveListener(
      mitk::MessageDelegate1<QmitkAbstractDataStorageModel, const mitk::DataNode*>(this, &QmitkAbstractDataStorageModel::NodeAdded));
    lockedDataStorage->RemoveNodeEvent.RemoveListener(
      mitk::MessageDelegate1<QmitkAbstractDataStorageModel, const mitk::DataNode*>(this, &QmitkAbstractDataStorageModel::NodeRemoved));
    lockedDataStorage->ChangedNodeEvent.RemoveListener(
      mitk::MessageDelegate1<QmitkAbstractDataStorageModel, const mitk::DataNode*>(this, &QmitkAbstractDataStorageModel::NodeChanged));
  }

  m_DataStorage = dataStorage;
  lockedDataStorage = m_DataStorage.Lock();

  if (lockedDataStorage.IsNotNull())
  {
    // add Listener for the data storage itself
    auto command = itk::SimpleMemberCommand<QmitkAbstractDataStorageModel>::New();
    command->SetCallbackFunction(this, &QmitkAbstractDataStorageModel::SetDataStorageDeleted);
    m_DataStorageDeletedTag = lockedDataStorage->AddObserver(itk::DeleteEvent(), command);

    // add listener for new data storage
    lockedDataStorage->AddNodeEvent.AddListener(
      mitk::MessageDelegate1<QmitkAbstractDataStorageModel, const mitk::DataNode*>(this, &QmitkAbstractDataStorageModel::NodeAdded));
    lockedDataStorage->RemoveNodeEvent.AddListener(
      mitk::MessageDelegate1<QmitkAbstractDataStorageModel, const mitk::DataNode*>(this, &QmitkAbstractDataStorageModel::NodeRemoved));
    lockedDataStorage->ChangedNodeEvent.AddListener(
      mitk::MessageDelegate1<QmitkAbstractDataStorageModel, const mitk::DataNode*>(this, &QmitkAbstractDataStorageModel::NodeChanged));
  }
  // update model if the data storage has been changed
  DataStorageChanged();
}

mitk::DataStorage::Pointer QmitkAbstractDataStorageModel::GetDataStorage() const
{
  return m_DataStorage.Lock();
}

void QmitkAbstractDataStorageModel::SetDataStorageDeleted()
{
  this->SetDataStorage(nullptr);
}

void QmitkAbstractDataStorageModel::SetNodePredicate(const mitk::NodePredicateBase* nodePredicate)
{
  if (m_NodePredicate == nodePredicate)
  {
    return;
  }

  m_NodePredicate = nodePredicate;
  // update model if the node predicate has been changed
  NodePredicateChanged();
}
