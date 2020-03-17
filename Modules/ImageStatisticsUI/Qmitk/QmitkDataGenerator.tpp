/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef __QMITK_DATA_GENERATOR_TPP
#define __QMITK_DATA_GENERATOR_TPP

#include <QThreadPool>
#include "mitkDataNode.h"
#include "mitkRTPredicates.h"
#include "QmitkDataGenerationJobBase.h"
#include "mitkPropertyConstants.h"

#include "QmitkDataGenerator.h"

mitk::DataStorage::SetOfObjects::ConstPointer QmitkDataGeneratorBase::GetStructureNodes() const
{
  return m_StructureNodes;
}

mitk::DataStorage::SetOfObjects::ConstPointer QmitkDataGeneratorBase::GetDoseNodes() const
{
  return m_DoseNodes;
}

void QmitkDataGeneratorBase::SetDoseNodes(mitk::DataStorage::SetOfObjects::ConstPointer doseNodes)
{
  m_DoseNodes = doseNodes;
}

void QmitkDataGeneratorBase::SetStructureNodes(mitk::DataStorage::SetOfObjects::ConstPointer structureNodes)
{
  m_StructureNodes = structureNodes;
}

std::vector<std::pair<mitk::DataNode::Pointer, mitk::DataNode::Pointer> >
QmitkDataGeneratorBase::GetAllDoseStructCombinations() const {
  std::vector<std::pair<mitk::DataNode::Pointer, mitk::DataNode::Pointer> > allCombinations;
  for (auto& doseNode : *m_DoseNodes) {
    for (auto& structNode : *m_StructureNodes) {
      allCombinations.push_back(std::make_pair(doseNode, structNode));
    }
  }
  return allCombinations;
}


template <class taskGenerationRule>
void
QmitkRTDataGenerator<taskGenerationRule>::DoGenerate() {
  auto doseAndStructCombinations = GetAllDoseStructCombinations();

  QThreadPool* threadPool = QThreadPool::globalInstance();
  bool jobSpawned = false;
  for (const auto& doseAndStruct : doseAndStructCombinations) {
    MITK_INFO << "processing node " << doseAndStruct.first->GetName() << " and struct " << doseAndStruct.second->GetName();
    taskGenerationRule aTaskGenerationRule = taskGenerationRule();
    if (!aTaskGenerationRule.IsResultAvailable(m_Storage.GetPointer(), doseAndStruct.first.GetPointer(), doseAndStruct.second.GetPointer())) {
      MITK_INFO << "no result available. Triggering computation of necessary jobs.";
      auto job = aTaskGenerationRule.GetNextMissingJob(m_Storage, doseAndStruct.first.GetPointer(), doseAndStruct.second.GetPointer());
      //other jobs are pending, nothing has to be done
      if (!job) {
        MITK_INFO << "waiting for other jobs to finish";
        return;
      }
      job->setAutoDelete(true);
      for (auto& dataNode : *(job->GetDataNodes())) {
        //only add interim nodes, their data get updated as soon as it's computed
        if (dataNode->GetData()->GetProperty(mitk::RT_CACHE_STATUS_NAME.c_str())) {
          m_Storage->Add(dataNode);
        }
      }
      connect(job, SIGNAL(Error(QString, const QmitkRTJobBase*)), this, SLOT(OnJobError(QString, const QmitkRTJobBase*)));
      connect(job, SIGNAL(ResultsAvailable(const mitk::DataStorage::SetOfObjects*, const QmitkRTJobBase*)), this, SLOT(OnFinalResultsAvailable(const mitk::DataStorage::SetOfObjects*, const QmitkRTJobBase*)));
      threadPool->start(job);
      jobSpawned = true;
    }
  }
  if (!jobSpawned) {
    emit AllJobsGenerated();
  }
}


#endif
