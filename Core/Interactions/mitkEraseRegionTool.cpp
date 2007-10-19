#include "mitkEraseRegionTool.h"

#include "mitkEraseRegionTool.xpm"

mitk::EraseRegionTool::EraseRegionTool()
:SetRegionTool(0)
{
  SegTool2D::SetFeedbackContourColor( 1.0, 0.0, 0.0 );
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

