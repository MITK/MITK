/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkFastMarchingTool3D.h"

namespace mitk
{
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, FastMarchingTool3D, "FastMarching3D tool");
}

mitk::FastMarchingTool3D::FastMarchingTool3D()
  : FastMarchingBaseTool(3)
{
}

mitk::FastMarchingTool3D::~FastMarchingTool3D()
{
}

const char *mitk::FastMarchingTool3D::GetName() const
{
  return "Fast Marching 3D";
}
