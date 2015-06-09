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

#include "mitkDrawPaintbrushTool.h"

#include "mitkDrawPaintbrushTool.xpm"

// us
#include <usModule.h>
#include <usModuleResource.h>
#include <usGetModuleContext.h>
#include <usModuleContext.h>

namespace mitk {
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, DrawPaintbrushTool, "Paintbrush drawing tool");
}

mitk::DrawPaintbrushTool::DrawPaintbrushTool()
:PaintbrushTool(1)
{
}

mitk::DrawPaintbrushTool::~DrawPaintbrushTool()
{
}

const char** mitk::DrawPaintbrushTool::GetXPM() const
{
  return mitkDrawPaintbrushTool_xpm;
}

us::ModuleResource mitk::DrawPaintbrushTool::GetIconResource() const
{
  us::Module* module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("Paint_48x48.png");
  return resource;
}

us::ModuleResource mitk::DrawPaintbrushTool::GetCursorIconResource() const
{
  us::Module* module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("Paint_Cursor_32x32.png");
  return resource;
}

const char* mitk::DrawPaintbrushTool::GetName() const
{
  return "Paint";
}

