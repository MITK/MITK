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

#include "mitkFillRegionTool.h"

#include "mitkFillRegionTool.xpm"

namespace mitk {
  MITK_TOOL_MACRO(MITKEXT_CORE_EXPORT, FillRegionTool, "Fill tool");
}

mitk::FillRegionTool::FillRegionTool()
:SetRegionTool(1)
{
}

mitk::FillRegionTool::~FillRegionTool()
{
}

const char** mitk::FillRegionTool::GetXPM() const
{
  return mitkFillRegionTool_xpm;
}

const char* mitk::FillRegionTool::GetName() const
{
  return "Fill";
}

