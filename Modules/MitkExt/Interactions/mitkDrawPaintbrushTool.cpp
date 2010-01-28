/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
Language:  C++
Date:      $Date$
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "mitkDrawPaintbrushTool.h"

#include "mitkDrawPaintbrushTool.xpm"

namespace mitk {
  MITK_TOOL_MACRO(MitkExt_EXPORT, DrawPaintbrushTool, "Paintbrush drawing tool");
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

const char* mitk::DrawPaintbrushTool::GetName() const
{
  return "Paint";
}

