/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkDrawPaintbrushTool.h"

#include "mitkDrawPaintbrushTool.xpm"

// us
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleResource.h>

namespace mitk
{
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, DrawPaintbrushTool, "Paintbrush drawing tool");
}

mitk::DrawPaintbrushTool::DrawPaintbrushTool() : PaintbrushTool(1)
{
  FeedbackContourTool::SetFeedbackContourColorDefault();
}

mitk::DrawPaintbrushTool::~DrawPaintbrushTool()
{
}

const char **mitk::DrawPaintbrushTool::GetXPM() const
{
  return mitkDrawPaintbrushTool_xpm;
}

us::ModuleResource mitk::DrawPaintbrushTool::GetIconResource() const
{
  us::Module *module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("Paint_48x48.png");
  return resource;
}

us::ModuleResource mitk::DrawPaintbrushTool::GetCursorIconResource() const
{
  us::Module *module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("Paint_Cursor_32x32.png");
  return resource;
}

const char *mitk::DrawPaintbrushTool::GetName() const
{
  return "Paint";
}
