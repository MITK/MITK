/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "mitkEraseRegionTool.h"

#include "mitkEraseRegionTool.xpm"

namespace mitk {
  MITK_TOOL_MACRO(MITKEXT_CORE_EXPORT, EraseRegionTool, "Erase tool");
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

const char* mitk::EraseRegionTool::GetName() const
{
  return "Erase";
}

