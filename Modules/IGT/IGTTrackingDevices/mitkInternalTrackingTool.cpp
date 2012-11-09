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

#include "mitkInternalTrackingTool.h"

#include <itkMutexLockHolder.h>

typedef itk::MutexLockHolder<itk::FastMutexLock> MutexLockHolder;

mitk::InternalTrackingTool::InternalTrackingTool()
: TrackingTool(),
m_TrackingError(0.0f),
m_Enabled(true),
m_DataValid(false),
m_ToolTipSet(false)
{
  m_Position[0] = 0.0f;
  m_Position[1] = 0.0f;
  m_Position[2] = 0.0f;
  m_Orientation[0] = 0.0f;
  m_Orientation[1] = 0.0f;
  m_Orientation[2] = 0.0f;
  m_Orientation[3] = 0.0f;
  // this should not be necessary as the tools bring their own tooltip transformation
  m_ToolTip[0] = 0.0f;
  m_ToolTip[1] = 0.0f;
  m_ToolTip[2] = 0.0f;
  m_ToolTipRotation[0] = 0.0f;
  m_ToolTipRotation[1] = 0.0f;
  m_ToolTipRotation[2] = 0.0f;
  m_ToolTipRotation[3] = 1.0f;
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
  if (m_ToolTipSet)
    {
    //compute position of tooltip
    vnl_vector<float> pos_vnl = (m_ToolTipRotation.rotate(m_Position.Get_vnl_vector()))+ m_ToolTip.Get_vnl_vector();
    position[0] = pos_vnl[0];
    position[1] = pos_vnl[1];
    position[2] = pos_vnl[2];
    }
  else
    {
    position[0] = m_Position[0];
    position[1] = m_Position[1];
    position[2] = m_Position[2];
    }
  this->Modified();
}


void mitk::InternalTrackingTool::SetPosition(mitk::Point3D position)
{
  itkDebugMacro("setting  m_Position to " << position);
  MutexLockHolder lock(*m_MyMutex); // lock and unlock the mutex
  m_Position = position;
  this->Modified();
}


void mitk::InternalTrackingTool::GetOrientation(mitk::Quaternion& orientation) const
{
  MutexLockHolder lock(*m_MyMutex); // lock and unlock the mutex
  if (m_ToolTipSet)
    {
    //compute rotation of tooltip
    orientation = m_ToolTipRotation * m_Orientation;
    }
  else
    {
    orientation = m_Orientation;
    }
}

void mitk::InternalTrackingTool::SetToolTip(mitk::Point3D toolTipPosition, mitk::Quaternion orientation)
{
if( (toolTipPosition[0] == 0) &&
    (toolTipPosition[1] == 0) &&
    (toolTipPosition[2] == 0) &&
    (orientation.x() == 0) &&
    (orientation.y() == 0) &&
    (orientation.z() == 0) &&
    (orientation.r() == 1))
    {
    m_ToolTipSet = false;
    }
else
    {
    m_ToolTipSet = true;
    }
m_ToolTip = toolTipPosition;
m_ToolTipRotation = orientation;
}

void mitk::InternalTrackingTool::SetOrientation(mitk::Quaternion orientation)
{
  itkDebugMacro("setting  m_Orientation to " << orientation);
  MutexLockHolder lock(*m_MyMutex); // lock and unlock the mutex
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
