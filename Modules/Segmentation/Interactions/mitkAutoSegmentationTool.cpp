/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkAutoSegmentationTool.h"
#include "mitkImage.h"
#include "mitkToolManager.h"
#include <mitkImageTimeSelector.h>

mitk::AutoSegmentationTool::AutoSegmentationTool() : Tool("dummy"), m_OverwriteExistingSegmentation(false)
{
}

mitk::AutoSegmentationTool::~AutoSegmentationTool()
{
}

const char *mitk::AutoSegmentationTool::GetGroup() const
{
  return "autoSegmentation";
}

mitk::Image::ConstPointer mitk::AutoSegmentationTool::Get3DImage(const mitk::Image* image, unsigned int timestep) const
{
  if (nullptr == image)
    return image;

  if (image->GetDimension() != 4)
    return image;

  mitk::ImageTimeSelector::Pointer imageTimeSelector = mitk::ImageTimeSelector::New();

  imageTimeSelector->SetInput(image);
  imageTimeSelector->SetTimeNr(static_cast<int>(timestep));

  imageTimeSelector->UpdateLargestPossibleRegion();

  return imageTimeSelector->GetOutput();
}

mitk::Image::ConstPointer mitk::AutoSegmentationTool::Get3DImageByTimePoint(const mitk::Image* image, TimePointType timePoint) const
{
  if (nullptr == image)
    return image;

  if (!image->GetTimeGeometry()->IsValidTimePoint(timePoint))
    return nullptr;

  return this->Get3DImage(image, image->GetTimeGeometry()->TimePointToTimeStep(timePoint));
}

void mitk::AutoSegmentationTool::SetOverwriteExistingSegmentation(bool overwrite)
{
  m_OverwriteExistingSegmentation = overwrite;
}

std::string mitk::AutoSegmentationTool::GetCurrentSegmentationName()
{
  if (m_ToolManager->GetWorkingData(0))
    return m_ToolManager->GetWorkingData(0)->GetName();
  else
    return "";
}

mitk::DataNode *mitk::AutoSegmentationTool::GetTargetSegmentationNode()
{
  mitk::DataNode::Pointer segmentationNode = m_ToolManager->GetWorkingData(0);
  if (!m_OverwriteExistingSegmentation)
  {
    mitk::DataNode::Pointer refNode = m_ToolManager->GetReferenceData(0);
    if (refNode.IsNull())
    {
      // TODO create and use segmentation exceptions instead!!
      MITK_ERROR << "No valid reference data!";
      return nullptr;
    }

    std::string nodename = refNode->GetName() + "_" + this->GetName();
    mitk::Color color;
    color.SetRed(1);
    color.SetBlue(0);
    color.SetGreen(0);
    //create a new segmentation node based on the current segmentation as template
    segmentationNode = CreateEmptySegmentationNode(dynamic_cast<mitk::Image *>(segmentationNode->GetData()), nodename, color);

    m_ToolManager->GetDataStorage()->Add(segmentationNode, refNode);
  }
  return segmentationNode;
}
