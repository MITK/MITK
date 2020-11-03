/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include "mitkOptitrackTrackingDevice.h"
#include <mitkOptitrackErrorMessages.h>

#ifdef MITK_USE_OPTITRACK_TRACKER



/**
* \brief API library header for Optitrack Systems
*/
#include <NPTrackingTools.h>

//=======================================================
// Static method: IsDeviceInstalled
//=======================================================
bool mitk::OptitrackTrackingDevice::IsDeviceInstalled()
{
  return true;
}

//=======================================================
// Constructor
//=======================================================
mitk::OptitrackTrackingDevice::OptitrackTrackingDevice()
    : mitk::TrackingDevice(),
      m_initialized(false)
{
  // Set the MultiThread and Mutex
  this->m_MultiThreader = itk::MultiThreader::New();
  this->m_ToolsMutex = itk::FastMutexLock::New();

  //Set the mitk device information
  SetData(mitk::DeviceDataNPOptitrack);

  //Clear List of tools
  this->m_AllTools.clear();
}

//=======================================================
// Destructor
//=======================================================
mitk::OptitrackTrackingDevice::~OptitrackTrackingDevice()
{
  MITK_DEBUG << "Deleting OptitrackTrackingDevice";
  int result;

  // If device is in Tracking mode, stop the Tracking firts
  if (this->GetState() == mitk::TrackingDevice::Tracking)
  {
    MITK_DEBUG << "OptitrackTrackingDevice in Tracking State -> Stopping Tracking";
    result = this->StopTracking();

    if(result == NPRESULT_SUCCESS){
      MITK_INFO << "OptitrackTrackingDevice Stopped";
    }
    else
  {
      MITK_INFO << "Error during Stopping";
      mitkThrowException(mitk::IGTException) << mitk::OptitrackErrorMessages::GetOptitrackErrorMessage(result);
    }
  }

  // If device is Ready, Close the connection to device and release the memory
  if (this->GetState() == mitk::TrackingDevice::Ready)
  {
    MITK_DEBUG << "OptitrackTrackingDevice in Ready State ->  Closing the Connection";
    result = this->CloseConnection();

    if(result)
    {
      MITK_INFO << "OptitrackTrackingDevice Connection closed";
    }
    else
    {
      MITK_DEBUG << "Error during Closing Connection";
      mitkThrowException(mitk::IGTException) << mitk::OptitrackErrorMessages::GetOptitrackErrorMessage(result);
    }
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
bool mitk::OptitrackTrackingDevice::OpenConnection()
{
  // Not initialize the system twice.
  if(!m_initialized)
  {
    MITK_DEBUG << "Initialize Optitrack Tracking System";

  if( this->InitializeCameras() )
  {
    m_initialized = true; // Set the initialized variable to true
    this->SetState(mitk::TrackingDevice::Ready);
      if(this->m_calibrationPath.empty()){
      MITK_INFO << "Numer of connected cameras = " << TT_CameraCount();
      MITK_WARN << "Attention: No calibration File defined !!";
      return m_initialized;
    }
    else
    {
      this->LoadCalibration();
    }
  }
  else
  {
    m_initialized = false; // Set the initialized variable to false
    this->SetState(mitk::TrackingDevice::Setup); // Set the State to Setup
    MITK_INFO << "Device initialization failed. Device is still in setup state";
    mitkThrowException(mitk::IGTException) << "Device initialization failed. Device is still in setup state";
  }
  }
  //this->LoadCalibration();
  return m_initialized;
}

//=======================================================
// InitializeCameras
//=======================================================
bool mitk::OptitrackTrackingDevice::InitializeCameras()
{
  MITK_DEBUG << "Initialize Optitrack";
  int result;

  result = TT_Initialize(); // Initialize the cameras

  if(result == NPRESULT_SUCCESS)
  {
    MITK_DEBUG << "Optitrack Initialization Succeed";
    return true;
  }
  else
  {
    MITK_DEBUG << mitk::OptitrackErrorMessages::GetOptitrackErrorMessage(result);
  // If not succeed after OPTITRACK_ATTEMPTS times launch exception
  MITK_INFO << "Optitrack Tracking System cannot be initialized \n" << mitk::OptitrackErrorMessages::GetOptitrackErrorMessage(result);
  mitkThrowException(mitk::IGTException) << "Optitrack Tracking System cannot be initialized \n" << mitk::OptitrackErrorMessages::GetOptitrackErrorMessage(result);
  return false;
  }
}

//=======================================================
// LoadCalibration
//=======================================================
bool mitk::OptitrackTrackingDevice::LoadCalibration()
{
  MITK_DEBUG << "Loading System Calibration";
  int resultLoadCalibration;

  // Check the file path
  if(this->m_calibrationPath.empty()){
    MITK_INFO << "Calibration Path is empty";
    mitkThrowException(mitk::IGTException) << "Calibration Path is empty";
    return false;
  }

  // Once the system is ready and Initialized , a calibration file is loaded.
  if(this->m_initialized)
  {

    for( int i=OPTITRACK_ATTEMPTS; i>0; i--)
    {
      resultLoadCalibration = TT_LoadCalibration(this->m_calibrationPath.c_str());

      if(resultLoadCalibration != NPRESULT_SUCCESS)
      {
        MITK_DEBUG << mitk::OptitrackErrorMessages::GetOptitrackErrorMessage(resultLoadCalibration);
        MITK_DEBUG << "Trying again...";
      }
      else
      {
        MITK_DEBUG << "Calibration file has been loaded successfully";
        return true;
      }

    }

    MITK_INFO << "System cannot load a calibration file";
    mitkThrowException(mitk::IGTException) << "System cannot load a calibration file";

  }
  else
  {
    MITK_INFO << "System is not ready for load a calibration file because it has not been initialized yet";
    mitkThrowException(mitk::IGTException) << "System is not ready for load a calibration file because it has not been initialized yet";
        return false;
  }

  // Never reach this point
  return false;
}

//=======================================================
// SetCalibrationPath
//=======================================================
void mitk::OptitrackTrackingDevice::SetCalibrationPath(std::string calibrationPath){

  MITK_DEBUG << "SetcalibrationPath";
  MITK_DEBUG << calibrationPath;

  // Check the file path
  if(calibrationPath.empty())
  {
    MITK_INFO << "Calibration Path is empty";
    //mitkThrowException(mitk::IGTException) << "Calibration Path is empty";
    return;
  }

  this->m_calibrationPath = calibrationPath;
  MITK_INFO << "Calibration Path has been updated to: " << this->m_calibrationPath;
  return;
}

//=======================================================
// CloseConnection
//=======================================================
bool mitk::OptitrackTrackingDevice::CloseConnection()
{
  MITK_DEBUG << "CloseConnection";
  int resultStop, resultShutdown;

  if(m_initialized) // Close connection if the System was initialized first
  {
    if(this->GetState() == mitk::TrackingDevice::Tracking)
  {
      MITK_DEBUG << "Device state: Tracking -> Stoping the Tracking";
      resultStop = this->StopTracking(); //Stop tracking on close
    }

    this->SetState(mitk::OptitrackTrackingDevice::Setup);

    for( int i=OPTITRACK_ATTEMPTS; i>0; i--)
    {

      TT_ClearTrackableList();
      resultShutdown = TT_Shutdown();

      if(resultShutdown == NPRESULT_SUCCESS)
      {
        MITK_DEBUG << "System has been Shutdown Correctly";
        Sleep(2000);
        return true;
      }
      else
      {
        MITK_DEBUG << "System cannot ShutDown now. Trying again...";
      }
    }

    MITK_INFO << "System cannot ShutDown now";
    mitkThrowException(mitk::IGTException) << mitk::OptitrackErrorMessages::GetOptitrackErrorMessage(resultShutdown);
    return false;
 }
 else
 {
   MITK_INFO << "System has not been initialized. Close connection cannot be done";
   mitkThrowException(mitk::IGTException) << "System has not been initialized. Close connection cannot be done";
   return false;
 }

 return false;
}

//=======================================================
// StartTracking
//=======================================================
bool mitk::OptitrackTrackingDevice::StartTracking()
{
  MITK_DEBUG << "StartTracking";
  bool resultIsTrackableTracked;

  if (this->GetState() != mitk::TrackingDevice::Ready)
  {
    MITK_INFO << "System is not in State Ready -> Cannot StartTracking";
    mitkThrowException(mitk::IGTException) << "System is not in State Ready -> Cannot StartTracking";
    return false;
  }

  this->SetState(mitk::TrackingDevice::Tracking);

  // Change the m_StopTracking Variable to false
  this->m_StopTrackingMutex->Lock();
  this->m_StopTracking = false;
  this->m_StopTrackingMutex->Unlock();

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

  for (  int i = 0; i < GetToolCount(); ++i)  // use mutexed methods to access tool container
  {
    resultIsTrackableTracked = TT_IsTrackableTracked(i);
    if(resultIsTrackableTracked)
    {
      MITK_DEBUG << "Trackable " << i << " is inside the Tracking Volume and it is Tracked";
    }
    else
    {
      MITK_DEBUG << "Trackable " << i << " is not been tracked. Check if it is inside the  Tracking volume";
    }

  }

  return true;
}

//=======================================================
// StopTracking
//=======================================================
bool mitk::OptitrackTrackingDevice::StopTracking()
{
  MITK_DEBUG << "StopTracking";

  if (this->GetState() == mitk::TrackingDevice::Tracking) // Only if the object is in the correct state
  {
    //Change the StopTracking value
    m_StopTrackingMutex->Lock();  // m_StopTracking is used by two threads, so we have to ensure correct thread handling
    m_StopTrackingMutex->Unlock();
    this->SetState(mitk::TrackingDevice::Ready);
  }
  else
  {
    m_TrackingFinishedMutex->Unlock();
    MITK_INFO << "System is not in State Tracking -> Cannot StopTracking";
    mitkThrowException(mitk::IGTException) << "System is not in State Tracking -> Cannot StopTracking";
    return false;
  }

  /******************************************************************************
  ###############################################################################
  TODO: check the timestamp from the Optitrack API
  ###############################################################################
  ******************************************************************************/
  mitk::IGTTimeStamp::GetInstance()->Stop(this);

  m_TrackingFinishedMutex->Unlock();
  return true;
}

//=======================================================
// ThreadStartTracking
//=======================================================
ITK_THREAD_RETURN_TYPE mitk::OptitrackTrackingDevice::ThreadStartTracking(void* pInfoStruct)
{
  MITK_DEBUG << "ThreadStartTracking";

  /* extract this pointer from Thread Info structure */
  struct itk::MultiThreader::ThreadInfoStruct * pInfo = (struct itk::MultiThreader::ThreadInfoStruct*)pInfoStruct;

  if (pInfo == nullptr)
  {
    return ITK_THREAD_RETURN_VALUE;
  }

  if (pInfo->UserData == nullptr)
  {
    return ITK_THREAD_RETURN_VALUE;
  }

  OptitrackTrackingDevice *trackingDevice = static_cast<OptitrackTrackingDevice*>(pInfo->UserData);

  if (trackingDevice != nullptr)
  {
    // Call the TrackTools function in this thread
    trackingDevice->TrackTools();
  }
  else
  {
    mitkThrowException(mitk::IGTException) << "In ThreadStartTracking(): trackingDevice is nullptr";
  }

  trackingDevice->m_ThreadID = -1; // reset thread ID because we end the thread here
  return ITK_THREAD_RETURN_VALUE;
}

//=======================================================
// GetOptitrackTool
//=======================================================
mitk::OptitrackTrackingTool* mitk::OptitrackTrackingDevice::GetOptitrackTool( unsigned int toolNumber) const
{
  MITK_DEBUG << "ThreadStartTracking";
  OptitrackTrackingTool* t = nullptr;

  MutexLockHolder toolsMutexLockHolder(*m_ToolsMutex); // lock and unlock the mutex
  if(toolNumber < m_AllTools.size())
  {
    t = m_AllTools.at(toolNumber);
  }
  else
  {
  MITK_INFO << "The tool numbered " << toolNumber << " does not exist";
    mitkThrowException(mitk::IGTException) << "The tool numbered " << toolNumber << " does not exist";
  }
  return t;
}

//=======================================================
// GetToolCount
//=======================================================
 unsigned int mitk::OptitrackTrackingDevice::GetToolCount() const
{
  MITK_DEBUG << "GetToolCount";
  MutexLockHolder lock(*m_ToolsMutex); // lock and unlock the mutex
  return ( int)(this->m_AllTools.size());
}

//=======================================================
// TrackTools
//=======================================================
void mitk::OptitrackTrackingDevice::TrackTools()
{
  MITK_DEBUG << "TrackTools";

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
      for (  int i = 0; i < GetToolCount(); ++i)  // use mutexed methods to access tool container
      {
        OptitrackTrackingTool* currentTool = this->GetOptitrackTool(i);
        if(currentTool != nullptr)
        {
          currentTool->updateTool();
          MITK_DEBUG << "Tool number " << i << " updated position";
        }
        else
        {
          MITK_DEBUG << "Get data from tool number " << i << " failed";
          mitkThrowException(mitk::IGTException) << "Get data from tool number " << i << " failed";
        }
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
bool mitk::OptitrackTrackingDevice::SetCameraParams(int exposure, int threshold , int intensity, int videoType )
{
  MITK_DEBUG << "SetCameraParams";

  if(this->m_initialized)
  {
    int num_cams = 0;
    int resultUpdate;
    bool resultSetCameraSettings;

    for( int i=OPTITRACK_ATTEMPTS; i>0; i--)
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
        MITK_DEBUG << mitk::OptitrackErrorMessages::GetOptitrackErrorMessage(resultUpdate);
        MITK_DEBUG << "Trying again...";
        Sleep(30);
      }
    }

    // If no cameras are connected
    if(num_cams == 0)
    {
      MITK_DEBUG << "No cameras are connected to the device";
      return false;
      mitkThrowException(mitk::IGTException) << "No cameras are connected to the device";
    }

    for(int cam = 0; cam < num_cams; cam++) // for all connected cameras
    {
      for( int i=OPTITRACK_ATTEMPTS; i>0; i--)
      {
        resultUpdate = TT_Update(); // Get Update for the Optitrack API

    if(resultUpdate == NPRESULT_SUCCESS)
        {
          MITK_DEBUG << "Update Succeed for camera number " << cam;
          resultSetCameraSettings = TT_SetCameraSettings(cam,videoType,exposure,threshold,intensity);

          if(resultSetCameraSettings)
          {
            MITK_INFO << "Camera # "<<cam<< " params are set";
            i = 0; // End attempts for camera #cam
          }
          else
          {
            MITK_DEBUG << mitk::OptitrackErrorMessages::GetOptitrackErrorMessage(resultSetCameraSettings);
            if(i == 1)
            mitkThrowException(mitk::IGTException) << "Camera number " << cam << " failed during setting the params. Error: " << mitk::OptitrackErrorMessages::GetOptitrackErrorMessage(resultSetCameraSettings);
          }
        }
        else
        {
          MITK_DEBUG << mitk::OptitrackErrorMessages::GetOptitrackErrorMessage(resultUpdate);
          MITK_DEBUG << "Update: Trying again...";
        }
      }
    }
  }
  else
  {
    MITK_INFO << "System is not Initialized -> System is not ready to perform the Camera Parameters Setting";
    mitkThrowException(mitk::IGTException) << "System is not Initialized -> System is not ready to perform the Camera Parameters Setting";
    return false;
  }
  return true;
}

//=======================================================
// GetTool
//=======================================================
mitk::TrackingTool* mitk::OptitrackTrackingDevice::GetTool(unsigned int toolNumber) const
{
  return static_cast<mitk::TrackingTool*>(GetOptitrackTool(toolNumber));
}

//=======================================================
// AddToolByFileName
//=======================================================
bool mitk::OptitrackTrackingDevice::AddToolByDefinitionFile(std::string fileName)
{
  bool resultSetToolByFileName;
  if(m_initialized)
  {
    OptitrackTrackingTool::Pointer t = OptitrackTrackingTool::New();
    resultSetToolByFileName= t->SetToolByFileName(fileName);

    if(resultSetToolByFileName)
    {
      this->m_AllTools.push_back(t);
      MITK_INFO << "Added tool "<<t->GetToolName()<< ". Tool vector size is now: "<<m_AllTools.size();
      return true;
    }
    else
    {
      MITK_INFO << "Tool could not be added";
      mitkThrowException(mitk::IGTException) << "Tool could not be added";
      return false;
  }
  }
  else
  {
    MITK_INFO << "System is not Initialized -> Cannot Add tools";
    mitkThrowException(mitk::IGTException) << "System is not Initialized -> Cannot Add tools";
    return false;
  }
}


//=======================================================
// IF Optitrack is not installed set functions to warnings
//=======================================================

#else

//=======================================================
// Static method: IsDeviceInstalled
//=======================================================
bool mitk::OptitrackTrackingDevice::IsDeviceInstalled()
{
  return false;
}

//=======================================================
// Constructor
//=======================================================
mitk::OptitrackTrackingDevice::OptitrackTrackingDevice()
    : mitk::TrackingDevice(),
      m_initialized(false)
{
  MITK_WARN("IGT") << "Error: " << mitk::OptitrackErrorMessages::GetOptitrackErrorMessage(100);
}

//=======================================================
// Destructor
//=======================================================
mitk::OptitrackTrackingDevice::~OptitrackTrackingDevice()
{
 MITK_WARN("IGT") << "Error: " << mitk::OptitrackErrorMessages::GetOptitrackErrorMessage(100);
}

//=======================================================
// OpenConnection
//=======================================================
bool mitk::OptitrackTrackingDevice::OpenConnection()
{
  MITK_WARN("IGT") << "Error: " << mitk::OptitrackErrorMessages::GetOptitrackErrorMessage(100);
  return false;
}

//=======================================================
// InitializeCameras
//=======================================================
bool mitk::OptitrackTrackingDevice::InitializeCameras()
{
  MITK_WARN("IGT") << "Error: " << mitk::OptitrackErrorMessages::GetOptitrackErrorMessage(100);
  return false;
}

//=======================================================
// LoadCalibration
//=======================================================
bool mitk::OptitrackTrackingDevice::LoadCalibration()
{
  MITK_WARN("IGT") << "Error: " << mitk::OptitrackErrorMessages::GetOptitrackErrorMessage(100);
  return false;
}

//=======================================================
// SetcalibrationPath
//=======================================================
void mitk::OptitrackTrackingDevice::SetCalibrationPath(std::string /*calibrationPath*/)
{
  MITK_WARN("IGT") << "Error: " << mitk::OptitrackErrorMessages::GetOptitrackErrorMessage(100);
}

//=======================================================
// CloseConnection
//=======================================================
bool mitk::OptitrackTrackingDevice::CloseConnection()
{
  MITK_WARN("IGT") << "Error: " << mitk::OptitrackErrorMessages::GetOptitrackErrorMessage(100);
  return false;
}

//=======================================================
// StartTracking
//=======================================================
bool mitk::OptitrackTrackingDevice::StartTracking()
{
  MITK_WARN("IGT") << "Error: " << mitk::OptitrackErrorMessages::GetOptitrackErrorMessage(100);
  return false;
}

//=======================================================
// StopTracking
//=======================================================
bool mitk::OptitrackTrackingDevice::StopTracking()
{
  MITK_WARN("IGT") << "Error: " << mitk::OptitrackErrorMessages::GetOptitrackErrorMessage(100);
  return false;
}

//=======================================================
// ThreadStartTracking
//=======================================================
ITK_THREAD_RETURN_TYPE mitk::OptitrackTrackingDevice::ThreadStartTracking(void*)
{
  MITK_WARN("IGT") << "Error: " << mitk::OptitrackErrorMessages::GetOptitrackErrorMessage(100);
  return 0;
}

//=======================================================
// GetOptitrackTool
//=======================================================
mitk::OptitrackTrackingTool* mitk::OptitrackTrackingDevice::GetOptitrackTool(unsigned int) const
{
  MITK_WARN("IGT") << "Error: " << mitk::OptitrackErrorMessages::GetOptitrackErrorMessage(100);
  return nullptr;
}

//=======================================================
// GetToolCount
//=======================================================
 unsigned int mitk::OptitrackTrackingDevice::GetToolCount() const
{
  MITK_WARN("IGT") << "Error: " << mitk::OptitrackErrorMessages::GetOptitrackErrorMessage(100);
  return 0;
}

//=======================================================
// TrackTools
//=======================================================
void mitk::OptitrackTrackingDevice::TrackTools()
{
  MITK_WARN("IGT") << "Error: " << mitk::OptitrackErrorMessages::GetOptitrackErrorMessage(100);
}

//=======================================================
// SetCameraParams
//=======================================================
bool mitk::OptitrackTrackingDevice::SetCameraParams(int, int, int, int)
{
  MITK_WARN("IGT") << "Error: " << mitk::OptitrackErrorMessages::GetOptitrackErrorMessage(100);
  return false;
}

//=======================================================
// GetTool
//=======================================================
mitk::TrackingTool* mitk::OptitrackTrackingDevice::GetTool(unsigned int) const
{
  MITK_WARN("IGT") << "Error: " << mitk::OptitrackErrorMessages::GetOptitrackErrorMessage(100);
  return nullptr;
}

//=======================================================
// AddToolByFileName
//=======================================================
bool mitk::OptitrackTrackingDevice::AddToolByDefinitionFile(std::string)
{
  MITK_WARN("IGT") << "Error: " << mitk::OptitrackErrorMessages::GetOptitrackErrorMessage(100);
  return false;
}

#endif
