#include "mitkClaronTool.h"
#include <stdio.h>
#include <vector>
#include <iostream>


mitk::ClaronTool::ClaronTool() :InternalTrackingTool()
{
}


mitk::ClaronTool::~ClaronTool(void)
{
}


std::string mitk::ClaronTool::GetCalibrationName()
{
  return m_CalibrationName;
}


void mitk::ClaronTool::SetCalibrationName(std::string name)
{
  this->m_CalibrationName=name;
}


bool mitk::ClaronTool::LoadFile(const char* filename)
{
  return this->LoadFile( std::string(filename));
}


bool mitk::ClaronTool::LoadFile(std::string filename)
{
  m_Filename = filename;

  int end = m_Filename.length();
  int start = end;

  //check whether the path is given in Windows format
  while( (start!=0) && (filename[start-1]!='\\') ) start--;

  //if not (start==0) perhaps it is given in Linux format
  if (start==0) {start = end; while( (start!=0) && (filename[start-1]!='/') ) start--;}

  //if there are no \ and no / in the string something must be wrong...
  if (start==0) return false;

  this->m_CalibrationName = m_Filename.substr(start,end);

  return true;
}


const char* mitk::ClaronTool::GetFile()
{
  return m_Filename.c_str();
}


void mitk::ClaronTool::SetToolHandle (mitk::claronToolHandle handle)
{
  this->m_ToolHandle = handle;
}


mitk::claronToolHandle mitk::ClaronTool::GetToolHandle()
{
  return this->m_ToolHandle;
}
