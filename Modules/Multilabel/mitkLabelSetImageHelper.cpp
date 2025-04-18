/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkLabelSetImageHelper.h>

#include <mitkDataStorage.h>
#include <mitkLabelSetImage.h>
#include <mitkExceptionMacro.h>
#include <mitkProperties.h>

#include <array>
#include <regex>
#include <vector>

namespace
{
  template <typename T>
  std::array<int, 3> QuantizeColor(const T* color)
  {
    return {
      static_cast<int>(std::round(color[0] * 255)),
      static_cast<int>(std::round(color[1] * 255)),
      static_cast<int>(std::round(color[2] * 255)) };
  }

  mitk::Color FromLookupTableColor(const double* lookupTableColor)
  {
    mitk::Color color;
    color.Set(
      static_cast<float>(lookupTableColor[0]),
      static_cast<float>(lookupTableColor[1]),
      static_cast<float>(lookupTableColor[2]));
    return color;
  }
}

mitk::DataNode::Pointer mitk::LabelSetImageHelper::CreateEmptySegmentationNode(const std::string& segmentationName)
{
  auto newSegmentationNode = mitk::DataNode::New();
  newSegmentationNode->SetName(segmentationName);

  // initialize "showVolume"-property to false to prevent recalculating the volume while working on the segmentation
  newSegmentationNode->SetProperty("showVolume", mitk::BoolProperty::New(false));

  return newSegmentationNode;
}


mitk::DataNode::Pointer mitk::LabelSetImageHelper::CreateNewSegmentationNode(const DataNode* referenceNode,
  const Image* initialSegmentationImage, const std::string& segmentationName, const DataStorage* dataStorage)
{
  std::string newSegmentationName = segmentationName;
  if (newSegmentationName.empty())
  {
    newSegmentationName = (nullptr!= referenceNode)? referenceNode->GetName() : "unkown";

    if (!newSegmentationName.empty())
      newSegmentationName.append("-");

    newSegmentationName.append("labels");
  }

  if (dataStorage != nullptr && dataStorage->GetNamedNode(newSegmentationName) != nullptr)
  {
    int id = 2;
    std::string suffix = "-" + std::to_string(id);

    while (dataStorage->GetNamedNode(newSegmentationName + suffix) != nullptr)
      suffix = "-" + std::to_string(++id);

    newSegmentationName.append(suffix);
  }

  if (nullptr == initialSegmentationImage)
  {
    return nullptr;
  }

  auto newLabelSetImage = mitk::MultiLabelSegmentation::New();
  try
  {
    newLabelSetImage->Initialize(initialSegmentationImage);
  }
  catch (mitk::Exception &e)
  {
    mitkReThrow(e) << "Could not initialize new label set image.";
    return nullptr;
  }

  auto newSegmentationNode = CreateEmptySegmentationNode(newSegmentationName);
  newSegmentationNode->SetData(newLabelSetImage);

  return newSegmentationNode;
}

mitk::Label::Pointer mitk::LabelSetImageHelper::CreateNewLabel(const MultiLabelSegmentation* labelSetImage, const std::string& namePrefix, bool hideIDIfUnique)
{
  if (nullptr == labelSetImage)
    return nullptr;

  const std::regex genericLabelNameRegEx(namePrefix + " ([1-9][0-9]*)");
  int maxGenericLabelNumber = 0;

  std::vector<std::array<int, 3>> colorsInUse = { {0,0,0} }; //black is always in use.

  for (auto & label : labelSetImage->GetLabels())
  {
    auto labelName = label->GetName();
    std::smatch match;

    if (std::regex_match(labelName, match, genericLabelNameRegEx))
      maxGenericLabelNumber = std::max(maxGenericLabelNumber, std::stoi(match[1].str()));

    const auto quantizedLabelColor = QuantizeColor(label->GetColor().data());

    if (std::find(colorsInUse.begin(), colorsInUse.end(), quantizedLabelColor) == std::end(colorsInUse))
      colorsInUse.push_back(quantizedLabelColor);
  }

  auto newLabel = mitk::Label::New();
  if (hideIDIfUnique && 0 == maxGenericLabelNumber)
  {
    newLabel->SetName(namePrefix);
  }
  else
  {
    newLabel->SetName(namePrefix + " " + std::to_string(maxGenericLabelNumber + 1));
  }

  auto lookupTable = mitk::LookupTable::New();
  lookupTable->SetType(mitk::LookupTable::LookupTableType::MULTILABEL);

  std::array<double, 3> lookupTableColor;
  const int maxTries = 25;
  bool newColorFound = false;

  for (int i = 0; i < maxTries; ++i)
  {
    lookupTable->GetColor(i, lookupTableColor.data());

    auto quantizedLookupTableColor = QuantizeColor(lookupTableColor.data());

    if (std::find(colorsInUse.begin(), colorsInUse.end(), quantizedLookupTableColor) == std::end(colorsInUse))
    {
      newLabel->SetColor(FromLookupTableColor(lookupTableColor.data()));
      newColorFound = true;
      break;
    }
  }

  if (!newColorFound)
  {
    lookupTable->GetColor(labelSetImage->GetTotalNumberOfLabels(), lookupTableColor.data());
    newLabel->SetColor(FromLookupTableColor(lookupTableColor.data()));
  }

  return newLabel;
}

mitk::LabelSetImageHelper::GroupIDToLabelValueMapType
mitk::LabelSetImageHelper::SplitLabelValuesByGroup(const MultiLabelSegmentation* labelSetImage, const MultiLabelSegmentation::LabelValueVectorType& labelValues)
{
  if (nullptr == labelSetImage)
    mitkThrow() << "Cannot split label values. Invalid MultiLabelSegmentation pointer passed";

  GroupIDToLabelValueMapType result;

  for (auto value : labelValues)
  {
    auto groupID = labelSetImage->GetGroupIndexOfLabel(value);

    //if groupID does not exist in result this call will also init an empty vector.
    result[groupID].push_back(value);
  }

  return result;
}

mitk::LabelSetImageHelper::LabelClassNameToLabelValueMapType
mitk::LabelSetImageHelper::SplitLabelValuesByClassName(const MultiLabelSegmentation* labelSetImage, MultiLabelSegmentation::GroupIndexType groupID)
{
  if (nullptr == labelSetImage)
    mitkThrow() << "Cannot split label values. Invalid MultiLabelSegmentation pointer passed";

  return SplitLabelValuesByClassName(labelSetImage, groupID, labelSetImage->GetLabelValuesByGroup(groupID));
}

mitk::LabelSetImageHelper::LabelClassNameToLabelValueMapType
mitk::LabelSetImageHelper::SplitLabelValuesByClassName(const MultiLabelSegmentation* labelSetImage, MultiLabelSegmentation::GroupIndexType groupID, const MultiLabelSegmentation::LabelValueVectorType& labelValues)
{
  if (nullptr == labelSetImage)
    mitkThrow() << "Cannot split label values. Invalid MultiLabelSegmentation pointer passed";

  LabelClassNameToLabelValueMapType result;

  for (const auto value : labelValues)
  {
    if (labelSetImage->GetGroupIndexOfLabel(value) == groupID)
    {
      auto className = labelSetImage->GetLabel(value)->GetName();

      //if className does not exist in result this call will also init an empty vector.
      result[className].push_back(value);
    }
  }

  return result;
}

mitk::LabelSetImageHelper::SourceToTargetGroupIDToLabelValueMappingMapType
mitk::LabelSetImageHelper::SplitLabelValueMappingBySourceAndTargetGroup(const MultiLabelSegmentation* sourceSeg, const MultiLabelSegmentation* targetSeg, const LabelValueMappingVector& labelMapping)
{
  SourceToTargetGroupIDToLabelValueMappingMapType result;

  if (nullptr == sourceSeg)
  {
    mitkThrow() << "Invalid call of SplitLabelValueMappingBySourceAndTargetGroup; sourceSeg must not be null.";
  }
  if (nullptr == targetSeg)
  {
    mitkThrow() << "Invalid call of SplitLabelValueMappingBySourceAndTargetGroup; targetSeg must not be null.";
  }

  //split all label mappings by source group id
  using GroupToLabelValueMappingMap = std::map <MultiLabelSegmentation::GroupIndexType, LabelValueMappingVector >;
  GroupToLabelValueMappingMap sourceGroupMappings;
  for (const auto& [sourceLabelValue, targetLabelValue] : labelMapping)
  {
    const auto sourceGroupID = sourceSeg->GetGroupIndexOfLabel(sourceLabelValue);
    const auto targetGroupID = targetSeg->GetGroupIndexOfLabel(targetLabelValue);

    result[sourceGroupID][targetGroupID].emplace_back(sourceLabelValue, targetLabelValue);
  }

  return result;
}


std::string mitk::LabelSetImageHelper::CreateDisplayGroupName(const MultiLabelSegmentation* labelSetImage, MultiLabelSegmentation::GroupIndexType groupID)
{
  const auto groupName = labelSetImage->GetGroupName(groupID);
  if (groupName.empty())
    return "Group "+std::to_string(groupID + 1);

  return groupName;
}

std::string mitk::LabelSetImageHelper::CreateDisplayLabelName(const MultiLabelSegmentation* labelSetImage, const Label* label)
{
  auto labelName = label->GetName();

  if (labelName.empty())
    labelName = "Unnamed";

  if (nullptr != labelSetImage &&
      labelSetImage->GetLabelValuesByName(labelSetImage->GetGroupIndexOfLabel(label->GetValue()), label->GetName()).size() > 1)
    labelName += " [" + label->GetTrackingID() + ']';

  return labelName;
}

std::string mitk::LabelSetImageHelper::CreateHTMLLabelName(const mitk::Label* label, const mitk::MultiLabelSegmentation* segmentation)
{
  std::stringstream stream;
  auto color = label->GetColor();
  stream << "<span style='color: #" << std::hex << std::setfill('0')
    << std::setw(2) << static_cast<int>(color.GetRed() * 255)
    << std::setw(2) << static_cast<int>(color.GetGreen() * 255)
    << std::setw(2) << static_cast<int>(color.GetBlue() * 255)
    << "; font-size: 20px '>&#x25A0;</span>" << std::dec;

  stream << "<font class=\"normal\"> " << CreateDisplayLabelName(segmentation, label);
  stream << "</font>";
  return stream.str();
}
