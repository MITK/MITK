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
#include "mitkOptitrackTrackingDevice.h"

namespace mitk
{

//=======================================================
// Constructor
//=======================================================
OptitrackTrackingDevice::OptitrackTrackingDevice()
    : mitk::TrackingDevice(),
      m_initialized(false)
{
  // Set the MultiThread and Mutex
    this->m_MultiThreader = itk::MultiThreader::New();
    this->m_ToolsMutex = itk::FastMutexLock::New();

    //Set the mitk device information
    mitk::TrackingDeviceData currentDevice;
    //currentDevice.Line = (mitk::TrackingDeviceType)Optitrack;
    currentDevice.Model = "Optitrack";
    SetData(currentDevice);

  //Clear List of tools
    this->m_AllTools.clear();
}

//=======================================================
// Destructor
//=======================================================
OptitrackTrackingDevice::~OptitrackTrackingDevice()
{
  MITK_DEBUG << "Deleting OptitrackTrackingDevice";
  int result;

  // If device is in Tracking mode, stop the Tracking firts
    if (this->GetState() == mitk::TrackingDevice::Tracking)
    {
    MITK_DEBUG << "OptitrackTrackingDevice in Tracking State -> Stopping Tracking";

        result = this->StopTracking();

    if(result == NPRESULT_SUCCESS){
      MITK_DEBUG << "OptitrackTrackingDevice stopped";
    }
    else{
      MITK_DEBUG << "Error during Stopping";
      mitkThrowException(mitk::IGTException) << GetOptitrackErrorMessage(result);
    }
    }

  // If device is Ready, Close the connection to device and release the memory
    if (this->GetState() == mitk::TrackingDevice::Ready)
    {
    MITK_DEBUG << "OptitrackTrackingDevice in Ready State ->  Closing the Connection";

        result = this->CloseConnection();

    if(result == NPRESULT_SUCCESS){
      MITK_DEBUG << "OptitrackTrackingDevice Connection closed";
    }
    else{
      MITK_DEBUG << "Error during Closing Connection";
      mitkThrowException(mitk::IGTException) << GetOptitrackErrorMessage(result);
    }
    }

  int resultCleanUp = TT_FinalCleanup();
  if(resultCleanUp == NPRESULT_SUCCESS){
    MITK_DEBUG << "FinalCleanup success";

  }
  else
  {
    MITK_DEBUG << "FinalCleanUp failed";
    mitkThrowException(mitk::IGTException) << GetOptitrackErrorMessage(resultCleanUp);
  }



  // Set the device off
    m_initialized = false;
  // Change State to Setup
    this->SetState(mitk::TrackingDevice::Setup);
  MITK_DEBUG <<"OptitrackTrackingDevice deleted successfully";

}

//=======================================================
// OpenConnection
//=======================================================
bool OptitrackTrackingDevice::OpenConnection()
{
  // Not initialize the system twice.
    if(!m_initialized)
    {
    MITK_DEBUG << "Initialize Optitrack Tracking System";
        if(this->InitializeCameras() && // Initialize the Tracking System
                this->LoadCalibration() ) // Load a Calibration File to the System
        {
            m_initialized = true; // Set the initialized variable to true
            this->SetState(mitk::TrackingDevice::Ready); // Set state to Ready for Tracking
            MITK_DEBUG << "Device initialization successfull. Device state is in ready state";
        }
        else
        {
            this->SetState(mitk::TrackingDevice::Setup); // Set the State to Setup
      mitkThrowException(mitk::IGTException) << "Device initialization failed. Device is still in setup state";
        }
    }

    return m_initialized;
}

//=======================================================
// InitializeCameras
//=======================================================
bool OptitrackTrackingDevice::InitializeCameras()
{
    MITK_DEBUG << "Initialize Cameras";
  int result;

    for(unsigned int i=OPTITRACK_ATTEMPTS; i>0; i--)
    {

    result = TT_Initialize(); // Initialize the cameras

        if(result == NPRESULT_SUCCESS)
        {
            MITK_DEBUG << "Initialization Succeed";
            return true;
        }
        else{
            MITK_DEBUG << GetOptitrackErrorMessage(result);
      MITK_DEBUG << "Trying again...";
    }
    }

  // If not succeed after OPTITRACK_ATTEMPTS times launch exception
  mitkThrowException(mitk::IGTException) << "Optitrack Tracking System cannot be initialized \n" << GetOptitrackErrorMessage(result);
    return false;
}

//=======================================================
// LoadCalibration
//=======================================================
bool OptitrackTrackingDevice::LoadCalibration()
{
  MITK_DEBUG << "Loading System Calibration";

  int resultUpdate, resultLoadCalibration;

  // Check the file path
  if(this->m_calibrationPath.empty()){
    MITK_DEBUG << "Calibration Path is empty";
    mitkThrowException(mitk::IGTException) << "Calibration Path is empty";
    return false;
  }

    // Once the system is ready and Initialized , a calibration file is loaded.
    if(this->m_initialized)
    {

    for(unsigned int i=OPTITRACK_ATTEMPTS; i>0; i--)
      {
        resultUpdate = TT_Update(); // Get Update for the Optitrack API
        if(resultUpdate == NPRESULT_SUCCESS)
        {

          MITK_DEBUG << "Update Succeed";
          resultLoadCalibration = TT_LoadCalibration(this->m_calibrationPath.c_str());

          if(resultLoadCalibration != NPRESULT_SUCCESS)
      {
        mitkThrowException(mitk::IGTException) << mitk::GetOptitrackErrorMessage(resultLoadCalibration);
      return false;
      }
          else
          {
            MITK_DEBUG << "Calibration file has been loaded successfully";
            return true;
          }

        }
        else
        {

          MITK_DEBUG << GetOptitrackErrorMessage(resultUpdate);
          MITK_DEBUG << "Trying again...";
        }

      }

    MITK_DEBUG << "System cannot load a calibration file";
    mitkThrowException(mitk::IGTException) << "System cannot load a calibration file";

    }
    else
    {
        MITK_DEBUG << "System is not ready for load a calibration file because it has not been initialized yet";
    mitkThrowException(mitk::IGTException) << "Calibration Path is empty";
        return false;
    }

  // Never reach this point
    return false;
}

//=======================================================
// CloseConnection
//=======================================================
bool OptitrackTrackingDevice::CloseConnection()
{
  MITK_DEBUG << "CloseConnection";

  bool resultStop, resultShutdown;

  if(m_initialized) // Close connection if the System was initialized first
  {
    if(this->GetState() == mitk::TrackingDevice::Tracking){
      MITK_DEBUG << "Device state: Tracking -> Stoping the Tracking";
      resultStop = this->StopTracking(); //Stop tracking on close
    }

    for(unsigned int i=OPTITRACK_ATTEMPTS; i>0; i--)
      {
        resultShutdown = TT_Shutdown();
        if(resultShutdown == NPRESULT_SUCCESS){
          MITK_DEBUG << "System has been Shutdown Correctly";
          return true;
        }
        else
        {
          MITK_DEBUG << "System cannot ShutDown now. Trying again...";
        }
    }

    MITK_DEBUG << "System cannot ShutDown now";
  mitkThrowException(mitk::IGTException) << mitk::GetOptitrackErrorMessage(resultShutdown);
  }
  else
  {
    MITK_DEBUG << "System has not been initialized. Close connection cannot be done";
    mitkThrowException(mitk::IGTException) << "System has not been initialized. Close connection cannot be done";
        return false;
  }

  return false;

}

//=======================================================
// StartTracking
//=======================================================
bool OptitrackTrackingDevice::StartTracking()
{
  MITK_DEBUG << "StartTracking";
  bool resultIsTrackableTracked;


    if (this->GetState() != mitk::TrackingDevice::Ready)
  {
    MITK_DEBUG << "System is not in State Ready -> Cannot StartTracking";
    mitkThrowException(mitk::IGTException) << "System is not in State Ready -> Cannot StartTracking";
        return false;
  }

    this->SetState(mitk::TrackingDevice::Tracking);

    // Change the m_StopTracking Variable to false
    this->m_StopTrackingMutex->Lock();
    this->m_StopTracking = false;
    this->m_StopTrackingMutex->Unlock();

    m_TrackingFinishedMutex->Unlock(); // transfer the execution rights to tracking thread

  /******************************************************************************
  ###############################################################################
  TODO: check the timestamp from the Optitrack API
  ###############################################################################
  ******************************************************************************/
    mitk::IGTTimeStamp::GetInstance()->Start(this);

    // Launch multiThreader using the Function ThreadStartTracking that executes the TrackTools() method
    m_ThreadID = m_MultiThreader->SpawnThread(this->ThreadStartTracking, this);    // start a new thread that executes the TrackTools() method

  // Information for the user
  if(GetToolCount() == 0) MITK_INFO << "No tools are defined";

  for ( unsigned int i = 0; i < GetToolCount(); ++i)  // use mutexed methods to access tool container
    {
      resultIsTrackableTracked = TT_IsTrackableTracked(i);
      if(resultIsTrackableTracked)
      {
        MITK_DEBUG << "Trackable " << i << " is inside the Tracking Volume and it is Tracked";
      }
      else
      {
        MITK_INFO << "Trackable " << i << " is not been tracked. Check if it is inside the  Tracking volume";
      }

    }


    return true;
}

//=======================================================
// StopTracking
//=======================================================
bool OptitrackTrackingDevice::StopTracking()
{
  MITK_DEBUG << "StopTracking";
    if (this->GetState() == mitk::TrackingDevice::Tracking) // Only if the object is in the correct state
    {
        //Change the StopTracking value
        m_StopTrackingMutex->Lock();  // m_StopTracking is used by two threads, so we have to ensure correct thread handling
        m_StopTrackingMutex->Unlock();
        this->SetState(mitk::TrackingDevice::Ready);
    }
  else{
    MITK_DEBUG << "System is not in State Tracking -> Cannot StopTracking";
    mitkThrowException(mitk::IGTException) << "System is not in State Tracking -> Cannot StopTracking";
    return false;
  }
  /******************************************************************************
  ###############################################################################
  TODO: check the timestamp from the Optitrack API
  ###############################################################################
  ******************************************************************************/
    mitk::IGTTimeStamp::GetInstance()->Stop(this);

    m_TrackingFinishedMutex->Lock();
    return true;
}

//=======================================================
// ThreadStartTracking
//=======================================================
ITK_THREAD_RETURN_TYPE OptitrackTrackingDevice::ThreadStartTracking(void* pInfoStruct)
{
  MITK_DEBUG << "ThreadStartTracking";
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
    OptitrackTrackingDevice *trackingDevice = static_cast<OptitrackTrackingDevice*>(pInfo->UserData);

    if (trackingDevice != NULL)
  {
        // Call the TrackTools function in this thread
        trackingDevice->TrackTools();
  }
  else
  {
    mitkThrowException(mitk::IGTException) << "In ThreadStartTracking(): trackingDevice is NULL";
  }

    trackingDevice->m_ThreadID = -1; // reset thread ID because we end the thread here
    return ITK_THREAD_RETURN_VALUE;
}

//=======================================================
// GetOptitrackTool
//=======================================================
mitk::OptitrackTrackingTool* OptitrackTrackingDevice::GetOptitrackTool(unsigned int toolNumber) const
{
  //MITK_DEBUG << "ThreadStartTracking";
    OptitrackTrackingTool* t = nullptr;

    MutexLockHolder toolsMutexLockHolder(*m_ToolsMutex); // lock and unlock the mutex
    if(toolNumber < m_AllTools.size())
  {
        t = m_AllTools.at(toolNumber);

  }
  else
  {
    mitkThrowException(mitk::IGTException) << "The tool numbered " << toolNumber << " does not exist";
  }
  return t;
}

//=======================================================
// GetToolCount
//=======================================================
unsigned int OptitrackTrackingDevice::GetToolCount() const
{
  //MITK_DEBUG << "GetToolCount";
    MutexLockHolder lock(*m_ToolsMutex); // lock and unlock the mutex
    return (unsigned int)(this->m_AllTools.size());
}

//=======================================================
// TrackTools
//=======================================================
void OptitrackTrackingDevice::TrackTools()
{
    //MITK_DEBUG << "TrackTools";
    Point3D position;
    ScalarType t = 0.0;

    try
    {
        bool localStopTracking;       // Because m_StopTracking is used by two threads, access has to be guarded by a mutex. To minimize thread locking, a local copy is used here
        this->m_StopTrackingMutex->Lock();  // update the local copy of m_StopTracking
        localStopTracking = this->m_StopTracking;

        /* lock the TrackingFinishedMutex to signal that the execution rights are now transfered to the tracking thread */
        if (!localStopTracking)
        {
            m_TrackingFinishedMutex->Lock();
        }
        this->m_StopTrackingMutex->Unlock();

        while ((this->GetState() == mitk::TrackingDevice::Tracking) && (localStopTracking == false))
        {
            // For each Tracked Tool update the position and orientation
            for ( unsigned int i = 0; i < GetToolCount(); ++i)  // use mutexed methods to access tool container
            {
                OptitrackTrackingTool* currentTool = this->GetOptitrackTool(i);
                if(currentTool != nullptr)
                {
                    currentTool->updateTool();
        //MITK_DEBUG << "Tool number " << i << " updated position";
                }
                else
          MITK_DEBUG << "Get data from tool number " << i << " failed";
                    mitkThrowException(mitk::IGTException) << "Get data from tool number " << i << " failed";
            }

            /* Update the local copy of m_StopTracking */
            this->m_StopTrackingMutex->Lock();
            localStopTracking = m_StopTracking;
            this->m_StopTrackingMutex->Unlock();

      Sleep(OPTITRACK_FRAME_RATE);
        } // tracking ends if we pass this line

        m_TrackingFinishedMutex->Unlock(); // transfer control back to main thread
    }
    catch(...)
    {
        m_TrackingFinishedMutex->Unlock();
        this->StopTracking();
        mitkThrowException(mitk::IGTException) << "Error while trying to track tools. Thread stopped.";
    }
}

//=======================================================
// SetCameraParams
//=======================================================
bool OptitrackTrackingDevice::SetCameraParams(int exposure, int threshold , int intensity, int videoType )
{
    MITK_DEBUG << "SetCameraParams";

    if(this->m_initialized)
    {
        int num_cams, resultUpdate, resultSetCameraSettings;

        for(unsigned int i=OPTITRACK_ATTEMPTS; i>0; i--)
        {
            resultUpdate = TT_Update(); // Get Update for the Optitrack API
      if(resultUpdate == NPRESULT_SUCCESS)
            {
        MITK_DEBUG << "Update Succeed";
                num_cams = TT_CameraCount();
                i = 0;
            }
      else
      {
        MITK_DEBUG << GetOptitrackErrorMessage(resultUpdate);
        MITK_DEBUG << "Trying again...";
      }
        }

        std::string dialog;
        char* buffer = new char[20];

        for(int cam=0; cam<num_cams; cam++) // for all connected cameras
        {
            for(unsigned int i=OPTITRACK_ATTEMPTS; i>0; i--)
            {
        resultUpdate = TT_Update(); // Get Update for the Optitrack API
        if(resultUpdate == NPRESULT_SUCCESS)
        {
          MITK_DEBUG << "Update Succeed for camera number " << i;
          resultSetCameraSettings = TT_SetCameraSettings(cam,videoType,exposure,threshold,intensity);
          if(resultSetCameraSettings == NPRESULT_SUCCESS)
          {
            MITK_DEBUG << "Camera # "<<cam<< " params are set";
            i = 0;
          }
          else
          {
            MITK_DEBUG << GetOptitrackErrorMessage(resultSetCameraSettings);
            MITK_DEBUG << "SetCameraSettings Trying again...";
          }


        }
        else
        {
          MITK_DEBUG << GetOptitrackErrorMessage(resultUpdate);
          MITK_DEBUG << "Update: Trying again...";
        }
      }
    }

    }
    else
    {
        MITK_INFO << "";
    MITK_DEBUG << "System is not Initialized -> System is not ready to perform the Camera Parameters Setting";
    mitkThrowException(mitk::IGTException) << "System is not Initialized -> System is not ready to perform the Camera Parameters Setting";
    return false;
    }
  return true;

}

//=======================================================
// GetTool
//=======================================================
mitk::TrackingTool* OptitrackTrackingDevice::GetTool(unsigned int toolNumber) const
{
     return static_cast<mitk::TrackingTool*>(GetOptitrackTool(toolNumber));
}

//=======================================================
// AddToolByFileName
//=======================================================
bool OptitrackTrackingDevice::AddToolByDefinitionFile(std::string fileName)
{
  bool resultSetToolByFileName;
  if(m_initialized)
  {
    OptitrackTrackingTool::Pointer t = OptitrackTrackingTool::New();
        resultSetToolByFileName= t->SetToolByFileName(fileName);
    if(resultSetToolByFileName)
    {
      this->m_AllTools.push_back(t);
      MITK_DEBUG << "Added tool "<<t->GetToolName()<< ". Tool vector size is now: "<<m_AllTools.size();
      return true;
    }
    else
    {
      MITK_DEBUG << "Tool could not be added";
      mitkThrowException(mitk::IGTException) << "Tool could not be added";
      return false;
    }

  }
  else
  {
    MITK_DEBUG << "System is not Initialized -> Cannot Add tools";
    mitkThrowException(mitk::IGTException) << "System is not Initialized -> Cannot Add tools";
        return false;
  }

}

} // end namespace
