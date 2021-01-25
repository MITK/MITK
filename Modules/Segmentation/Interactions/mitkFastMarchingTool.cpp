/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkFastMarchingTool.h"


namespace mitk
{
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, FastMarchingTool, "FastMarching2D tool");
}

mitk::FastMarchingTool::FastMarchingTool()
  : FastMarchingBaseTool(2)
{
}

mitk::FastMarchingTool::~FastMarchingTool()
{
}

const char *mitk::FastMarchingTool::GetName() const
{
  return "2D Fast Marching";
}

