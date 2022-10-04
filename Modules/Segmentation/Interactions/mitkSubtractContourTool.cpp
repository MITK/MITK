/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSubtractContourTool.h"

#include "mitkSubtractContourTool.xpm"

// us
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleResource.h>

namespace mitk
{
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, SubtractContourTool, "Subtract tool");
}

mitk::SubtractContourTool::SubtractContourTool() : ContourTool(0)
{
  FeedbackContourTool::SetFeedbackContourColor(1.0, 0.0, 0.0);
}

mitk::SubtractContourTool::~SubtractContourTool()
{
}

const char **mitk::SubtractContourTool::GetXPM() const
{
  return mitkSubtractContourTool_xpm;
}

us::ModuleResource mitk::SubtractContourTool::GetIconResource() const
{
  us::Module *module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("Subtract.svg");
  return resource;
}

us::ModuleResource mitk::SubtractContourTool::GetCursorIconResource() const
{
  us::Module *module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("Subtract_Cursor.svg");
  return resource;
}

const char *mitk::SubtractContourTool::GetName() const
{
  return "Subtract";
}
