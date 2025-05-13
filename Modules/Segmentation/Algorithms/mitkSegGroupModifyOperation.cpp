/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSegGroupModifyOperation.h"
#include <mitkImageTimeSelector.h>
#include <mitkImage.h>

mitk::SegGroupModifyOperation::SegGroupModifyOperation(MultiLabelSegmentation* segmentation,
  const ModifyGroupImageMapType& modifiedGroupImages,
  const ModifyLabelsMapType& modifiedLabels,
  const ModifyGroupNameMapType& modifiedNames)
  : SegChangeOperationBase(segmentation, 1), m_ModifiedLabels(modifiedLabels), m_ModifiedNames(modifiedNames)
{
  for (auto& [groupID, tsImageMap] : modifiedGroupImages)
  {
    for (auto& [timeStep, image] : tsImageMap)
    {
      auto compressor = std::make_unique<CompressedImageContainer>();
      compressor->CompressImage(image);
      m_ModifiedImages[groupID].emplace(timeStep, std::move(compressor));
    }
  }
}

mitk::SegGroupModifyOperation::GroupIndexVectorType mitk::SegGroupModifyOperation::GetImageGroupIDs() const
{
  GroupIndexVectorType result;
  for (auto& pair : m_ModifiedImages)
  {
    result.push_back(pair.first);
  }
  return result;
}


mitk::SegGroupModifyOperation::TimeStepVectorType mitk::SegGroupModifyOperation::GetImageTimeSteps(MultiLabelSegmentation::GroupIndexType groupID) const
{
  TimeStepVectorType result;
  for (auto& pair : m_ModifiedImages.at(groupID))
  {
    result.push_back(pair.first);
  }
  return result;
}

mitk::SegGroupModifyOperation::GroupIndexVectorType mitk::SegGroupModifyOperation::GetLabelGroupIDs() const
{
  GroupIndexVectorType result;
  for (auto& pair : m_ModifiedLabels)
  {
    result.push_back(pair.first);
  }
  return result;
}

mitk::SegGroupModifyOperation::GroupIndexVectorType mitk::SegGroupModifyOperation::GetNameGroupIDs() const
{
  GroupIndexVectorType result;
  for (auto& pair : m_ModifiedNames)
  {
    result.push_back(pair.first);
  }
  return result;
}

mitk::Image::Pointer mitk::SegGroupModifyOperation::GetModifiedGroupImage(MultiLabelSegmentation::GroupIndexType groupID, TimeStepType timeStep) const
{
  auto image = m_ModifiedImages.at(groupID).at(timeStep)->DecompressImage();
  return image;
}

mitk::MultiLabelSegmentation::ConstLabelVectorType mitk::SegGroupModifyOperation::GetModifiedLabels(MultiLabelSegmentation::GroupIndexType groupID) const
{
  return m_ModifiedLabels.at(groupID);
}

std::string mitk::SegGroupModifyOperation::GetModifiedName(MultiLabelSegmentation::GroupIndexType groupID) const
{
  return m_ModifiedNames.at(groupID);
}

mitk::SegGroupModifyOperation* mitk::SegGroupModifyOperation::CreatFromSegmentation(
  MultiLabelSegmentation* segmentation,
  const std::set<MultiLabelSegmentation::GroupIndexType>& relevantGroupIDs,
  bool coverAllTimeSteps, TimeStepType relevantTimeStep,
  bool noLabels, bool noGroupImages, bool noNames)
{
  if (noLabels && noGroupImages)
    mitkThrow() << "Invalid call of mitk::SegGroupModifyOperation::CreatFromSegmentation. Arguments noLabels and noGroupImages must not be both true at the same time.";

  SegGroupModifyOperation::ModifyLabelsMapType labelData;
  SegGroupModifyOperation::ModifyGroupImageMapType imageData;
  SegGroupModifyOperation::ModifyGroupNameMapType names;

  std::vector<TimeStepType> relevantTSs({ relevantTimeStep });
  if (coverAllTimeSteps)
  {
    relevantTSs.resize(segmentation->GetTimeSteps());
    std::iota(relevantTSs.begin(), relevantTSs.end(), 0);
  }

  for (auto groupID : relevantGroupIDs)
  {
    if (!noLabels)
    {
      //clone relevant labels
      auto labels = segmentation->GetConstLabelsByValue(segmentation->GetLabelValuesByGroup(groupID));
      MultiLabelSegmentation::ConstLabelVectorType clonedLabels;
      clonedLabels.reserve(labels.size());

      std::transform(labels.begin(), labels.end(),
        std::back_inserter(clonedLabels),
        [](const Label* label)
        {
          return label->Clone();
        });

      labelData.emplace(groupID, std::move(clonedLabels));
    }

    if (!noGroupImages)
    {
      //clone all relevant images
      for (auto aTimeStep : relevantTSs)
      {
        imageData[groupID][aTimeStep] = SelectImageByTimeStep(segmentation->GetGroupImage(groupID), aTimeStep);
      }
    }

    if (!noNames)
    {
      names.emplace(groupID, segmentation->GetGroupName(groupID));
    }
  }

  return new SegGroupModifyOperation(segmentation, imageData, labelData, names);
}

