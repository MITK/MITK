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

#include "mitkAddContourTool.h"

#include "mitkAddContourTool.xpm"

namespace mitk {
  MITK_TOOL_MACRO(MITKEXT_CORE_EXPORT, AddContourTool, "Add tool");
}

mitk::AddContourTool::AddContourTool()
:ContourTool(1)
{
}

mitk::AddContourTool::~AddContourTool()
{
}

const char** mitk::AddContourTool::GetXPM() const
{
  return mitkAddContourTool_xpm;
}

const char* mitk::AddContourTool::GetName() const
{
  return "Add";
}

