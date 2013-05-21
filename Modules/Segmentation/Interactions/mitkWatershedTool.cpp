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

#include "mitkWatershedTool.h"

#include "mitkBinaryThresholdTool.xpm"
#include "mitkToolManager.h"

namespace mitk {
  MITK_TOOL_MACRO(Segmentation_EXPORT, WatershedTool, "Watershed tool");
}


mitk::WatershedTool::WatershedTool()
{
}

mitk::WatershedTool::~WatershedTool()
{
}

void mitk::WatershedTool::Activated()
{
  Superclass::Activated();
}

void mitk::WatershedTool::Deactivated()
{
  Superclass::Deactivated();
}

const char** mitk::WatershedTool::GetXPM() const
{
  return mitkBinaryThresholdTool_xpm;
}

const char* mitk::WatershedTool::GetName() const
{
  return "Watershedding";
}
