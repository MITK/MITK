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
#include "mitkProperties.h"
#include "mitkImageStatisticsContainerManager.h"

#include <QThreadPool>

QmitkDataGeneratorBase::QmitkDataGeneratorBase(mitk::DataStorage::Pointer storage, QObject* parent) : QObject(parent)
{
  this->SetDataStorage(storage);
}

QmitkDataGeneratorBase::QmitkDataGeneratorBase(QObject* parent): QObject(parent)
{}

QmitkDataGeneratorBase::~QmitkDataGeneratorBase()
{
  auto dataStorage = m_Storage.Lock();
  if (dataStorage.IsNotNull())
  {
    // remove "change node listener" from data storage
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
  return m_WIP;
}

void QmitkDataGeneratorBase::SetDataStorage(mitk::DataStorage* storage)
{
  if (storage == m_Storage) return;

  std::lock_guard<std::mutex> mutexguard(m_DataMutex);

  auto oldStorage = m_Storage.Lock();
  if (oldStorage.IsNotNull())
  {
    // remove "change node listener" from old data storage
    oldStorage->ChangedNodeEvent.RemoveListener(
      mitk::MessageDelegate1<QmitkDataGeneratorBase, const mitk::DataNode*>(this, &QmitkDataGeneratorBase::NodeAddedOrModified));
  }

  m_Storage = storage;

  auto newStorage = m_Storage.Lock();

  if (newStorage.IsNotNull())
  {
    // add change node listener for new data storage
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

void QmitkDataGeneratorBase::OnFinalResultsAvailable(JobResultMapType results, const QmitkDataGenerationJobBase *job) const
{
  auto resultnodes = mitk::DataStorage::SetOfObjects::New();

  for (const auto &pos : results)
  {
    resultnodes->push_back(this->PrepareResultForStorage(pos.first, pos.second, job));
  }

  {
    std::lock_guard<std::mutex> mutexguard(m_DataMutex);
    auto storage = m_Storage.Lock();
    if (storage.IsNotNull())
    {
      m_AddingToStorage = true;
      for (auto pos = resultnodes->Begin(); pos != resultnodes->End(); ++pos)
      {
        storage->Add(pos->Value());
      }
      m_AddingToStorage = false;
    }
  }

  emit NewDataAvailable(resultnodes.GetPointer());

  if (!resultnodes->empty())
  {
    this->EnsureRecheckingAndGeneration();
  }
}

void QmitkDataGeneratorBase::NodeAddedOrModified(const mitk::DataNode* node)
{
  if (!m_AddingToStorage)
  {
    if (this->ChangedNodeIsRelevant(node))
    {
      this->EnsureRecheckingAndGeneration();
    }
  }
}

void QmitkDataGeneratorBase::EnsureRecheckingAndGeneration() const
{
  m_RestartGeneration = true;
  if (!m_InGenerate)
  {
    this->Generate();
  }
}

bool QmitkDataGeneratorBase::Generate() const
{
  bool everythingValid = false;
  if (m_InGenerate)
  {
    m_RestartGeneration = true;
  }
  else
  {
    m_InGenerate = true;
    m_RestartGeneration = true;
    while (m_RestartGeneration)
    {
      m_RestartGeneration = false;
      everythingValid = DoGenerate();
    }

    m_InGenerate = false;
  }
  return everythingValid;
}

mitk::DataNode::Pointer QmitkDataGeneratorBase::CreateWIPDataNode(mitk::BaseData* dataDummy, const std::string& nodeName)
{
  if (!dataDummy) {
    mitkThrow() << "data is nullptr";
  }

  auto interimResultNode = mitk::DataNode::New();
  interimResultNode->SetProperty("helper object", mitk::BoolProperty::New(true));
  dataDummy->SetProperty(mitk::STATS_GENERATION_STATUS_PROPERTY_NAME.c_str(), mitk::StringProperty::New(mitk::STATS_GENERATION_STATUS_VALUE_PENDING));
  interimResultNode->SetVisibility(false);
  interimResultNode->SetData(dataDummy);
  if (!nodeName.empty())
  {
    interimResultNode->SetName(nodeName);
  }
  return interimResultNode;
}


QmitkDataGeneratorBase::InputPairVectorType QmitkDataGeneratorBase::FilterImageROICombinations(InputPairVectorType&& imageROICombinations) const
{
  std::lock_guard<std::mutex> mutexguard(m_DataMutex);

  InputPairVectorType filteredImageROICombinations;

  auto storage = m_Storage.Lock();
  if (storage.IsNotNull())
  {
    for (auto inputPair : imageROICombinations)
    {
      if (storage->Exists(inputPair.first) && (inputPair.second.IsNull() || storage->Exists(inputPair.second)))
      {
        filteredImageROICombinations.emplace_back(inputPair);
      }
      else
      {
        MITK_DEBUG << "Ignore pair because at least one of the nodes is not in storage. Pair: " << GetPairDescription(inputPair);
      }
    }
  }
  return filteredImageROICombinations;
}

std::string QmitkDataGeneratorBase::GetPairDescription(const InputPairVectorType::value_type& imageAndSeg) const
{
  if (imageAndSeg.second.IsNotNull())
  {
    return imageAndSeg.first->GetName() + " and ROI " + imageAndSeg.second->GetName();
  }
  else
  {
    return imageAndSeg.first->GetName();
  }
}

bool QmitkDataGeneratorBase::DoGenerate() const
{
  auto filteredImageROICombinations = FilterImageROICombinations(this->GetAllImageROICombinations());

  QThreadPool* threadPool = QThreadPool::globalInstance();

  bool everythingValid = true;

  for (const auto& imageAndSeg : filteredImageROICombinations)
  {
    MITK_DEBUG << "checking node " << GetPairDescription(imageAndSeg);

    if (!this->IsValidResultAvailable(imageAndSeg.first.GetPointer(), imageAndSeg.second.GetPointer()))
    {
      this->IndicateFutureResults(imageAndSeg.first.GetPointer(), imageAndSeg.second.GetPointer());

      if (everythingValid)
      {
        m_WIP = true;
        everythingValid = false;
      }

      MITK_DEBUG << "No valid result available. Requesting next necessary job." << imageAndSeg.first->GetName();
      auto nextJob = this->GetNextMissingGenerationJob(imageAndSeg.first.GetPointer(), imageAndSeg.second.GetPointer());

      //other jobs are pending, nothing has to be done
      if (nextJob.first==nullptr && nextJob.second.IsNotNull())
      {
        MITK_DEBUG << "Last generation job still running, pass on till job is finished...";
      }
      else if(nextJob.first != nullptr && nextJob.second.IsNotNull())
      {
        MITK_DEBUG << "Next generation job started...";
        nextJob.first->setAutoDelete(true);
        nextJob.second->GetData()->SetProperty(mitk::STATS_GENERATION_STATUS_PROPERTY_NAME.c_str(), mitk::StringProperty::New(mitk::STATS_GENERATION_STATUS_VALUE_WORK_IN_PROGRESS));
        connect(nextJob.first, &QmitkDataGenerationJobBase::Error, this, &QmitkDataGeneratorBase::OnJobError, Qt::BlockingQueuedConnection);
        connect(nextJob.first, &QmitkDataGenerationJobBase::ResultsAvailable, this, &QmitkDataGeneratorBase::OnFinalResultsAvailable, Qt::BlockingQueuedConnection);
        emit DataGenerationStarted(imageAndSeg.first.GetPointer(), imageAndSeg.second.GetPointer(), nextJob.first);
        threadPool->start(nextJob.first);
      }
    }
    else
    {
      this->RemoveObsoleteDataNodes(imageAndSeg.first.GetPointer(), imageAndSeg.second.GetPointer());
    }
  }

  if (everythingValid && m_WIP)
  {
    m_WIP = false;
    emit GenerationFinished();
  }

  return everythingValid;
}
