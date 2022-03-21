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

  auto newSegmentationNode = mitk::DataNode::New();
  newSegmentationNode->SetData(newLabelSetImage);
  newSegmentationNode->SetName(newSegmentationName);

  // set additional image information
  newLabelSetImage->GetExteriorLabel()->SetProperty("name.parent", mitk::StringProperty::New(referenceNode->GetName()));
  newLabelSetImage->GetExteriorLabel()->SetProperty("name.image", mitk::StringProperty::New(newSegmentationName));

  // initialize "showVolume"-property to false to prevent recalculating the volume while working on the segmentation
  newSegmentationNode->SetProperty("showVolume", mitk::BoolProperty::New(false));

  return newSegmentationNode;
}

mitk::Label::Pointer mitk::LabelSetImageHelper::CreateNewLabel(const LabelSetImage* labelSetImage)
{
  int numberOfLabels = labelSetImage->GetActiveLabelSet()->GetNumberOfLabels();

  std::string labelName = "New label " + std::to_string(numberOfLabels);

  mitk::LookupTable::Pointer lookupTable = mitk::LookupTable::New();
  lookupTable->SetType(mitk::LookupTable::LookupTableType::MULTILABEL);
  double rgb[3];
  lookupTable->GetColor(numberOfLabels, rgb);
  mitk::Color labelColor;
  labelColor.Set(static_cast<float>(rgb[0]), static_cast<float>(rgb[1]), static_cast<float>(rgb[2]));

  mitk::Label::Pointer newLabel = mitk::Label::New();
  newLabel->SetName(labelName);
  newLabel->SetColor(labelColor);

  return newLabel;
}
