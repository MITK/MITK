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

#include "mitkErasePaintbrushTool.h"

#include "mitkErasePaintbrushTool.xpm"

// us
#include <usModule.h>
#include <usModuleResource.h>
#include <usGetModuleContext.h>
#include <usModuleContext.h>

namespace mitk {
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, ErasePaintbrushTool, "Paintbrush erasing tool");
}

mitk::ErasePaintbrushTool::ErasePaintbrushTool()
:PaintbrushTool(0)
{
  FeedbackContourTool::SetFeedbackContourColor( 1.0, 0.0, 0.0 );
}

mitk::ErasePaintbrushTool::~ErasePaintbrushTool()
{
}

const char** mitk::ErasePaintbrushTool::GetXPM() const
{
  return mitkErasePaintbrushTool_xpm;
}

us::ModuleResource mitk::ErasePaintbrushTool::GetIconResource() const
{
  us::Module* module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("Wipe_48x48.png");
  return resource;
}

us::ModuleResource mitk::ErasePaintbrushTool::GetCursorIconResource() const
{
  us::Module* module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("Wipe_Cursor_32x32.png");
  return resource;
}

const char* mitk::ErasePaintbrushTool::GetName() const
{
  return "Wipe";
}

