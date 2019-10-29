/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkImageStatisticsContainerManager.h"

#include "mitkNodePredicateAnd.h"
#include "mitkNodePredicateOr.h"
#include "mitkNodePredicateDataType.h"
#include "mitkNodePredicateNot.h"
#include "mitkStatisticsToImageRelationRule.h"
#include "mitkStatisticsToMaskRelationRule.h"

mitk::ImageStatisticsContainer::ConstPointer mitk::ImageStatisticsContainerManager::GetImageStatistics(const mitk::DataStorage* dataStorage, const mitk::BaseData* image, const mitk::BaseData* mask)
{
  if (!dataStorage) {
    mitkThrow() << "data storage is nullptr!";
  }
  if (!image) {
    mitkThrow() << "Image is nullptr";
  }

  mitk::NodePredicateBase::ConstPointer predicate = GetPredicateForSources(image, mask);;

  if (predicate) {
    auto nodePredicateImageStatisticsContainer = mitk::NodePredicateDataType::New(ImageStatisticsContainer::GetStaticNameOfClass());
    predicate = mitk::NodePredicateAnd::New(predicate, nodePredicateImageStatisticsContainer);

    auto statisticContainerCandidateNodes = dataStorage->GetSubset(predicate);
    mitk::DataStorage::SetOfObjects::Pointer statisticContainerCandidateNodesFiltered;

    statisticContainerCandidateNodesFiltered = mitk::DataStorage::SetOfObjects::New();
    for (const auto& node : *statisticContainerCandidateNodes) {
      statisticContainerCandidateNodesFiltered->push_back(node);
    }
    
    if (statisticContainerCandidateNodesFiltered->empty()) {
      return nullptr;
    }

    auto newestElement = statisticContainerCandidateNodesFiltered->front();
    if (statisticContainerCandidateNodesFiltered->size() > 1) {
        //in case of multiple found statistics, return only newest one
        auto newestIter = std::max_element(std::begin(*statisticContainerCandidateNodesFiltered), std::end(*statisticContainerCandidateNodesFiltered), [](mitk::DataNode::Pointer a, mitk::DataNode::Pointer b) {
          return a->GetData()->GetMTime() < b->GetData()->GetMTime();
        });
        newestElement = *newestIter;
        MITK_WARN << "multiple statistics (" << statisticContainerCandidateNodesFiltered->size() << ") for image/mask found. Returning only newest one.";
        for (const auto& node : *statisticContainerCandidateNodesFiltered) {
          MITK_DEBUG << node->GetName() << ", timestamp: " << node->GetData()->GetMTime();
        }
      }
      return dynamic_cast<mitk::ImageStatisticsContainer*>(newestElement->GetData());
  }
  else {
    return nullptr;
  }
}

mitk::NodePredicateBase::ConstPointer mitk::ImageStatisticsContainerManager::GetPredicateForSources(const mitk::BaseData* image, const mitk::BaseData* mask)
{
  if (!image) {
    mitkThrow() << "Image is nullptr";
  }

  auto imageRule = mitk::StatisticsToImageRelationRule::New();
  mitk::NodePredicateBase::ConstPointer predicate = imageRule->GetSourcesDetector(image);

  auto maskRule = mitk::StatisticsToMaskRelationRule::New();
  if (mask)
  {
    auto maskPredicate = maskRule->GetSourcesDetector(mask);
    predicate = mitk::NodePredicateAnd::New(predicate, maskPredicate);
  }
  else
  {
    auto maskPredicate = mitk::NodePredicateNot::New(maskRule->GetConnectedSourcesDetector());
    predicate = mitk::NodePredicateAnd::New(predicate, maskPredicate);
  }

  return predicate;
}
