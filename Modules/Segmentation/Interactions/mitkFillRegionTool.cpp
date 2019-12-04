/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkFillRegionTool.h"

#include "mitkFillRegionTool.xpm"

// us
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleResource.h>

namespace mitk
{
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, FillRegionTool, "Fill tool");
}

mitk::FillRegionTool::FillRegionTool() : SetRegionTool(1)
{
}

mitk::FillRegionTool::~FillRegionTool()
{
}

const char **mitk::FillRegionTool::GetXPM() const
{
  return mitkFillRegionTool_xpm;
}

us::ModuleResource mitk::FillRegionTool::GetIconResource() const
{
  us::Module *module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("Fill_48x48.png");
  return resource;
}

us::ModuleResource mitk::FillRegionTool::GetCursorIconResource() const
{
  us::Module *module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("Fill_Cursor_32x32.png");
  return resource;
}

const char *mitk::FillRegionTool::GetName() const
{
  return "Fill";
}
