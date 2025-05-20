/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSegGroupInsertOperation.h"
#include <mitkImage.h>
#include <mitkUndoController.h>

mitk::SegGroupInsertOperation::SegGroupInsertOperation(MultiLabelSegmentation* segmentation,
  const GroupIndexSetType& groupIDs,
  const ModifyGroupImageMapType& groupImages,
  const ModifyLabelsMapType& groupLabels)
  : SegChangeOperationBase(segmentation, 1), m_GroupIDs(groupIDs), m_Labels(groupLabels)
{
  for (auto& [groupID, image] : groupImages)
  {
    auto compressor = std::make_unique<CompressedImageContainer>();
    compressor->CompressImage(image);
    m_Images.emplace(groupID, std::move(compressor));
  }
  for (auto& groupID : groupIDs)
  {
    m_Names.emplace(groupID, segmentation->GetGroupName(groupID));
  }
}

mitk::SegGroupInsertOperation::GroupIndexSetType mitk::SegGroupInsertOperation::GetGroupIDs() const
{
  return m_GroupIDs;
}

mitk::SegGroupInsertOperation::GroupIndexSetType mitk::SegGroupInsertOperation::GetImageGroupIDs() const
{
  GroupIndexSetType result;
  for (auto& pair : m_Images)
  {
    result.insert(pair.first);
  }
  return result;
}

mitk::SegGroupInsertOperation::GroupIndexSetType mitk::SegGroupInsertOperation::GetLabelGroupIDs() const
{
  GroupIndexSetType result;
  for (auto& pair : m_Labels)
  {
    result.insert(pair.first);
  }
  return result;
}

mitk::Image::Pointer mitk::SegGroupInsertOperation::GetGroupImage(MultiLabelSegmentation::GroupIndexType groupID) const
{
  if (m_Images.cend() != m_Images.find(groupID))
  {
    auto image = m_Images.at(groupID)->DecompressImage();
    return image;
  }
  
  return nullptr;
}

mitk::MultiLabelSegmentation::ConstLabelVectorType mitk::SegGroupInsertOperation::GetGroupLabels(MultiLabelSegmentation::GroupIndexType groupID) const
{
  if (m_Labels.cend() != m_Labels.find(groupID))
  {
    return m_Labels.at(groupID);
  }

  return {};
}

std::string mitk::SegGroupInsertOperation::GetGroupName(MultiLabelSegmentation::GroupIndexType groupID) const
{
  if (m_Names.cend() != m_Names.find(groupID))
  {
    return m_Names.at(groupID);
  }

  return "";
}

mitk::SegGroupInsertOperation* mitk::SegGroupInsertOperation::CreatFromSegmentation(
  MultiLabelSegmentation* segmentation,
  const GroupIndexSetType& relevantGroupIDs,
  bool noLabels, bool noGroupImages)
{
  if (nullptr == segmentation)
    mitkThrow() << "Invalid call of CreatFromSegmentation. Segmentation is not valid (nullptr).";

  SegGroupInsertOperation::ModifyLabelsMapType labelData;
  SegGroupInsertOperation::ModifyGroupImageMapType imageData;

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
      imageData[groupID] = segmentation->GetGroupImage(groupID);
    }
  }

  return new SegGroupInsertOperation(segmentation, relevantGroupIDs,imageData, labelData);
}