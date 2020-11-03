/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkClaronTool.h"
#include <cstdio>
#include <vector>
#include <iostream>

mitk::ClaronTool::ClaronTool() : TrackingTool()
{
}

mitk::ClaronTool::~ClaronTool(void)
{
}

std::string mitk::ClaronTool::GetCalibrationName()
{
  return &m_CalibrationName[0];
}

void mitk::ClaronTool::SetCalibrationName(std::string name)
{
  this->m_CalibrationName=name;
}

bool mitk::ClaronTool::LoadFile(const char* filename)
{
  if (filename==nullptr)
  {
    return false;
  }
  else
  {
    return this->LoadFile(std::string(filename));
  }
}

bool mitk::ClaronTool::LoadFile(std::string filename)
{
  //This method is not really "loading" a file. It is saving the filename and
  //parsing the calibration name out of the filename. The calibration name is
  //later used by the tracking device to really load the file.

  if (filename.empty())
  {
    return false;
  }
  else
  {
    m_Filename = filename;

    int end = m_Filename.length();
    int start = end;

    //check whether the path is given in Windows format
    while( (start!=0) && (filename[start-1]!='\\') ) start--;

    //if not (start==0) perhaps it is given in Linux format
    if (start==0)
    {
      start = end;
      while( (start!=0) && (filename[start-1]!='/') )
      {
        start--;
      }
    }

    //if there are no \ and no / in the string something must be wrong...
    if (start==0) return false;

    this->m_CalibrationName = m_Filename.substr(start,end);

    return true;
  }
}

std::string mitk::ClaronTool::GetFile()
{
  return m_Filename;
}

void mitk::ClaronTool::SetToolHandle (mitk::claronToolHandle handle)
{
  this->m_ToolHandle = handle;
}

mitk::claronToolHandle mitk::ClaronTool::GetToolHandle()
{
  return this->m_ToolHandle;
}
