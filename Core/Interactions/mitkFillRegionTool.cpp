#include "mitkFillRegionTool.h"

#include "mitkFillRegionTool.xpm"

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

