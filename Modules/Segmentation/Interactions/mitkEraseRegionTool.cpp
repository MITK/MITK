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

namespace mitk {
  MITK_TOOL_MACRO(Segmentation_EXPORT, EraseRegionTool, "Erase tool");
}

mitk::EraseRegionTool::EraseRegionTool()
:SetRegionTool(0)
{
  FeedbackContourTool::SetFeedbackContourColor( 1.0, 0.0, 0.0 );
}

mitk::EraseRegionTool::~EraseRegionTool()
{
}

const char** mitk::EraseRegionTool::GetXPM() const
{
  return mitkEraseRegionTool_xpm;
}

std::string mitk::EraseRegionTool::GetIconPath() const
{
  return ":/Segmentation/Erase_48x48.png";
}

const char* mitk::EraseRegionTool::GetName() const
{
  return "Erase";
}

