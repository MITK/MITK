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
#include "mitkModule.h"
#include "mitkModuleResource.h"
#include <mitkGetModuleContext.h>

namespace mitk {
  MITK_TOOL_MACRO(Segmentation_EXPORT, ErasePaintbrushTool, "Paintbrush erasing tool");
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

mitk::ModuleResource mitk::ErasePaintbrushTool::GetIconResource() const
{
  Module* module = GetModuleContext()->GetModule();
  ModuleResource resource = module->GetResource("Wipe_48x48.png");
  return resource;
}

std::string mitk::ErasePaintbrushTool::GetCursorIconPath() const
{
  return ":/Segmentation/Wipe_Cursor_48x48.png";
}

const char* mitk::ErasePaintbrushTool::GetName() const
{
  return "Wipe";
}

