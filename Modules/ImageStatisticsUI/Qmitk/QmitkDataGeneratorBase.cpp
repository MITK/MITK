/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "QmitkDataGeneratorBase.h"

#include "QmitkDataGenerationJobBase.h"
#include "mitkDataNode.h"

QmitkDataGeneratorBase::QmitkDataGeneratorBase(mitk::DataStorage::Pointer storage) :
  m_Storage(storage)
{}

QmitkDataGeneratorBase::QmitkDataGeneratorBase()
{}

QmitkDataGeneratorBase::~QmitkDataGeneratorBase()
{
  auto dataStorage = m_Storage.Lock();
  if (dataStorage.IsNotNull())
  {
    // remove "add node listener" from data storage
    dataStorage->AddNodeEvent.RemoveListener(
      mitk::MessageDelegate1<QmitkDataGeneratorBase, const mitk::DataNode*>(this, &QmitkDataGeneratorBase::NodeAddedToStorage));

    // remove "remove node listener" from data storage
    dataStorage->ChangedNodeEvent.RemoveListener(
      mitk::MessageDelegate1<QmitkDataGeneratorBase, const mitk::DataNode*>(this, &QmitkDataGeneratorBase::NodeModified));
  }
}

mitk::DataStorage* QmitkDataGeneratorBase::GetDataStorage() const
{
  return m_Storage;
}

bool QmitkDataGeneratorBase::GetAutoUpdate() const
{
  return m_AutoUpdate;
}

void QmitkDataGeneratorBase::SetDataStorage(mitk::DataStorage* storage)
{
  if (storage == m_Storage) return;

  std::shared_lock<std::shared_mutex> mutexguard(m_DataMutex);

  auto oldStorage = m_Storage.Lock();
  if (oldStorage.IsNotNull())
  {
    // remove "add node listener" from old data storage
    oldStorage->AddNodeEvent.RemoveListener(
      mitk::MessageDelegate1<QmitkDataGeneratorBase, const mitk::DataNode*>(this, &QmitkDataGeneratorBase::NodeAddedToStorage));

    // remove "remove node listener" from old data storage
    oldStorage->ChangedNodeEvent.RemoveListener(
      mitk::MessageDelegate1<QmitkDataGeneratorBase, const mitk::DataNode*>(this, &QmitkDataGeneratorBase::NodeModified));
  }

  m_Storage = storage;

  auto newStorage = m_Storage.Lock();

  if (newStorage.IsNotNull())
  {
    // add "add node listener" for new data storage
    newStorage->AddNodeEvent.AddListener(
      mitk::MessageDelegate1<QmitkDataGeneratorBase, const mitk::DataNode*>(this, &QmitkDataGeneratorBase::NodeAddedToStorage));

    // add remove node listener for new data storage
    newStorage->ChangedNodeEvent.AddListener(
      mitk::MessageDelegate1<QmitkDataGeneratorBase, const mitk::DataNode*>(this, &QmitkDataGeneratorBase::NodeModified));
  }
}

void QmitkDataGeneratorBase::SetAutoUpdate(bool autoUpdate)
{
  m_AutoUpdate = autoUpdate;
}

void QmitkDataGeneratorBase::OnJobError(QString error, const QmitkRTJobBase* failedJob)
{
  emit JobError(error, failedJob);
}

void QmitkDataGeneratorBase::OnFinalResultsAvailable(const mitk::DataStorage::SetOfObjects* results, const QmitkRTJobBase *job)
{
  if (results)
  {
    TODO go through the results and ensure that the cache status property is removed
    //no need to remove the WIP property (mitk::RT_CACHE_STATUS_NAME) again, as directly set the BaseData in the job (where this property is not set)
    emit NewDataAvailable(results);
  }
}

TODO if storage is set the generator should register observers for node changes of datastorage. See e.g. node selection widget how to do it.
If some node has changed that is relevant it flags new generation if a doGeneration is ongoing or triggers a new one.

void QmitkDataGeneratorBase::Generate()
{
  m_RunningGeneration = true;
  while (m_RestartGeneration)
  {
    m_RestartGeneration = false;
    DoGeneration();
  }

  m_RunningGeneration = false;
}