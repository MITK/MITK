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
#include "mitkModule.h"
#include "mitkModuleResource.h"
#include <mitkGetModuleContext.h>

namespace mitk {
  MITK_TOOL_MACRO(Segmentation_EXPORT, DrawPaintbrushTool, "Paintbrush drawing tool");
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

mitk::ModuleResource mitk::DrawPaintbrushTool::GetIconResource() const
{
  Module* module = GetModuleContext()->GetModule();
  ModuleResource resource = module->GetResource("Paint_48x48.png");
  return resource;
}

std::string mitk::DrawPaintbrushTool::GetCursorIconPath() const
{
  return ":/Segmentation/Paint_Cursor_48x48.png";
}

const char* mitk::DrawPaintbrushTool::GetName() const
{
  return "Paint";
}

