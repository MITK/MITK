/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: $
Version:   $Revision: $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkVirtualTrackingDevice.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <mitkTimeStamp.h>
#include <itksys/SystemTools.hxx>
#include <itkMutexLockHolder.h>

typedef itk::MutexLockHolder<itk::FastMutexLock> MutexLockHolder;


mitk::VirtualTrackingDevice::VirtualTrackingDevice() : mitk::TrackingDevice(), 
  m_AllTools(), m_ToolsMutex(NULL), m_MultiThreader(NULL), m_ThreadID(-1), m_RefreshRate(100), m_NumberOfControlPoints(20)
{ 
  m_Type = mitk::VirtualTrackingDevice;
  m_Bounds[0] = m_Bounds[2] = m_Bounds[4] = -400.0;  // initialize bounds to -400 ... +400 (mm) cube
  m_Bounds[1] = m_Bounds[3] = m_Bounds[5] =  400.0;
  m_ToolsMutex = itk::FastMutexLock::New();
}


mitk::VirtualTrackingDevice::~VirtualTrackingDevice()
{
  if (this->GetState() == Tracking)
  {
    this->StopTracking();
  }
  if (this->GetState() == Ready)
  {
    this->CloseConnection();
  }
  /* cleanup tracking thread */
  if (m_MultiThreader.IsNotNull() && (m_ThreadID != -1))
  {
    m_MultiThreader->TerminateThread(m_ThreadID);
    m_MultiThreader = NULL;
  }  
  m_AllTools.clear();
}


mitk::TrackingTool* mitk::VirtualTrackingDevice::AddTool(const char* toolName)
{
  //if (this->GetState() == Tracking)
  //{
  //  return NULL;
  //}
  mitk::VirtualTrackingTool::Pointer t = mitk::VirtualTrackingTool::New();
  t->SetToolName(toolName);
  t->SetVelocity(0.1);
  this->InitializeSpline(t);
  MutexLockHolder lock(*m_ToolsMutex); // lock and unlock the mutex
  m_AllTools.push_back(t);
  return t;
}


bool mitk::VirtualTrackingDevice::StartTracking()
{
  if (this->GetState() != Ready)
    return false;
  this->SetState(Tracking);            // go to mode Tracking
  this->m_StopTrackingMutex->Lock();  
  this->m_StopTracking = false;
  this->m_StopTrackingMutex->Unlock();

  m_TrackingFinishedMutex->Unlock(); // transfer the execution rights to tracking thread

  mitk::TimeStamp::GetInstance()->Start(this);
  
  if (m_MultiThreader.IsNotNull() && (m_ThreadID != -1))
    m_MultiThreader->TerminateThread(m_ThreadID);
  if (m_MultiThreader.IsNull())
    m_MultiThreader = itk::MultiThreader::New();

  m_ThreadID = m_MultiThreader->SpawnThread(this->ThreadStartTracking, this);    // start a new thread that executes the TrackTools() method
  return true;
}


bool mitk::VirtualTrackingDevice::StopTracking()
{
  if (this->GetState() == Tracking) // Only if the object is in the correct state
  {
    m_StopTrackingMutex->Lock();  // m_StopTracking is used by two threads, so we have to ensure correct thread handling
    m_StopTracking = true;
    m_StopTrackingMutex->Unlock();
    this->SetState(Ready);
  }

  mitk::TimeStamp::GetInstance()->Stop(this);
  m_TrackingFinishedMutex->Lock();

  return true;
}


unsigned int mitk::VirtualTrackingDevice::GetToolCount() const
{
  MutexLockHolder lock(*m_ToolsMutex); // lock and unlock the mutex
  return static_cast<unsigned int>(this->m_AllTools.size());
}


mitk::TrackingTool* mitk::VirtualTrackingDevice::GetTool(unsigned int toolNumber) const
{
  MutexLockHolder lock(*m_ToolsMutex); // lock and unlock the mutex
  if ( toolNumber < m_AllTools.size()) 
    return this->m_AllTools.at(toolNumber);
  return NULL;
}


bool mitk::VirtualTrackingDevice::OpenConnection()
{
  if (m_NumberOfControlPoints < 1)
  {
    this->SetErrorMessage("to few control points for spline interpolation");
    return false;
  }
  srand(time(NULL)); //Init random number generator

  this->SetState(Ready);
  return true;
}


void mitk::VirtualTrackingDevice::InitializeSpline( mitk::VirtualTrackingTool* t )
{
  if (t == NULL)
  return;

  typedef mitk::VirtualTrackingTool::SplineType SplineType;
  /* create random control points */
  SplineType::ControlPointListType controlPoints;
  controlPoints.reserve(m_NumberOfControlPoints + 1);

  controlPoints.push_back(this->GetRandomPoint()); // insert point 0
  double length = 0.0;  // estimate spline length by calculating line segments lengths
  for (unsigned int i = 1; i < m_NumberOfControlPoints - 1; ++i) // set points 1..n-2
  {
    SplineType::ControlPointType pos;
    pos = this->GetRandomPoint();
    length += controlPoints.at(i - 1).EuclideanDistanceTo(pos);
    controlPoints.push_back(pos);
  }
  controlPoints.push_back(controlPoints.at(0));  // close spline --> insert point last control point with same value as first control point
  length += controlPoints.at(controlPoints.size() - 2).EuclideanDistanceTo(controlPoints.at(controlPoints.size() - 1));

  /* Create knot list. TODO: rethink knot list values and list size. Is there a better solution? */
  SplineType::KnotListType knotList;
  knotList.push_back(0.0);
  for (unsigned int i = 1; i < controlPoints.size() + t->GetSpline()->GetSplineOrder() + 1; ++i)
    knotList.push_back(i);
  knotList.push_back(controlPoints.size() + t->GetSpline()->GetSplineOrder() + 1);

  t->GetSpline()->SetControlPoints(controlPoints);
  t->GetSpline()->SetKnots(knotList);
  t->SetSplineLength(length);
}


bool mitk::VirtualTrackingDevice::CloseConnection()
{
  bool returnValue = true; 
  if(this->GetState() == Setup)
    return true;

  this->SetState(Setup);
  return returnValue;
}


mitk::ScalarType mitk::VirtualTrackingDevice::GetSplineChordLength(unsigned int idx)
{
  mitk::VirtualTrackingTool* t = this->GetInternalTool(idx);
  if (t != NULL)
    return t->GetSplineLength();
  else
    throw std::invalid_argument("invalid index");
}


void mitk::VirtualTrackingDevice::SetToolSpeed(unsigned int idx, mitk::ScalarType roundsPerSecond)
{ 
  if (roundsPerSecond < 0.0001)
    throw std::invalid_argument("Minimum tool speed is 0.0001 rounds per second");

    mitk::VirtualTrackingTool* t = this->GetInternalTool(idx);
    if (t != NULL)
      t->SetVelocity(roundsPerSecond);
    else
      throw std::invalid_argument("invalid index");
}


mitk::VirtualTrackingTool* mitk::VirtualTrackingDevice::GetInternalTool(unsigned int idx)
{
  MutexLockHolder toolsMutexLockHolder(*m_ToolsMutex); // lock and unlock the mutex
  if (idx < m_AllTools.size())
    return m_AllTools.at(idx);
  else
    return NULL;
}


void mitk::VirtualTrackingDevice::TrackTools()
{
  try
  {
    /* lock the TrackingFinishedMutex to signal that the execution rights are now transfered to the tracking thread */
    MutexLockHolder trackingFinishedLockHolder(*m_TrackingFinishedMutex); // keep lock until end of scope
    bool localStopTracking;       // Because m_StopTracking is used by two threads, access has to be guarded by a mutex. To minimize thread locking, a local copy is used here 
    this->m_StopTrackingMutex->Lock();  // update the local copy of m_StopTracking
    localStopTracking = this->m_StopTracking;
    this->m_StopTrackingMutex->Unlock();
    mitk::ScalarType t = 0.0;
    while ((this->GetState() == Tracking) && (localStopTracking == false))
    {
      ToolContainer::size_type toolCount = this->GetToolCount();

      //for (ToolContainer::iterator itAllTools = m_AllTools.begin(); itAllTools != m_AllTools.end(); itAllTools++)
      for (unsigned int i = 0; i < this->GetToolCount(); ++i)
      {
        mitk::VirtualTrackingTool::Pointer currentTool = this->GetInternalTool(i);
        mitk::VirtualTrackingTool::SplineType::PointType pos;
        /* calculate tool position with spline interpolation */
        pos = currentTool->GetSpline()->EvaluateSpline(t);
        mitk::Point3D mp;
        mitk::itk2vtk(pos, mp); // convert from SplineType::PointType to mitk::Point3D
        currentTool->SetPosition(mp);
        // Currently, a constant speed is used. TODO: use tool velocity setting
        t += 0.001;
        if (t >= 1.0)
          t = 0.0;

        mitk::Quaternion quat;
        /* fix quaternion rotation */
        quat.x() = 1.0;
        quat.y() = 1.0;
        quat.z() = 1.0;
        quat.r() = 1.0;
        currentTool->SetOrientation(quat);
        // TODO: rotate once per cycle around a fixed rotation vector

        currentTool->SetTrackingError( 2 * (rand() / (RAND_MAX + 1.0)));  // tracking error in 0 .. 2 Range
        currentTool->SetDataValid(true);
      }
      itksys::SystemTools::Delay(m_RefreshRate);
      /* Update the local copy of m_StopTracking */
      this->m_StopTrackingMutex->Lock();  
      localStopTracking = m_StopTracking;
      this->m_StopTrackingMutex->Unlock();
    } // tracking ends if we pass this line

    m_TrackingFinishedMutex->Unlock(); // transfer control back to main thread
  }
  catch(...)
  {
    m_TrackingFinishedMutex->Unlock();
    this->StopTracking();
    this->SetErrorMessage("Error while trying to track tools. Thread stopped.");
  }
}


ITK_THREAD_RETURN_TYPE mitk::VirtualTrackingDevice::ThreadStartTracking(void* pInfoStruct)
{
  /* extract this pointer from Thread Info structure */
  struct itk::MultiThreader::ThreadInfoStruct * pInfo = (struct itk::MultiThreader::ThreadInfoStruct*)pInfoStruct;
  if (pInfo == NULL)
  {
    return ITK_THREAD_RETURN_VALUE;
  }
  if (pInfo->UserData == NULL)
  {
    return ITK_THREAD_RETURN_VALUE;
  }
  VirtualTrackingDevice *trackingDevice = static_cast<VirtualTrackingDevice*>(pInfo->UserData);

  if (trackingDevice != NULL) 
    trackingDevice->TrackTools();

  trackingDevice->m_ThreadID = -1; // reset thread ID because we end the thread here
  return ITK_THREAD_RETURN_VALUE;
}


mitk::VirtualTrackingDevice::ControlPointType mitk::VirtualTrackingDevice::GetRandomPoint()
{
  ControlPointType pos;
  pos[0] = m_Bounds[0] + (m_Bounds[1] - m_Bounds[0]) * (rand() / (RAND_MAX + 1.0));  // X =  xMin + xRange * (random number between 0 and 1)
  pos[1] = m_Bounds[2] + (m_Bounds[3] - m_Bounds[2]) * (rand() / (RAND_MAX + 1.0));  // Y
  pos[2] = m_Bounds[4] + (m_Bounds[5] - m_Bounds[4]) * (rand() / (RAND_MAX + 1.0));  // Z
  return pos;
}
