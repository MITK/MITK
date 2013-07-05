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

#include "mitkSubtractContourTool.h"

#include "mitkSubtractContourTool.xpm"

namespace mitk {
  MITK_TOOL_MACRO(Segmentation_EXPORT, SubtractContourTool, "Subtract tool");
}

mitk::SubtractContourTool::SubtractContourTool()
:ContourTool(0)
{
  FeedbackContourTool::SetFeedbackContourColor( 1.0, 0.0, 0.0 );
}

mitk::SubtractContourTool::~SubtractContourTool()
{
}

const char** mitk::SubtractContourTool::GetXPM() const
{
  return mitkSubtractContourTool_xpm;
}

std::string mitk::SubtractContourTool::GetIconPath() const
{
  return ":/Segmentation/Subtract_48x48.png";
}

std::string mitk::SubtractContourTool::GetCursorIconPath() const
{
  return ":/Segmentation/Subtract_Cursor_32x32.png";
}

const char* mitk::SubtractContourTool::GetName() const
{
  return "Subtract";
}

