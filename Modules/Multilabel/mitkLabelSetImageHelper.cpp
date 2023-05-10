/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkLabelSetImageHelper.h>

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
  const Image* initialSegmentationImage, const std::string& segmentationName)
{
  std::string newSegmentationName = segmentationName;
  if (newSegmentationName.empty())
  {
    newSegmentationName = referenceNode->GetName();
    newSegmentationName.append("-labels");
  }

  if (nullptr == initialSegmentationImage)
  {
    return nullptr;
  }

  auto newLabelSetImage = mitk::LabelSetImage::New();
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

mitk::Label::Pointer mitk::LabelSetImageHelper::CreateNewLabel(const LabelSetImage* labelSetImage, const std::string& namePrefix)
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
  newLabel->SetName(namePrefix + " " + std::to_string(maxGenericLabelNumber + 1));

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
