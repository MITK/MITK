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

#include "mitkTrackingDevice.h"
#include "mitkTimeStamp.h"
#include "mitkTrackingTool.h"
#include <itkMutexLockHolder.h>

typedef itk::MutexLockHolder<itk::FastMutexLock> MutexLockHolder;


mitk::TrackingDevice::TrackingDevice() :
  m_Type(TrackingSystemNotSpecified),
  m_State(mitk::TrackingDevice::Setup),
  m_StopTracking(false), m_ErrorMessage("")

{
  m_StopTrackingMutex = itk::FastMutexLock::New();
  m_StateMutex = itk::FastMutexLock::New();
  m_TrackingFinishedMutex = itk::FastMutexLock::New();
  m_TrackingFinishedMutex->Lock();  // execution rights are owned by the application thread at the beginning
}


mitk::TrackingDevice::~TrackingDevice()
{
  m_TrackingFinishedMutex = NULL;
  m_StopTrackingMutex = NULL;
}


mitk::TrackingDevice::TrackingDeviceState mitk::TrackingDevice::GetState() const
{
  MutexLockHolder lock(*m_StateMutex);
  return m_State;
}


void mitk::TrackingDevice::SetState( TrackingDeviceState state )
{
  itkDebugMacro("setting  m_State to " << state);

  MutexLockHolder lock(*m_StateMutex); // lock and unlock the mutex 
  if (m_State == state)
  { 
    return;
  }
  m_State = state;
  this->Modified();
}


bool mitk::TrackingDevice::StopTracking()
{
  if (this->GetState() == Tracking) // Only if the object is in the correct state
  {
    m_StopTrackingMutex->Lock();  // m_StopTracking is used by two threads, so we have to ensure correct thread handling
    m_StopTracking = true;
    m_StopTrackingMutex->Unlock();
    //we have to wait here that the other thread recognizes the STOP-command and executes it
    m_TrackingFinishedMutex->Lock();
    mitk::TimeStamp::GetInstance()->Stop(this); // notify realtime clock
    // StopTracking was called, thus the mode should be changed back
    //   to Ready now that the tracking loop has ended.
    this->SetState(Ready);
  }
  return true;
}


mitk::TrackingTool* mitk::TrackingDevice::GetToolByName( std::string name ) const
{
  unsigned int toolCount = this->GetToolCount();
  for (unsigned int i = 0; i < toolCount; ++i)
    if (name == this->GetTool(i)->GetToolName())
      return this->GetTool(i);
  return NULL;
}

