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
#include <mitkLabelSetImageHelper.h>

mitk::AutoSegmentationTool::AutoSegmentationTool() : Tool("dummy"), m_OverwriteExistingSegmentation(false)
{
}

mitk::AutoSegmentationTool::AutoSegmentationTool(const char* interactorType, const us::Module* interactorModule) : Tool(interactorType, interactorModule), m_OverwriteExistingSegmentation(false)
{
}

mitk::AutoSegmentationTool::~AutoSegmentationTool()
{
}

void mitk::AutoSegmentationTool::Activated()
{
  Superclass::Activated();

  m_NoneOverwriteTargetSegmentationNode = nullptr;
}

void mitk::AutoSegmentationTool::Deactivated()
{
  m_NoneOverwriteTargetSegmentationNode = nullptr;

  Superclass::Deactivated();
}

const char *mitk::AutoSegmentationTool::GetGroup() const
{
  return "autoSegmentation";
}

mitk::Image::ConstPointer mitk::AutoSegmentationTool::GetImageByTimeStep(const mitk::Image* image, unsigned int timestep)
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

mitk::Image::Pointer mitk::AutoSegmentationTool::GetImageByTimeStep(mitk::Image* image, unsigned int timestep)
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

mitk::Image::ConstPointer mitk::AutoSegmentationTool::GetImageByTimePoint(const mitk::Image* image, TimePointType timePoint)
{
  if (nullptr == image)
    return image;

  if (!image->GetTimeGeometry()->IsValidTimePoint(timePoint))
    return nullptr;

  return AutoSegmentationTool::GetImageByTimeStep(image, image->GetTimeGeometry()->TimePointToTimeStep(timePoint));
}

void mitk::AutoSegmentationTool::SetOverwriteExistingSegmentation(bool overwrite)
{
  if (m_OverwriteExistingSegmentation != overwrite)
  {
    m_OverwriteExistingSegmentation = overwrite;
    m_NoneOverwriteTargetSegmentationNode = nullptr;
  }
}

std::string mitk::AutoSegmentationTool::GetCurrentSegmentationName()
{
  if (this->GetToolManager()->GetWorkingData(0))
    return this->GetToolManager()->GetWorkingData(0)->GetName();
  else
    return "";
}

mitk::DataNode *mitk::AutoSegmentationTool::GetTargetSegmentationNode() const
{
  mitk::DataNode::Pointer segmentationNode = this->GetToolManager()->GetWorkingData(0);
  if (!m_OverwriteExistingSegmentation)
  {
    if (m_NoneOverwriteTargetSegmentationNode.IsNull())
    {
      mitk::DataNode::Pointer refNode = this->GetToolManager()->GetReferenceData(0);
      if (refNode.IsNull())
      {
        // TODO create and use segmentation exceptions instead!!
        MITK_ERROR << "No valid reference data!";
        return nullptr;
      }
      std::string nodename = refNode->GetName() + "_" + this->GetName();

      const auto labelSetImage = dynamic_cast<mitk::LabelSetImage*>(segmentationNode->GetData());
      if (nullptr == labelSetImage)
      {
        //TODO: this part of the if statement is old legacy code and should be removed.
        //Keept because I didn't want to break/rework to many things before
        //the release 2022.04. Should be removed when the seg tool classes are streamlined and the
        //multi data structure is the only one used in seg APIs and code.

        mitk::Color color;
        color.SetRed(1);
        color.SetBlue(0);
        color.SetGreen(0);
        //create a new segmentation node based on the current segmentation as template
        m_NoneOverwriteTargetSegmentationNode = CreateEmptySegmentationNode(dynamic_cast<mitk::Image*>(segmentationNode->GetData()), nodename, color);
      }
      else
      {
        auto clonedSegmentation = labelSetImage->Clone();
        m_NoneOverwriteTargetSegmentationNode = LabelSetImageHelper::CreateEmptySegmentationNode(nodename);
        m_NoneOverwriteTargetSegmentationNode->SetData(clonedSegmentation);
      }
    }
    segmentationNode = m_NoneOverwriteTargetSegmentationNode;
  }
  return segmentationNode;
}

void mitk::AutoSegmentationTool::EnsureTargetSegmentationNodeInDataStorage() const
{
  auto targetNode = this->GetTargetSegmentationNode();
  if (!this->GetToolManager()->GetDataStorage()->Exists(targetNode))
  {
    this->GetToolManager()->GetDataStorage()->Add(targetNode, this->GetToolManager()->GetReferenceData(0));
  }
}
