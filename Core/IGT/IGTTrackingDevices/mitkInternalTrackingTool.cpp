/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

#include "mitkInternalTrackingTool.h"



mitk::InternalTrackingTool::InternalTrackingTool() 
: TrackingTool(),
m_TrackingError(0.0f), 
m_Enabled(true), 
m_DataValid(false)
{
  m_Position[0] = 0.0f;
  m_Position[1] = 0.0f;
  m_Position[2] = 0.0f;
  m_Orientation[0] = 0.0f;
  m_Orientation[1] = 0.0f;
  m_Orientation[2] = 0.0f;
  m_Orientation[3] = 0.0f;
}

mitk::InternalTrackingTool::~InternalTrackingTool()
{
}


void mitk::InternalTrackingTool::SetToolName(const char* _arg)
{
  m_MyMutex->Lock();
  if ( _arg && (_arg == this->m_ToolName) ) 
  { 
    m_MyMutex->Unlock();
    return;
  }
  if (_arg)
    {
    this->m_ToolName= _arg;
    }
    else
    {
    this->m_ToolName= "";
    }
  this->Modified(); 
  m_MyMutex->Unlock();
} 


void mitk::InternalTrackingTool::GetPosition(mitk::Point3D& position) const
{
  m_MyMutex->Lock();
  position[0] = m_Position[0];
  position[1] = m_Position[1];
  position[2] = m_Position[2];
  m_MyMutex->Unlock();
}


void mitk::InternalTrackingTool::SetPosition(float x, float y, float z)
{
  m_MyMutex->Lock();
  bool modified = false;
  if (m_Position[0] != x)
  {
    m_Position[0] = x;
    modified = true;
  }
  if (m_Position[1] != y)
  {
    m_Position[1] = y;
    modified = true;
  }
  if (m_Position[2] != z)
  {
    m_Position[2] = z;
    modified = true;
  }
  if (modified == true)
    this->Modified();
  m_MyMutex->Unlock();
}


void mitk::InternalTrackingTool::GetQuaternion(mitk::Quaternion& orientation) const
{
  m_MyMutex->Lock();
  orientation.r() = m_Orientation[0];
  orientation.x() = m_Orientation[1];
  orientation.y() = m_Orientation[2];
  orientation.z() = m_Orientation[3];
  m_MyMutex->Unlock();
}


void mitk::InternalTrackingTool::SetQuaternion(float q0, float qx, float qy, float qz)
{
  m_MyMutex->Lock();
  bool modified = false;
  if (m_Orientation[0] != q0)
  {
    m_Orientation[0] = q0;
    modified = true;
  }
  if (m_Orientation[1] != qx)
  {
    m_Orientation[1] = qx;
    modified = true;
  }
  if (m_Orientation[2] != qy)
  {
    m_Orientation[2] = qy;
    modified = true;
  }
  if (m_Orientation[3] != qz)
  {
    m_Orientation[3] = qz;
    modified = true;
  }
  if (modified == true)
    this->Modified();
  m_MyMutex->Unlock();
}


void mitk::InternalTrackingTool::SetTrackingError(float error)
{
  m_MyMutex->Lock();
  m_TrackingError = error;
  m_MyMutex->Unlock();
}


float mitk::InternalTrackingTool::GetTrackingError() const
{
  m_MyMutex->Lock();
  float r = m_TrackingError;
  m_MyMutex->Unlock();
  return r;

}


bool mitk::InternalTrackingTool::Enable()
{
  m_MyMutex->Lock();
  if (IsEnabled() == false)
  {
    this->m_Enabled = true;
    this->Modified();
  }
  m_MyMutex->Unlock();
  return true;
}


bool mitk::InternalTrackingTool::Disable()
{
  m_MyMutex->Lock();
  if (IsEnabled() == true)
  {
    this->m_Enabled = false;
    this->Modified();
  }
  m_MyMutex->Unlock();
  return true;
}


bool mitk::InternalTrackingTool::IsEnabled() const
{
  m_MyMutex->Lock();
  bool e = m_Enabled;
  m_MyMutex->Unlock();
  return e;
}


bool mitk::InternalTrackingTool::IsDataValid() const
{
  m_MyMutex->Lock();
  bool b = m_DataValid;
  m_MyMutex->Unlock();
  return b;
}


void mitk::InternalTrackingTool::SetDataValid(bool _arg)
{  
  itkDebugMacro("setting m_DataValid to " << _arg);
  if (this->m_DataValid != _arg)
  {
    m_MyMutex->Lock();
    this->m_DataValid = _arg;
    this->Modified();
    m_MyMutex->Unlock();
  }  
}


void mitk::InternalTrackingTool::SetErrorMessage(const char* _arg)
{
  itkDebugMacro("setting  m_ErrorMessage  to " << _arg);
  m_MyMutex->Lock();
  if ((_arg == NULL) || (_arg == this->m_ErrorMessage)) 
  { 
    m_MyMutex->Unlock();
    return;
  }
  if (_arg != NULL)
    this->m_ErrorMessage = _arg;
  else
    this->m_ErrorMessage = "";

  this->Modified(); 
  m_MyMutex->Unlock();
} 
