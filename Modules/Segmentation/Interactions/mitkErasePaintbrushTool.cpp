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

const char* mitk::ErasePaintbrushTool::GetName() const
{
  return "Wipe";
}

