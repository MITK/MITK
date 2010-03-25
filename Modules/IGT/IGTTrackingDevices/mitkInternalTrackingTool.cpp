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

#include <itkMutexLockHolder.h>

typedef itk::MutexLockHolder<itk::FastMutexLock> MutexLockHolder;

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
  itkDebugMacro("setting  m_ToolName to " << _arg);
  MutexLockHolder lock(*m_MyMutex); // lock and unlock the mutex
  if ( _arg && (_arg == this->m_ToolName) ) 
  { 
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
} 


void mitk::InternalTrackingTool::SetToolName( const std::string _arg )
{
  this->SetToolName(_arg.c_str());
}


void mitk::InternalTrackingTool::GetPosition(mitk::Point3D& position) const
{
  MutexLockHolder lock(*m_MyMutex); // lock and unlock the mutex
  position[0] = m_Position[0];
  position[1] = m_Position[1];
  position[2] = m_Position[2];
  this->Modified();
}


void mitk::InternalTrackingTool::SetPosition(mitk::Point3D position)
{
  itkDebugMacro("setting  m_Position to " << position);

  MutexLockHolder lock(*m_MyMutex); // lock and unlock the mutex 
  if (mitk::Equal(position, m_Position))
  { 
    return;
  }
  m_Position = position;
  this->Modified();
}


void mitk::InternalTrackingTool::GetOrientation(mitk::Quaternion& orientation) const
{
  MutexLockHolder lock(*m_MyMutex); // lock and unlock the mutex 
  orientation = m_Orientation;
}


void mitk::InternalTrackingTool::SetOrientation(mitk::Quaternion orientation)
{
  itkDebugMacro("setting  m_Orientation to " << orientation);
  MutexLockHolder lock(*m_MyMutex); // lock and unlock the mutex 
  if (mitk::Equal(orientation, m_Orientation))
  { 
    return;
  }
  m_Orientation = orientation;
  this->Modified();
}


void mitk::InternalTrackingTool::SetTrackingError(float error)
{
  itkDebugMacro("setting  m_TrackingError  to " << error);
  MutexLockHolder lock(*m_MyMutex); // lock and unlock the mutex 
  if (error == m_TrackingError)
  { 
    return;
  }
  m_TrackingError = error;
  this->Modified();
}


float mitk::InternalTrackingTool::GetTrackingError() const
{
  MutexLockHolder lock(*m_MyMutex); // lock and unlock the mutex 
  float r = m_TrackingError;
  return r;
}


bool mitk::InternalTrackingTool::Enable()
{
  MutexLockHolder lock(*m_MyMutex); // lock and unlock the mutex 
  if (m_Enabled == false)
  {
    this->m_Enabled = true;
    this->Modified();
  }  
  return true;
}


bool mitk::InternalTrackingTool::Disable()
{
  MutexLockHolder lock(*m_MyMutex); // lock and unlock the mutex 
  if (m_Enabled == true)
  {
    this->m_Enabled = false;
    this->Modified();
  }
  return true;
}


bool mitk::InternalTrackingTool::IsEnabled() const
{
  MutexLockHolder lock(*m_MyMutex); // lock and unlock the mutex 
  return m_Enabled;
}


bool mitk::InternalTrackingTool::IsDataValid() const
{
  MutexLockHolder lock(*m_MyMutex); // lock and unlock the mutex 
  return m_DataValid;
}


void mitk::InternalTrackingTool::SetDataValid(bool _arg)
{  
  itkDebugMacro("setting m_DataValid to " << _arg);
  if (this->m_DataValid != _arg)
  {
    MutexLockHolder lock(*m_MyMutex); // lock and unlock the mutex 
    this->m_DataValid = _arg;
    this->Modified();
  }  
}


void mitk::InternalTrackingTool::SetErrorMessage(const char* _arg)
{
  itkDebugMacro("setting  m_ErrorMessage  to " << _arg);
  MutexLockHolder lock(*m_MyMutex); // lock and unlock the mutex 
  if ((_arg == NULL) || (_arg == this->m_ErrorMessage)) 
    return;

  if (_arg != NULL)
    this->m_ErrorMessage = _arg;
  else
    this->m_ErrorMessage = "";
  this->Modified(); 
} 
