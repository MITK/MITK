/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkImageStatisticsContainerManager.h"

#include "mitkNodePredicateAnd.h"
#include "mitkNodePredicateOr.h"
#include "mitkNodePredicateDataType.h"
#include "mitkNodePredicateNot.h"
#include "mitkNodePredicateFunction.h"
#include "mitkNodePredicateDataProperty.h"
#include "mitkProperties.h"

#include "mitkStatisticsToImageRelationRule.h"
#include "mitkStatisticsToMaskRelationRule.h"

mitk::ImageStatisticsContainer::Pointer mitk::ImageStatisticsContainerManager::GetImageStatistics(const mitk::DataStorage* dataStorage, const mitk::BaseData* image, const mitk::BaseData* mask, bool ignoreZeroVoxel, unsigned int histogramNBins, bool onlyIfUpToDate, bool noWIP)
{
  auto node = GetImageStatisticsNode(dataStorage, image, mask, ignoreZeroVoxel, histogramNBins, onlyIfUpToDate, noWIP);

  mitk::ImageStatisticsContainer::Pointer result;

  if (node.IsNotNull())
  {
    result = dynamic_cast<mitk::ImageStatisticsContainer*>(node->GetData());
  }

  return result;
}

mitk::DataNode::Pointer mitk::ImageStatisticsContainerManager::GetImageStatisticsNode(const mitk::DataStorage* dataStorage, const mitk::BaseData* image, const mitk::BaseData* mask, bool ignoreZeroVoxel, unsigned int histogramNBins, bool onlyIfUpToDate, bool noWIP)
{
  if (!dataStorage) {
    mitkThrow() << "data storage is nullptr!";
  }
  if (!image) {
    mitkThrow() << "Image is nullptr";
  }

  mitk::NodePredicateBase::ConstPointer predicate = GetStatisticsPredicateForSources(image, mask);

  mitk::DataNode::Pointer result;

  if (predicate)
  {
    auto binPredicate = mitk::NodePredicateDataProperty::New(mitk::STATS_HISTOGRAM_BIN_PROPERTY_NAME.c_str(), mitk::UIntProperty::New(histogramNBins));
    auto zeroPredicate = mitk::NodePredicateDataProperty::New(mitk::STATS_IGNORE_ZERO_VOXEL_PROPERTY_NAME.c_str(), mitk::BoolProperty::New(ignoreZeroVoxel));

    predicate = mitk::NodePredicateAnd::New(predicate, binPredicate, zeroPredicate).GetPointer();

    if (noWIP)
    {
      auto noWIPPredicate = mitk::NodePredicateNot::New(mitk::NodePredicateDataProperty::New(mitk::STATS_GENERATION_STATUS_PROPERTY_NAME.c_str()));
      predicate = mitk::NodePredicateAnd::New(predicate, noWIPPredicate).GetPointer();
    }

    auto statisticContainerCandidateNodes = dataStorage->GetSubset(predicate);

    auto statisticContainerCandidateNodesFiltered = mitk::DataStorage::SetOfObjects::New();

    for (const auto& node : *statisticContainerCandidateNodes)
    {
      auto isUpToDate = image->GetMTime() < node->GetData()->GetMTime()
        && (mask == nullptr || mask->GetMTime() < node->GetData()->GetMTime());

      if (!onlyIfUpToDate || isUpToDate)
      {
        statisticContainerCandidateNodesFiltered->push_back(node);
      }
    }

    if (!statisticContainerCandidateNodesFiltered->empty())
    {
      auto newestElement = statisticContainerCandidateNodesFiltered->front();
      if (statisticContainerCandidateNodesFiltered->size() > 1)
      {
        //in case of multiple found statistics, return only newest one
        auto newestIter = std::max_element(std::begin(*statisticContainerCandidateNodesFiltered), std::end(*statisticContainerCandidateNodesFiltered), [](mitk::DataNode::Pointer a, mitk::DataNode::Pointer b) {
          return a->GetData()->GetMTime() < b->GetData()->GetMTime();
        });
        newestElement = *newestIter;
        MITK_DEBUG << "multiple statistics (" << statisticContainerCandidateNodesFiltered->size() << ") for image/mask found. Returning only newest one.";
        for (const auto& node : *statisticContainerCandidateNodesFiltered)
        {
          MITK_DEBUG << node->GetName() << ", timestamp: " << node->GetData()->GetMTime();
        }
      }
      result = newestElement;
    }
  }

  return result;
}

mitk::NodePredicateBase::ConstPointer mitk::ImageStatisticsContainerManager::GetStatisticsPredicateForSources(const mitk::BaseData* image, const mitk::BaseData* mask)
{
  if (!image) {
    mitkThrow() << "Image is nullptr";
  }

  auto nodePredicateImageStatisticsContainer = mitk::NodePredicateDataType::New(ImageStatisticsContainer::GetStaticNameOfClass());

  auto imageRule = mitk::StatisticsToImageRelationRule::New();
  auto imagePredicate = imageRule->GetSourcesDetector(image);

  mitk::NodePredicateBase::ConstPointer predicate = mitk::NodePredicateAnd::New(nodePredicateImageStatisticsContainer, imagePredicate).GetPointer();

  auto maskRule = mitk::StatisticsToMaskRelationRule::New();
  if (mask)
  {
    auto maskPredicate = maskRule->GetSourcesDetector(mask);
    predicate = mitk::NodePredicateAnd::New(predicate, maskPredicate).GetPointer();
  }
  else
  {
    auto maskPredicate = mitk::NodePredicateNot::New(maskRule->GetConnectedSourcesDetector());
    predicate = mitk::NodePredicateAnd::New(predicate, maskPredicate).GetPointer();
  }

  return predicate;
}
