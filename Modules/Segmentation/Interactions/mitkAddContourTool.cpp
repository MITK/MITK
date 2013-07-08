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

// us
#include "mitkModule.h"
#include "mitkModuleResource.h"
#include <mitkGetModuleContext.h>

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

mitk::ModuleResource mitk::AddContourTool::GetIconResource() const
{
  Module* module = GetModuleContext()->GetModule();
  ModuleResource resource = module->GetResource("Add_48x48.png");
  return resource;
}

mitk::ModuleResource mitk::AddContourTool::GetCursorIconResource() const
{
  Module* module = GetModuleContext()->GetModule();
  ModuleResource resource = module->GetResource("Add_Cursor_32x32.png");
  return resource;
}

const char* mitk::AddContourTool::GetName() const
{
  return "Add";
}

