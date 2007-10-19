#include "mitkAddContourTool.h"

#include "mitkAddContourTool.xpm"

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

