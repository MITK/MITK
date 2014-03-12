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

#include "mitkAutoSegmentationTool.h"
#include "mitkToolManager.h"
#include "mitkImage.h"
#include <mitkImageTimeSelector.h>

mitk::AutoSegmentationTool::AutoSegmentationTool()
:Tool("dummy"),
m_OverwriteExistingSegmentation (false)
{
}

mitk::AutoSegmentationTool::~AutoSegmentationTool()
{
}

const char* mitk::AutoSegmentationTool::GetGroup() const
{
  return "autoSegmentation";
}

mitk::Image::Pointer mitk::AutoSegmentationTool::Get3DImage(mitk::Image::Pointer image, unsigned int timestep)
{
  if (image->GetDimension() != 4)
    return image;

  mitk::ImageTimeSelector::Pointer imageTimeSelector = mitk::ImageTimeSelector::New();

  imageTimeSelector->SetInput(image);
  imageTimeSelector->SetTimeNr(static_cast<int>(timestep));

  imageTimeSelector->UpdateLargestPossibleRegion();

  return imageTimeSelector->GetOutput();
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

mitk::DataNode* mitk::AutoSegmentationTool::GetTargetSegmentationNode()
{
  mitk::DataNode::Pointer emptySegmentation;
  if (m_OverwriteExistingSegmentation)
  {
    emptySegmentation = m_ToolManager->GetWorkingData(0);
  }
  else
  {
    mitk::DataNode::Pointer refNode = m_ToolManager->GetReferenceData(0);
    if (refNode.IsNull())
    {
      //TODO create and use segmentation exceptions instead!!
      MITK_ERROR<<"No valid reference data!";
      return NULL;
    }
    std::string nodename = m_ToolManager->GetReferenceData(0)->GetName()+"_"+this->GetName();
    mitk::Color color;
    color.SetRed(1);
    color.SetBlue(0);
    color.SetGreen(0);
    emptySegmentation = CreateEmptySegmentationNode(dynamic_cast<mitk::Image*>(refNode->GetData()), nodename, color);
    m_ToolManager->GetDataStorage()->Add(emptySegmentation, refNode);

  }
  return emptySegmentation;
}

