/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical Image Computing.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

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
  if (nullptr != m_DataStorage)
  {
    // remove listener from data storage
    m_DataStorage->AddNodeEvent.RemoveListener(
      mitk::MessageDelegate1<QmitkAbstractDataStorageModel, const mitk::DataNode*>(this, &QmitkAbstractDataStorageModel::NodeAdded));
    m_DataStorage->RemoveNodeEvent.RemoveListener(
      mitk::MessageDelegate1<QmitkAbstractDataStorageModel, const mitk::DataNode*>(this, &QmitkAbstractDataStorageModel::NodeRemoved));
    m_DataStorage->ChangedNodeEvent.RemoveListener(
      mitk::MessageDelegate1<QmitkAbstractDataStorageModel, const mitk::DataNode*>(this, &QmitkAbstractDataStorageModel::NodeChanged));
  }
}

void QmitkAbstractDataStorageModel::SetDataStorage(mitk::DataStorage* dataStorage)
{
  if (m_DataStorage == dataStorage)
  {
    return;
  }

  if (nullptr != m_DataStorage)
  {
    // remove listener from old data storage
    m_DataStorage->AddNodeEvent.RemoveListener(
      mitk::MessageDelegate1<QmitkAbstractDataStorageModel, const mitk::DataNode*>(this, &QmitkAbstractDataStorageModel::NodeAdded));
    m_DataStorage->RemoveNodeEvent.RemoveListener(
      mitk::MessageDelegate1<QmitkAbstractDataStorageModel, const mitk::DataNode*>(this, &QmitkAbstractDataStorageModel::NodeRemoved));
    m_DataStorage->ChangedNodeEvent.RemoveListener(
      mitk::MessageDelegate1<QmitkAbstractDataStorageModel, const mitk::DataNode*>(this, &QmitkAbstractDataStorageModel::NodeChanged));
  }

  m_DataStorage = dataStorage;

  if (nullptr != dataStorage)
  {
    // add listener for new data storage
    m_DataStorage->AddNodeEvent.AddListener(
      mitk::MessageDelegate1<QmitkAbstractDataStorageModel, const mitk::DataNode*>(this, &QmitkAbstractDataStorageModel::NodeAdded));
    m_DataStorage->RemoveNodeEvent.AddListener(
      mitk::MessageDelegate1<QmitkAbstractDataStorageModel, const mitk::DataNode*>(this, &QmitkAbstractDataStorageModel::NodeRemoved));
    m_DataStorage->ChangedNodeEvent.AddListener(
      mitk::MessageDelegate1<QmitkAbstractDataStorageModel, const mitk::DataNode*>(this, &QmitkAbstractDataStorageModel::NodeChanged));
  }
  // update model if the data storage has been changed
  DataStorageChanged();
}

void QmitkAbstractDataStorageModel::SetNodePredicate(mitk::NodePredicateBase* nodePredicate)
{
  if (m_NodePredicate == nodePredicate)
  {
    return;
  }

  m_NodePredicate = nodePredicate;
  // update model if the node predicate has been changed
  NodePredicateChanged();
}
