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

#include "mitkAdaptiveRegionGrowingTool.h"

#include "mitkToolManager.h"


namespace mitk {
  MITK_TOOL_MACRO(Segmentation_EXPORT, AdaptiveRegionGrowingTool, "AdaptiveRegionGrowingTool");
}

mitk::AdaptiveRegionGrowingTool::AdaptiveRegionGrowingTool()
{

}

mitk::AdaptiveRegionGrowingTool::~AdaptiveRegionGrowingTool()
{
}

const char** mitk::AdaptiveRegionGrowingTool::GetXPM() const
{
  return NULL;
}

const char* mitk::AdaptiveRegionGrowingTool::GetName() const
{
  return "RegionGrowing";
}

std::string mitk::AdaptiveRegionGrowingTool::GetIconPath() const
{
  return ":/Segmentation/RegionGrowing_48x48.png";
}

void mitk::AdaptiveRegionGrowingTool::Activated()
{

}

void mitk::AdaptiveRegionGrowingTool::Deactivated()
{
}

mitk::DataNode* mitk::AdaptiveRegionGrowingTool::GetReferenceData(){
  return this->m_ToolManager->GetReferenceData(0);
}

mitk::DataStorage* mitk::AdaptiveRegionGrowingTool::GetDataStorage(){
  return this->m_ToolManager->GetDataStorage();
}

mitk::DataNode* mitk::AdaptiveRegionGrowingTool::GetWorkingData(){
  return this->m_ToolManager->GetWorkingData(0);
}
