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

#include <QThreadPool>

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
      mitk::MessageDelegate1<QmitkDataGeneratorBase, const mitk::DataNode*>(this, &QmitkDataGeneratorBase::NodeAddedOrModified));

    // remove "remove node listener" from data storage
    dataStorage->ChangedNodeEvent.RemoveListener(
      mitk::MessageDelegate1<QmitkDataGeneratorBase, const mitk::DataNode*>(this, &QmitkDataGeneratorBase::NodeAddedOrModified));
  }
}

mitk::DataStorage::Pointer QmitkDataGeneratorBase::GetDataStorage() const
{
  return m_Storage.Lock();
}

bool QmitkDataGeneratorBase::GetAutoUpdate() const
{
  return m_AutoUpdate;
}

bool QmitkDataGeneratorBase::IsGenerating() const
{
  return m_RunningGeneration;
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
      mitk::MessageDelegate1<QmitkDataGeneratorBase, const mitk::DataNode*>(this, &QmitkDataGeneratorBase::NodeAddedOrModified));

    // remove "remove node listener" from old data storage
    oldStorage->ChangedNodeEvent.RemoveListener(
      mitk::MessageDelegate1<QmitkDataGeneratorBase, const mitk::DataNode*>(this, &QmitkDataGeneratorBase::NodeAddedOrModified));
  }

  m_Storage = storage;

  auto newStorage = m_Storage.Lock();

  if (newStorage.IsNotNull())
  {
    // add "add node listener" for new data storage
    newStorage->AddNodeEvent.AddListener(
      mitk::MessageDelegate1<QmitkDataGeneratorBase, const mitk::DataNode*>(this, &QmitkDataGeneratorBase::NodeAddedOrModified));

    // add remove node listener for new data storage
    newStorage->ChangedNodeEvent.AddListener(
      mitk::MessageDelegate1<QmitkDataGeneratorBase, const mitk::DataNode*>(this, &QmitkDataGeneratorBase::NodeAddedOrModified));
  }
}

void QmitkDataGeneratorBase::SetAutoUpdate(bool autoUpdate)
{
  m_AutoUpdate = autoUpdate;
}

void QmitkDataGeneratorBase::OnJobError(QString error, const QmitkDataGenerationJobBase* failedJob) const
{
  emit JobError(error, failedJob);
}

void QmitkDataGeneratorBase::OnFinalResultsAvailable(mitk::DataStorage::SetOfObjects::ConstPointer results, const QmitkDataGenerationJobBase *job) const
{
  if (results.IsNotNull())
  {
    {
      std::shared_lock<std::shared_mutex> mutexguard(m_DataMutex);
      auto storage = m_Storage.Lock();
      if (storage.IsNull())
      {
        for (auto pos = results->Begin(); pos != results->End(); ++pos)
        {
          storage->Add(pos->Value());
        }
      }
    }

    emit NewDataAvailable(results);
  }
}

void QmitkDataGeneratorBase::NodeAddedOrModified(const mitk::DataNode* node)
{
  if (this->NodeChangeIsRelevant(node))
  {
    m_RestartGeneration = true;
    if (!IsGenerating())
    {
      this->Generate();
    }
  }
}

void QmitkDataGeneratorBase::Generate() const
{
  m_RunningGeneration = true;
  m_RestartGeneration = true;
  while (m_RestartGeneration)
  {
    m_RestartGeneration = false;
    DoGenerate();
  }

  m_RunningGeneration = false;
}

void QmitkDataGeneratorBase::DoGenerate() const
{
  auto imageSegCombinations = this->GetAllImageStructCombinations();

  QThreadPool* threadPool = QThreadPool::globalInstance();

  bool everythingValid = true;

  for (const auto& imageAndSeg : imageSegCombinations)
  {
    MITK_INFO << "processing node " << imageAndSeg.first->GetName() << " and struct " << imageAndSeg.second->GetName();

    if (!this->IsValidResultAvailable(imageAndSeg.first.GetPointer(), imageAndSeg.second.GetPointer()))
    {
      this->IndicateFutureResults(imageAndSeg.first.GetPointer(), imageAndSeg.second.GetPointer());

      if (everythingValid)
      {
        emit GenerationStarted();
        everythingValid = false;
      }

      MITK_INFO << "no valid result available. Triggering computation of necessary jobs.";
      auto job = this->GetNextMissingGenerationJob(imageAndSeg.first.GetPointer(), imageAndSeg.second.GetPointer());

      //other jobs are pending, nothing has to be done
      if (!job) {
        MITK_INFO << "waiting for other jobs to finish";
      }
      else
      {
        job->setAutoDelete(true);
        connect(job, &QmitkDataGenerationJobBase::Error, this, &QmitkDataGeneratorBase::OnJobError);
        connect(job, &QmitkDataGenerationJobBase::ResultsAvailable, this, &QmitkDataGeneratorBase::OnFinalResultsAvailable);
        threadPool->start(job);
      }
    }
    else
    {
      this->RemoveObsoleteDataNodes(imageAndSeg.first.GetPointer(), imageAndSeg.second.GetPointer());
    }
  }

  if (everythingValid)
  {
    emit GenerationFinished();
  }
}
