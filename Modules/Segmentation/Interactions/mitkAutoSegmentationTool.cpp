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

mitk::AutoSegmentationTool::AutoSegmentationTool()
:Tool("dummy")
{
}

mitk::AutoSegmentationTool::~AutoSegmentationTool()
{
}

const char* mitk::AutoSegmentationTool::GetGroup() const
{
  return "autoSegmentation";
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

