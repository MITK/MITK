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

#include "mitkRandomTrackingDevice.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <mitkTimeStamp.h>
#include <itksys/SystemTools.hxx>


mitk::RandomTrackingDevice::RandomTrackingDevice() : mitk::TrackingDevice(), 
  m_AllTools(), m_MultiThreader(NULL), m_ThreadID(-1), m_RefreshRate(20), m_NumberOfControlPoints(20),
  m_Interpolators(), m_SplineLengths(), m_ToolSpeeds()
{
  //set the type of this tracking device
  this->m_Type = mitk::TrackingSystemNotSpecified;
  
  m_Bounds[0] = m_Bounds[2] = m_Bounds[4] = -400.0;  // initialize bounds to -400 ... +400 (mm) cube
  m_Bounds[1] = m_Bounds[3] = m_Bounds[5] =  400.0;
  
  m_RefreshRate = 20;
  this->m_MultiThreader = itk::MultiThreader::New();
}


mitk::RandomTrackingDevice::~RandomTrackingDevice()
{
  if (GetMode() == Tracking)
  {
    this->StopTracking();
  }
  if (GetMode() == Ready)
  {
    this->CloseConnection();
  }
  /* cleanup tracking thread */
  if ((m_ThreadID != 0) && (m_MultiThreader.IsNotNull()))
  {
    m_MultiThreader->TerminateThread(m_ThreadID);
  }
  m_MultiThreader = NULL;
  m_AllTools.clear();
}


mitk::TrackingTool* mitk::RandomTrackingDevice::AddTool(const char* toolName)
{
  mitk::InternalTrackingTool::Pointer t = mitk::InternalTrackingTool::New();
  t->SetToolName(toolName);
  m_AllTools.push_back(t);
  m_ToolSpeeds.push_back(0.1); // standard speed of the tool is 10 seconds per cycle
  return t;
}


bool mitk::RandomTrackingDevice::StartTracking()
{

  this->SetMode(Tracking);            // go to mode Tracking
  this->m_StopTrackingMutex->Lock();  
  this->m_StopTracking = false;
  this->m_StopTrackingMutex->Unlock();

  m_TrackingFinishedMutex->Unlock(); // transfer the execution rights to tracking thread

  mitk::TimeStamp::GetInstance()->StartTracking(this);

  m_ThreadID = m_MultiThreader->SpawnThread(this->ThreadStartTracking, this);    // start a new thread that executes the TrackTools() method
  return true;
}


bool mitk::RandomTrackingDevice::StopTracking()
{
  if (this->GetMode() == Tracking) // Only if the object is in the correct state
  {
    m_StopTrackingMutex->Lock();  // m_StopTracking is used by two threads, so we have to ensure correct thread handling
    m_StopTracking = true;
    m_StopTrackingMutex->Unlock();
    this->SetMode(Ready);
  }

  mitk::TimeStamp::GetInstance()->StopTracking(this);
  m_TrackingFinishedMutex->Lock();

  return true;
}


unsigned int mitk::RandomTrackingDevice::GetToolCount() const
{
  return (unsigned int)this->m_AllTools.size();
}


mitk::TrackingTool* mitk::RandomTrackingDevice::GetTool(unsigned int toolNumber) const
{
  if ( toolNumber >= this->GetToolCount()) 
    return NULL;
  else 
    return this->m_AllTools[toolNumber];
}


bool mitk::RandomTrackingDevice::OpenConnection()
{
  /* create bspline for all tools */
  if (m_NumberOfControlPoints < 1)
  {
    this->SetErrorMessage("to few control points for spline interpolation");
    return false;
  }
  //Init Random
  srand(time(NULL));

  for (ToolContainer::iterator itAllTools = m_AllTools.begin(); itAllTools != m_AllTools.end(); itAllTools++)  // for each tool
  {
    SplineType::Pointer spline = SplineType::New(); // create spline
    SplineType::ControlPointListType controlPoints;
    controlPoints.push_back(this->GetRandomPoint()); // insert point 0
    double length = 0.0;  // estimate spline lenght by calculating line segments lengths
    for (unsigned int i = 1; i < m_NumberOfControlPoints - 1; ++i) // set points 1..n-2
    {
      SplineType::ControlPointType pos;
      pos = this->GetRandomPoint();        
      length += controlPoints.at(i-1).EuclideanDistanceTo(pos);
      controlPoints.push_back(pos);
    }
    controlPoints.push_back(controlPoints.at(0));  // close spline --> insert point n-1
    length += controlPoints.at(controlPoints.size() - 2).EuclideanDistanceTo(controlPoints.at(controlPoints.size() - 1));
    spline->SetPoints(controlPoints); // SetControlPoints() ???
    m_Interpolators.push_back(spline);
    m_SplineLengths.push_back(length);
  }
  std::cout << "Splines: number: " << m_Interpolators.size() << "\n";
  for (unsigned int i = 0; i < m_Interpolators.size(); ++i)
    std::cout << "Splines " << i << ": " << m_Interpolators[i] << ", Points: " << /*m_Interpolators[i]->GetPoints() << */".\n"; 

  this->SetMode(Ready);
  return true;
}


bool mitk::RandomTrackingDevice::CloseConnection()
{
  bool returnValue = true; 
  if(this->GetMode() == Setup)
    return true;

//  returnValue = m_Device->StopTracking();
//  delete m_Device;

  this->SetMode(Setup);
  return returnValue;
}


mitk::ScalarType mitk::RandomTrackingDevice::GetSplineChordLength(unsigned int idx)
{
  if (idx >= m_SplineLengths.size())
    throw std::invalid_argument("No chord length available for this index");
  return m_SplineLengths.at(idx);
}

void mitk::RandomTrackingDevice::SetToolSpeed(unsigned int idx, mitk::ScalarType roundsPerSecond)
{
  if (idx >= m_ToolSpeeds.size())
    throw std::invalid_argument("No tool available for this index");
  if (roundsPerSecond < 0.0001)
    throw std::invalid_argument("Minimum tool speed is 0.0001 rounds per second");

  m_ToolSpeeds.at(idx) = roundsPerSecond;
}


void mitk::RandomTrackingDevice::TrackTools()
{
  try
  {
    /* lock the TrackingFinishedMutex to signal that the execution rights are now transfered to the tracking thread */
    m_TrackingFinishedMutex->Lock();

    bool localStopTracking;       // Because m_StopTracking is used by two threads, access has to be guarded by a mutex. To minimize thread locking, a local copy is used here 
    this->m_StopTrackingMutex->Lock();  // update the local copy of m_StopTracking
    localStopTracking = this->m_StopTracking;
    this->m_StopTrackingMutex->Unlock();
    while ((this->GetMode() == Tracking) && (localStopTracking == false))
    {
      for (ToolContainer::iterator itAllTools = m_AllTools.begin(); itAllTools != m_AllTools.end(); itAllTools++)
      {
        mitk::InternalTrackingTool::Pointer currentTool = *itAllTools;

        mitk::Point3D pos;
        mitk::Quaternion quat;

// TODO: read position from spline
        pos = this->GetRandomPoint();
        currentTool->SetPosition(pos);

        //generate numbers between 0 and 1
        quat[0] = rand() / (RAND_MAX + 1.0);
        quat[1] = rand() / (RAND_MAX + 1.0);
        quat[2] = rand() / (RAND_MAX + 1.0);
        quat[3] = rand() / (RAND_MAX + 1.0);       
        currentTool->SetOrientation(quat);
// todo: rotate once per cylcle around fixed rotation vector

        currentTool->SetTrackingError( 2 * (rand() / (RAND_MAX + 1.0)));  // tracking errror in 0 .. 2 Range
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
    this->StopTracking();
    this->SetErrorMessage("Error while trying to track tools. Thread stopped.");
  }
}


ITK_THREAD_RETURN_TYPE mitk::RandomTrackingDevice::ThreadStartTracking(void* pInfoStruct)
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
  RandomTrackingDevice *trackingDevice = static_cast<RandomTrackingDevice*>(pInfo->UserData);

  if (trackingDevice != NULL) 
    trackingDevice->TrackTools();

  return ITK_THREAD_RETURN_VALUE;
}


mitk::Point3D mitk::RandomTrackingDevice::GetRandomPoint()
{
  mitk::Point3D pos;
  pos[0] = m_Bounds[0] + (m_Bounds[1] - m_Bounds[0]) * (rand() / (RAND_MAX + 1.0));  // X =  xMin + xRange * (random number between 0 and 1)
  pos[1] = m_Bounds[2] + (m_Bounds[3] - m_Bounds[2]) * (rand() / (RAND_MAX + 1.0));  // Y
  pos[2] = m_Bounds[4] + (m_Bounds[5] - m_Bounds[4]) * (rand() / (RAND_MAX + 1.0));  // Z
  return pos;
}
