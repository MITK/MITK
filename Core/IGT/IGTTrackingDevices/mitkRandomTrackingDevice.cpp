#include "mitkRandomTrackingDevice.h"

//#include <itksys/SystemTools.hxx>
//#include <iostream>

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <mitkTimeStamp.h>

//for the pause 
#include <itksys/SystemTools.hxx>

mitk::RandomTrackingDevice::~RandomTrackingDevice()
{
}


bool mitk::RandomTrackingDevice::AddTool(InternalTrackingTool::Pointer tool)
{
  m_AllTools.push_back(tool);
  return true;
}


mitk::RandomTrackingDevice::RandomTrackingDevice(void)
{
  //set the type of this tracking device
  this->m_Type = mitk::TrackingSystemNotSpecified;

  m_RefreshRate = 50;
  //set the tracking volume
  //this->m_TrackingVolume->SetTrackingDeviceType(this->m_Type);

  this->m_MultiThreader = itk::MultiThreader::New();
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


mitk::TrackingTool* mitk::RandomTrackingDevice::GetTool(unsigned int toolNumber)
{
  if ( toolNumber >= this->GetToolCount()) 
    return NULL;
  else 
    return this->m_AllTools[toolNumber];
}


bool mitk::RandomTrackingDevice::OpenConnection()
{
  bool returnValue = true;
  //There must be no connection for a random device
  

  if (returnValue) 
  {    
    this->SetMode(Ready);
  } 
  else 
  {
    //reset everything
  }
  return returnValue;
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

std::vector<mitk::InternalTrackingTool::Pointer> mitk::RandomTrackingDevice::GetAllTools()
{
  return this->m_AllTools;
}


void mitk::RandomTrackingDevice::TrackTools()
{
  try
  {
    //Init Random
    srand(time(NULL));

    /* lock the TrackingFinishedMutex to signal that the execution rights are now transfered to the tracking thread */
    m_TrackingFinishedMutex->Lock();

    bool localStopTracking;       // Because m_StopTracking is used by two threads, access has to be guarded by a mutex. To minimize thread locking, a local copy is used here 
    this->m_StopTrackingMutex->Lock();  // update the local copy of m_StopTracking
    localStopTracking = this->m_StopTracking;
    this->m_StopTrackingMutex->Unlock();
    while ((this->GetMode() == Tracking) && (localStopTracking == false))
    {
      std::vector<mitk::InternalTrackingTool::Pointer> allTools = this->GetAllTools();
      std::vector<mitk::InternalTrackingTool::Pointer>::iterator itAllTools;
      for(itAllTools = allTools.begin(); itAllTools != allTools.end(); itAllTools++)
      {
        mitk::InternalTrackingTool::Pointer currentTool = *itAllTools;

        mitk::Point3D pos;
        mitk::Quaternion quat;

        //generate numbers between 0 and 99 for each direction
        pos[0] = (double)(rand()%100); 
        pos[1] = (double)(rand()%100);
        pos[2] = (double)(rand()%100);

        //generate numbers between 0 and 1
        quat[0] = (double)(rand()%100) / 100;
        quat[1] = (double)(rand()%100) / 100;
        quat[2] = (double)(rand()%100) / 100;
        quat[3] = (double)(rand()%100) / 100;

        currentTool->SetPosition(pos);

        currentTool->SetOrientation(quat);
        currentTool->SetTrackingError((double)(rand()%100) / 100);
        currentTool->SetDataValid(true);
      }

      //there should be a short pause perhaps the multithreader can do this?
      itksys::SystemTools::Delay(m_RefreshRate);
      /* Update the local copy of m_StopTracking */
      this->m_StopTrackingMutex->Lock();  
      localStopTracking = m_StopTracking;
      this->m_StopTrackingMutex->Unlock();
    }
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
  RandomTrackingDevice *trackingDevice = (RandomTrackingDevice*)pInfo->UserData;

  if (trackingDevice != NULL) 
    trackingDevice->TrackTools();

  return ITK_THREAD_RETURN_VALUE;
}
