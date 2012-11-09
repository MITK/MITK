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

#include "mitkDrawPaintbrushTool.h"

#include "mitkDrawPaintbrushTool.xpm"

namespace mitk {
  MITK_TOOL_MACRO(Segmentation_EXPORT, DrawPaintbrushTool, "Paintbrush drawing tool");
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

