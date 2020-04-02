/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkImageStatisticsDataGenerator.h"

#include "mitkImageStatisticsContainer.h"
#include "mitkStatisticsToImageRelationRule.h"
#include "mitkStatisticsToMaskRelationRule.h"
#include "mitkNodePredicateFunction.h"
#include "mitkNodePredicateAnd.h"
#include "mitkNodePredicateNot.h"
#include "mitkNodePredicateDataProperty.h"
#include "mitkProperties.h"
#include "mitkImageStatisticsContainerManager.h"

#include "QmitkImageStatisticsCalculationRunnable.h"

void QmitkImageStatisticsDataGenerator::SetIgnoreZeroValueVoxel(bool _arg)
{
  if (m_IgnoreZeroValueVoxel != _arg)
  {
    m_IgnoreZeroValueVoxel = _arg;

    if (m_AutoUpdate)
    {
      this->EnsureRecheckingAndGeneration();
    }
  }
}

bool QmitkImageStatisticsDataGenerator::GetIgnoreZeroValueVoxel() const
{
  return this->m_IgnoreZeroValueVoxel;
}

void QmitkImageStatisticsDataGenerator::SetHistogramNBins(unsigned int nbins)
{
  if (m_HistogramNBins != nbins)
  {
    m_HistogramNBins = nbins;

    if (m_AutoUpdate)
    {
      this->EnsureRecheckingAndGeneration();
    }
  }
}

unsigned int QmitkImageStatisticsDataGenerator::GetHistogramNBins() const
{
  return this->m_HistogramNBins;
}

bool QmitkImageStatisticsDataGenerator::ChangedNodeIsRelevant(const mitk::DataNode* changedNode) const
{
  auto result = QmitkImageAndRoiDataGeneratorBase::ChangedNodeIsRelevant(changedNode);

  if (!result)
  {
    if (changedNode->GetProperty(mitk::STATS_GENERATION_STATUS_PROPERTY_NAME.c_str()) == nullptr)
    {
      auto stats = dynamic_cast<const mitk::ImageStatisticsContainer*>(changedNode->GetData());
      result = stats != nullptr;
    }
  }
  return result;
}

bool QmitkImageStatisticsDataGenerator::IsValidResultAvailable(const mitk::DataNode* imageNode, const mitk::DataNode* roiNode) const
{
  auto resultNode = this->GetLatestResult(imageNode, roiNode, true, true);
  return resultNode.IsNotNull();
}

mitk::DataNode::Pointer QmitkImageStatisticsDataGenerator::GetLatestResult(const mitk::DataNode* imageNode, const mitk::DataNode* roiNode, bool onlyIfUpToDate, bool noWIP) const
{
  auto storage = m_Storage.Lock();

  if (imageNode == nullptr || !imageNode->GetData())
  {
    mitkThrow() << "Image is nullptr";
  }

  const auto image = imageNode->GetData();
  const mitk::BaseData* mask = nullptr;
  if (roiNode)
  {
    mask = roiNode->GetData();
  }
    
  std::lock_guard<std::mutex> mutexguard(m_DataMutex);
  return mitk::ImageStatisticsContainerManager::GetImageStatisticsNode(storage, image, mask, m_IgnoreZeroValueVoxel, m_HistogramNBins, onlyIfUpToDate, noWIP);
}

void QmitkImageStatisticsDataGenerator::IndicateFutureResults(const mitk::DataNode* imageNode, const mitk::DataNode* roiNode) const
{
  if (imageNode == nullptr || !imageNode->GetData())
  {
    mitkThrow() << "Image node is nullptr";
  }

  auto image = dynamic_cast<const mitk::Image*>(imageNode->GetData());
  if (!image)
  {
    mitkThrow() << "Image node date is nullptr or no image.";
  }

  const mitk::BaseData* mask = nullptr;
  if (roiNode != nullptr)
  {
    mask = roiNode->GetData();
  }

  auto resultDataNode = this->GetLatestResult(imageNode, roiNode, true, false);
  if (resultDataNode.IsNull())
  {
    auto dummyStats = mitk::ImageStatisticsContainer::New();

    auto imageRule = mitk::StatisticsToImageRelationRule::New();
    imageRule->Connect(dummyStats, image);

    if (nullptr != mask)
    {
      auto maskRule = mitk::StatisticsToMaskRelationRule::New();
      maskRule->Connect(dummyStats, mask);
    }

    dummyStats->SetProperty(mitk::STATS_HISTOGRAM_BIN_PROPERTY_NAME.c_str(), mitk::UIntProperty::New(m_HistogramNBins));
    dummyStats->SetProperty(mitk::STATS_IGNORE_ZERO_VOXEL_PROPERTY_NAME.c_str(), mitk::BoolProperty::New(m_IgnoreZeroValueVoxel));

    auto dummyNode = CreateWIPDataNode(dummyStats, "WIP_"+GenerateStatisticsNodeName(image, mask));

    auto storage = m_Storage.Lock();
    if (storage != nullptr)
    {
      std::lock_guard<std::mutex> mutexguard(m_DataMutex);
      storage->Add(dummyNode);
    }
  }
}

std::pair<QmitkDataGenerationJobBase*, mitk::DataNode::Pointer> QmitkImageStatisticsDataGenerator::GetNextMissingGenerationJob(const mitk::DataNode* imageNode, const mitk::DataNode* roiNode) const
{
  auto resultDataNode = this->GetLatestResult(imageNode, roiNode, true, false);

  std::string status;
  if (resultDataNode.IsNull() || (resultDataNode->GetStringProperty(mitk::STATS_GENERATION_STATUS_PROPERTY_NAME.c_str(), status) && status == mitk::STATS_GENERATION_STATUS_VALUE_PENDING))
  {
    if (imageNode == nullptr || !imageNode->GetData())
    {
      mitkThrow() << "Image node is nullptr";
    }

    auto image = dynamic_cast<const mitk::Image*>(imageNode->GetData());
    if (image == nullptr)
    {
      mitkThrow() << "Image node date is nullptr or no image.";
    }

    const mitk::Image* mask = nullptr;
    const mitk::PlanarFigure* planar = nullptr;
    if (roiNode != nullptr)
    {
      mask = dynamic_cast<const mitk::Image*>(roiNode->GetData());
      planar = dynamic_cast<const mitk::PlanarFigure*>(roiNode->GetData());
    }

    auto newJob = new QmitkImageStatisticsCalculationRunnable;

    newJob->Initialize(image, mask, planar);
    newJob->SetIgnoreZeroValueVoxel(m_IgnoreZeroValueVoxel);
    newJob->SetHistogramNBins(m_HistogramNBins);

    return std::pair<QmitkDataGenerationJobBase*, mitk::DataNode::Pointer>(newJob, resultDataNode.GetPointer());
  }
  else if (resultDataNode->GetStringProperty(mitk::STATS_GENERATION_STATUS_PROPERTY_NAME.c_str(), status) && status == mitk::STATS_GENERATION_STATUS_VALUE_WORK_IN_PROGRESS)
  {
    return std::pair<QmitkDataGenerationJobBase*, mitk::DataNode::Pointer>(nullptr, resultDataNode.GetPointer());
  }
  return std::pair<QmitkDataGenerationJobBase*, mitk::DataNode::Pointer>(nullptr, nullptr);
}

void QmitkImageStatisticsDataGenerator::RemoveObsoleteDataNodes(const mitk::DataNode* imageNode, const mitk::DataNode* roiNode) const
{
  if (imageNode == nullptr || !imageNode->GetData())
  {
    mitkThrow() << "Image is nullptr";
  }

  const auto image = imageNode->GetData();
  const mitk::BaseData* mask = nullptr;
  if (roiNode != nullptr)
  {
    mask = roiNode->GetData();
  }

  auto lastResult = this->GetLatestResult(imageNode, roiNode, false, false);

  auto rulePredicate = mitk::ImageStatisticsContainerManager::GetStatisticsPredicateForSources(image, mask);
  auto notLatestPredicate = mitk::NodePredicateFunction::New([lastResult](const mitk::DataNode* node) { return node != lastResult; });
  auto binPredicate = mitk::NodePredicateDataProperty::New(mitk::STATS_HISTOGRAM_BIN_PROPERTY_NAME.c_str(), mitk::UIntProperty::New(m_HistogramNBins));
  auto zeroPredicate = mitk::NodePredicateDataProperty::New(mitk::STATS_IGNORE_ZERO_VOXEL_PROPERTY_NAME.c_str(), mitk::BoolProperty::New(m_IgnoreZeroValueVoxel));

  mitk::NodePredicateBase::ConstPointer predicate = mitk::NodePredicateAnd::New(rulePredicate, notLatestPredicate).GetPointer();
  predicate = mitk::NodePredicateAnd::New(predicate, binPredicate, zeroPredicate).GetPointer();

  auto storage = m_Storage.Lock();
  if (storage != nullptr)
  {
    std::lock_guard<std::mutex> mutexguard(m_DataMutex);

    auto oldStatisticContainerNodes = storage->GetSubset(predicate);
    storage->Remove(oldStatisticContainerNodes);
  }
}

mitk::DataNode::Pointer QmitkImageStatisticsDataGenerator::PrepareResultForStorage(const std::string& /*label*/, mitk::BaseData* result, const QmitkDataGenerationJobBase* job) const
{
  auto statsJob = dynamic_cast<const QmitkImageStatisticsCalculationRunnable*>(job);

  if (statsJob != nullptr)
  {
    auto resultNode = mitk::DataNode::New();
    resultNode->SetProperty("helper object", mitk::BoolProperty::New(true));
    resultNode->SetVisibility(false);
    resultNode->SetData(result);
    
    const mitk::BaseData* roi = statsJob->GetMaskImage();
    if (roi == nullptr)
    {
      roi = statsJob->GetPlanarFigure();
    }
    resultNode->SetName(this->GenerateStatisticsNodeName(statsJob->GetStatisticsImage(), roi));

    return resultNode;
  }

  return nullptr;
}

std::string QmitkImageStatisticsDataGenerator::GenerateStatisticsNodeName(const mitk::Image* image, const mitk::BaseData* roi) const
{
  std::stringstream statisticsNodeName;
  statisticsNodeName << "statistics_bins-" << m_HistogramNBins <<"_";

  if (m_IgnoreZeroValueVoxel)
  {
    statisticsNodeName << "noZeros_";
  }

  if (image == nullptr)
  {
    mitkThrow() << "Image is nullptr";
  }

  statisticsNodeName << image->GetUID();

  if (roi != nullptr)
  {
    statisticsNodeName << "_" + roi->GetUID();
  }

  return statisticsNodeName.str();
}
