/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2007-07-10 17:24:20 +0200 (Di, 10 Jul 2007) $
Version:   $Revision: 11220 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkMicroBirdTool.h"

mitk::MicroBirdTool::MicroBirdTool() 
: TrackingTool(), 
  m_TrackingError(0.0f),
  m_Enabled(true),
  m_DataValid(false)
{
  m_Position[0] = 0.0f;
  m_Position[1] = 0.0f;
  m_Position[2] = 0.0f;
  m_Quaternion[0] = 0.0f;
  m_Quaternion[1] = 0.0f;
  m_Quaternion[2] = 0.0f;
  m_Quaternion[3] = 0.0f;
}

mitk::MicroBirdTool::~MicroBirdTool()
{
  m_MyMutex = NULL;
}

void mitk::MicroBirdTool::GetPosition(mitk::Point3D& position) const
{
  m_MyMutex->Lock();
  position[0] = m_Position[0];
  position[1] = m_Position[1];
  position[2] = m_Position[2];
  m_MyMutex->Unlock();

  // Debugging output
  //std::cout << "Position got: " << position << "\n";
}

void mitk::MicroBirdTool::SetPosition(float x, float y, float z)
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

void mitk::MicroBirdTool::GetQuaternion(mitk::Quaternion& orientation) const
{
  m_MyMutex->Lock();
  orientation.r() = m_Quaternion[0];
  orientation.x() = m_Quaternion[1];
  orientation.y() = m_Quaternion[2];
  orientation.z() = m_Quaternion[3];
  m_MyMutex->Unlock();

  // Debugging output
  //std::cout << "Orientation got: " << orientation << "\n";
}

void mitk::MicroBirdTool::SetQuaternion(float q0, float qx, float qy, float qz)
{
  m_MyMutex->Lock();
  bool modified = false;
  if (m_Quaternion[0] != q0)
  {
    m_Quaternion[0] = q0;
    modified = true;
  }
  if (m_Quaternion[1] != qx)
  {
    m_Quaternion[1] = qx;
    modified = true;
  }
  if (m_Quaternion[2] != qy)
  {
    m_Quaternion[2] = qy;
    modified = true;
  }
  if (m_Quaternion[3] != qz)
  {
    m_Quaternion[3] = qz;
    modified = true;
  }
  if (modified == true)
    this->Modified();
  m_MyMutex->Unlock();
}

void mitk::MicroBirdTool::SetTrackingError(float error)
{
  m_MyMutex->Lock();
  m_TrackingError = error;
  m_MyMutex->Unlock();
}

float mitk::MicroBirdTool::GetTrackingError() const
{
  m_MyMutex->Lock();
  float r = m_TrackingError;
  m_MyMutex->Unlock();
  return r;
}

bool mitk::MicroBirdTool::Enable()
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

bool mitk::MicroBirdTool::Disable()
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

bool mitk::MicroBirdTool::IsEnabled() const
{
  m_MyMutex->Lock();
  bool e = m_Enabled;
  m_MyMutex->Unlock();
  return e;
}

bool mitk::MicroBirdTool::IsDataValid() const
{
  m_MyMutex->Lock();
  bool b = m_DataValid;
  m_MyMutex->Unlock();
  return b;
}

void mitk::MicroBirdTool::SetErrorMessage(const char* _arg)
{
  itkDebugMacro("setting m_ErrorMessage to " << _arg);
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


void mitk::MicroBirdTool::SetDataValid(bool _arg)
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
