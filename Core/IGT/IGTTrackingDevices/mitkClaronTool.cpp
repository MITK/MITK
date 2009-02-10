#include "mitkClaronTool.h"
#include <stdio.h>
#include <vector>
#include <iostream>

mitk::ClaronTool::ClaronTool()
  {
  }

mitk::ClaronTool::~ClaronTool(void)
  {
  }

std::string mitk::ClaronTool::GetCalibrationName()
  {
  return m_calibrationName;
  }

bool mitk::ClaronTool::LoadFile(const char* filename)
  {
  return this->LoadFile( std::string(filename));
  }

bool mitk::ClaronTool::LoadFile(std::string filename)
  {
  m_filename = filename;

  int end = m_filename.length();
  int start = end;

  //check wether the path is given in Windows format
  while( (start!=0) && (filename[start-1]!='\\') ) start--;

  //if not (start==0) perhaps it is given in Linux format
  if (start==0) {start = end; while( (start!=0) && (filename[start-1]!='/') ) start--;}

  //if there are no \ and no / in the string something must be wrong...
  if (start==0) return false;

  this->m_calibrationName = m_filename.substr(start,end);

  return true;
  }

const char* mitk::ClaronTool::GetFile()
  {
  return m_filename.c_str();
  }

void mitk::ClaronTool::SetToolHandle (mitk::claronToolHandle handle)
  {
  this->toolHandle = handle;
  }

mitk::claronToolHandle mitk::ClaronTool::GetToolHandle()
  {
  return this->toolHandle;
  }

void mitk::ClaronTool::GetPosition (float &x, float &y, float &z) const
  {
  x = m_Position[0];
  y = m_Position[1];
  z = m_Position[2];
  }

void mitk::ClaronTool::SetPosition (float x, float y, float z)
  {
  m_MyMutex->Lock();
  m_Position[0] = x;
  m_Position[1] = y;
  m_Position[2] = z;
  //if the tool coordinates are 0,0,0 the tool wasn't found, so the data is not valid
  if (x==0 && y==0 && z==0) m_DataValid = false;
  else m_DataValid = true;
  m_MyMutex->Unlock();
  }

void mitk::ClaronTool::GetQuaternion (float &q0, float &q1, float &q2, float &q3) const
  {
  m_MyMutex->Lock();
  q0 = m_Quaternion[0];
  q1 = m_Quaternion[1];
  q2 = m_Quaternion[2];
  q3 = m_Quaternion[3];
  m_MyMutex->Unlock();
  }

void mitk::ClaronTool::GetQuaternion(mitk::Quaternion& orientation) const
{
  m_MyMutex->Lock();
  orientation.r() = m_Quaternion[0];
  orientation.x() = m_Quaternion[1];
  orientation.y() = m_Quaternion[2];
  orientation.z() = m_Quaternion[3];
  m_MyMutex->Unlock();
}


void mitk::ClaronTool::GetPosition(mitk::Point3D& position) const
{
  m_MyMutex->Lock();
  position[0] = m_Position[0];
  position[1] = m_Position[1];
  position[2] = m_Position[2];
  m_MyMutex->Unlock();
}

void mitk::ClaronTool::SetQuaternion (float q0, float q1, float q2, float q3)
  {
  m_MyMutex->Lock();
  m_Quaternion[0]=q0;
  m_Quaternion[1]=q1;
  m_Quaternion[2]=q2;
  m_Quaternion[3]=q3;
  m_MyMutex->Unlock();
  }


void mitk::ClaronTool::SetToolName(std::string name)
  {
  this->m_ToolName=name;
  }

void mitk::ClaronTool::SetCalibrationName(std::string name)
  {
  this->m_calibrationName=name;
  }

std::string mitk::ClaronTool::GetToolName()
  {
  return this->m_ToolName;
  }

bool mitk::ClaronTool::Enable()
  {
  m_MyMutex->Lock();
  this->m_Enabled = true;
  m_MyMutex->Unlock();
  return true;
  }

bool mitk::ClaronTool::Disable()
  {
  m_MyMutex->Lock();
  this->m_Enabled = false;
  m_MyMutex->Unlock();
  return true;
  }

bool mitk::ClaronTool::IsEnabled() const
  {
  return this->m_Enabled;
  }

bool mitk::ClaronTool::IsDataValid() const
  {
  return m_DataValid;
  return false;
  }

float mitk::ClaronTool::GetTrackingError() const
  {
  //GEERBT -> noch zu implementieren!
  return 0;
  }
