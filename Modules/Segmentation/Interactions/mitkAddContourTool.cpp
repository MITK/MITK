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

#include "mitkAddContourTool.h"

#include "mitkAddContourTool.xpm"

namespace mitk {
  MITK_TOOL_MACRO(Segmentation_EXPORT, AddContourTool, "Add tool");
}

mitk::AddContourTool::AddContourTool()
:ContourTool(1)
{
}

mitk::AddContourTool::~AddContourTool()
{
}

const char** mitk::AddContourTool::GetXPM() const
{
  return mitkAddContourTool_xpm;
}

const char* mitk::AddContourTool::GetName() const
{
  return "Add";
}

