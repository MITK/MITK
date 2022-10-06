/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkAddContourTool.h"

#include "mitkAddContourTool.xpm"

// us
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleResource.h>

namespace mitk
{
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, AddContourTool, "Add tool");
}

mitk::AddContourTool::AddContourTool() : ContourTool(1)
{
}

mitk::AddContourTool::~AddContourTool()
{
}

const char **mitk::AddContourTool::GetXPM() const
{
  return mitkAddContourTool_xpm;
}

us::ModuleResource mitk::AddContourTool::GetIconResource() const
{
  us::Module *module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("Add.svg");
  return resource;
}

us::ModuleResource mitk::AddContourTool::GetCursorIconResource() const
{
  us::Module *module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("Add_Cursor.svg");
  return resource;
}

const char *mitk::AddContourTool::GetName() const
{
  return "Add";
}
