/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkErasePaintbrushTool.h"

#include "mitkErasePaintbrushTool.xpm"

// us
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleResource.h>

namespace mitk
{
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, ErasePaintbrushTool, "Paintbrush erasing tool");
}

mitk::ErasePaintbrushTool::ErasePaintbrushTool() : PaintbrushTool(false)
{
}

mitk::ErasePaintbrushTool::~ErasePaintbrushTool()
{
}

const char **mitk::ErasePaintbrushTool::GetXPM() const
{
  return mitkErasePaintbrushTool_xpm;
}

us::ModuleResource mitk::ErasePaintbrushTool::GetIconResource() const
{
  us::Module *module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("Wipe.svg");
  return resource;
}

us::ModuleResource mitk::ErasePaintbrushTool::GetCursorIconResource() const
{
  us::Module *module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("Wipe_Cursor.svg");
  return resource;
}

const char *mitk::ErasePaintbrushTool::GetName() const
{
  return "Wipe";
}
