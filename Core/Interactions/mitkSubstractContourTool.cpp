#include "mitkSubstractContourTool.h"

#include "mitkSubstractContourTool.xpm"

mitk::SubstractContourTool::SubstractContourTool()
:ContourTool(0)
{
  SegTool2D::SetFeedbackContourColor( 1.0, 0.0, 0.0 );
}

mitk::SubstractContourTool::~SubstractContourTool()
{
}

const char** mitk::SubstractContourTool::GetXPM() const
{
  return mitkSubstractContourTool_xpm;
}

const char* mitk::SubstractContourTool::GetName() const
{
  return "Substract";
}

