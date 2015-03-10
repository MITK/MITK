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

#include "mitkEraseRegionTool.h"

#include "mitkEraseRegionTool.xpm"

// us
#include <usModule.h>
#include <usModuleResource.h>
#include <usGetModuleContext.h>
#include <usModuleContext.h>

namespace mitk {
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, EraseRegionTool, "Erase tool");
}

mitk::EraseRegionTool::EraseRegionTool()
:SetRegionTool(0)
{
  FeedbackContourTool::SetFeedbackContourColor( 1.0, 1.0, 0.0 );
}

mitk::EraseRegionTool::~EraseRegionTool()
{
}

const char** mitk::EraseRegionTool::GetXPM() const
{
  return mitkEraseRegionTool_xpm;
}

us::ModuleResource mitk::EraseRegionTool::GetIconResource() const
{
  us::Module* module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("Erase_48x48.png");
  return resource;
}

us::ModuleResource mitk::EraseRegionTool::GetCursorIconResource() const
{
  us::Module* module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("Erase_Cursor_32x32.png");
  return resource;
}

const char* mitk::EraseRegionTool::GetName() const
{
  return "Erase";
}

