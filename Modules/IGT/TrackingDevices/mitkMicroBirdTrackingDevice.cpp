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

#include "mitkMicrobirdTrackingDevice.h"

#include <itksys/SystemTools.hxx>
#include <itkMutexLockHolder.h>

typedef itk::MutexLockHolder<itk::FastMutexLock> MutexLockHolder;

mitk::MicroBirdTrackingDevice::MicroBirdTrackingDevice() : TrackingDevice(),
m_ErrorMessage(""),
m_ThreadID(0),
m_pl(50), // 50 Hz for Europe
m_metric(true),
m_agcModeBoth(true),
m_measurementRate(68.3), // 68.3 for mid-range transmitter, 40.5 for flat transmitter
m_TransmitterConfig(nullptr),
m_SensorConfig(nullptr)
{
  // Flat transmitter needs measurement rate: 40.5
  // Mid-range transmitter needs measurement rate: 68.3;

  // Set the tracker type
  this->m_Data = mitk::DeviceDataMicroBird;

  // Clear tools vector
  m_Tools.clear();

  // Create tools vector mutex
  m_ToolsMutex = itk::FastMutexLock::New();

  // Prepare multi-threading
  m_MultiThreader = itk::MultiThreader::New();

  // Pointer to record member variable
  pRecord = &record;
}


mitk::MicroBirdTrackingDevice::~MicroBirdTrackingDevice()
{
  if (m_MultiThreader)
    m_MultiThreader->TerminateThread(m_ThreadID);
  m_MultiThreader = nullptr;
  if (m_ToolsMutex)
    m_ToolsMutex->Unlock();
  m_ToolsMutex = nullptr;

  this->StopTracking();
  this->CloseConnection();

  if (m_TransmitterConfig != nullptr)
    delete [] m_TransmitterConfig;
  if (m_SensorConfig != nullptr)
    delete [] m_SensorConfig;

  //\TODO: Do we need to clean up the pointers to PCIBird data like DOUBLE_POSITION_QUATERNION_TIME_Q_RECORD?
}


bool mitk::MicroBirdTrackingDevice::OpenConnection()
{
  /* Check whether in setup mode */
  if (this->GetState() != Setup)
  {
    this->SetErrorMessage("Can only try to open the connection if in setup mode");
    return false;
  }

  int errorCode; // Holds error code

  /* Initialize the PCIBIRD driver and DLL */
  errorCode = InitializeBIRDSystem(); // this function can take a few seconds
  if (!CompareError(errorCode, BIRD_ERROR_SUCCESS))
  {
    HandleError(errorCode);
    return false;
  }
  /// @todo : check for transmitter serial numbers here?
  // Serial numbers could be compared to known ones for some simple
  //    parameters sanity check (measurement frequency etc.)

  /* Get system configuration */
  errorCode = GetBIRDSystemConfiguration(&m_SystemConfig);
  if (!CompareError(errorCode, BIRD_ERROR_SUCCESS))
  {
    HandleError(errorCode);
    return false;
  }

  /* use metric measurements in mm */
  errorCode = SetSystemParameter(METRIC, &m_metric, sizeof(m_metric));
  if (!CompareError(errorCode, BIRD_ERROR_SUCCESS))
  {
    HandleError(errorCode);
    return false;
  }

  /* Set the measurement rate to m_measurementRate */
  if ((m_measurementRate > 30) && (m_measurementRate < 80))
  {
    errorCode = SetSystemParameter(MEASUREMENT_RATE, &m_measurementRate, sizeof(m_measurementRate));
    if (!CompareError(errorCode, BIRD_ERROR_SUCCESS))
    {
      HandleError(errorCode);
      return false;
    }
  }

  /* Set power line frequency */
  if ((m_pl >= 50) && (m_pl <= 60))
  {
    errorCode = SetSystemParameter(POWER_LINE_FREQUENCY, &m_pl, sizeof(m_pl));
    if (!CompareError(errorCode, BIRD_ERROR_SUCCESS))
    {
      HandleError(errorCode);
      return false;
    }
  }

  /* Set AGC mode */
  m_agc = m_agcModeBoth ? TRANSMITTER_AND_SENSOR_AGC : SENSOR_AGC_ONLY;
  errorCode = SetSystemParameter(AGC_MODE, &m_agc, sizeof(m_agc));
  if (!CompareError(errorCode, BIRD_ERROR_SUCCESS))
  {
    HandleError(errorCode);
    return false;
  }

  /* Get system configuration */
  errorCode = GetBIRDSystemConfiguration(&m_SystemConfig);
  if (!CompareError(errorCode, BIRD_ERROR_SUCCESS))
  {
    HandleError(errorCode);
    return false;
  }

  /* Get sensor information */
  m_SensorConfig = new SENSOR_CONFIGURATION[m_SystemConfig.numberSensors];
  for (int i = 0; i < m_SystemConfig.numberSensors; i++)
  {
    errorCode = GetSensorConfiguration(i, &(m_SensorConfig[i]));
    if (!CompareError(errorCode, BIRD_ERROR_SUCCESS))
    {
      HandleError(errorCode);
    }

    /* Initialize the quality parameter structure */
    QUALITY_PARAMETERS qualityParameters; // = { 164, 0, 32, 3072 };
    GetSensorParameter(i, QUALITY, &qualityParameters, sizeof(qualityParameters));

    /* Set data format to matrix format */
    //DATA_FORMAT_TYPE tempBuffer = DOUBLE_POSITION_MATRIX_TIME_Q;
    /* Set data format to quaternion format */
    DATA_FORMAT_TYPE tempBuffer = DOUBLE_POSITION_QUATERNION_TIME_Q;

    /* Set data format for sensor */
    DATA_FORMAT_TYPE *pTempBuffer = &tempBuffer;
    errorCode = SetSensorParameter(i, DATA_FORMAT, pTempBuffer, sizeof(tempBuffer));
    if (!CompareError(errorCode, BIRD_ERROR_SUCCESS))
    {
      HandleError(errorCode);
    }
  }

  /* Initialize tools vector */
  {
    MutexLockHolder(*m_ToolsMutex);
    for (int i = 0; i < m_SystemConfig.numberSensors; i++)
    {
      if (m_SensorConfig[i].attached)
        m_Tools.push_back(ToolType::New());

    }
  }

  /* Get transmitter configuration */
  m_TransmitterConfig = new TRANSMITTER_CONFIGURATION[m_SystemConfig.numberTransmitters];
  for (int i = 0; i < m_SystemConfig.numberTransmitters; i++)
  {
    errorCode = GetTransmitterConfiguration(i, &(m_TransmitterConfig[i]));
    if (!CompareError(errorCode, BIRD_ERROR_SUCCESS))
    {
      HandleError(errorCode);
    }
  }

  /* Switch off transmitter */
  SwitchTransmitter(true);
  SwitchTransmitter(false);

  // @todo : set up error scaling?

  /* finish  - now all tools should be added, initialized and enabled, so that tracking can be started */
  this->SetState(Ready);
  this->SetErrorMessage("");
  return true; // Return success
}


bool mitk::MicroBirdTrackingDevice::SwitchTransmitter(bool switchOn)
{
  if (switchOn)
  {
    /* Search for the first attached transmitter and turn it on */
    for (short id = 0; id < m_SystemConfig.numberTransmitters; id++)
    {
      if (m_TransmitterConfig[id].attached)
      {
        // Transmitter selection is a system function.
        // Using the SELECT_TRANSMITTER parameter we send the id of the
        // transmitter that we want to run with the SetSystemParameter() call
        int errorCode = SetSystemParameter(SELECT_TRANSMITTER, &id, sizeof(id));
        if (!CompareError(errorCode, BIRD_ERROR_SUCCESS))
        {
          HandleError(errorCode);
          return false;
        }
        else
          return true; //break; // \TODO: Stop after the first attached transmitter was turned off?
      }
    }
  }
  else
  {
    /* Transmitter selection is a system function, Note: a selector of -1 switches off the current transmitter */
    short TRANSMITTER_OFF = -1;
    int errorCode = SetSystemParameter(SELECT_TRANSMITTER, &TRANSMITTER_OFF, sizeof(TRANSMITTER_OFF));
    if (!CompareError(errorCode, BIRD_ERROR_SUCCESS))
    {
      HandleError(errorCode);
      return false;
    }
    else
      return true;
  }
  // Return success
  return true;
}


bool mitk::MicroBirdTrackingDevice::CloseConnection()
{
  SwitchTransmitter(false); // Switch off the transmitter

  int errorCode = CloseBIRDSystem(); // Close connection. This function can take a few seconds

  // Error checking
  if (!CompareError(errorCode, BIRD_ERROR_SUCCESS))
    HandleError(errorCode);

  // Delete configuration
  if (m_TransmitterConfig != nullptr)
  {
    delete [] m_TransmitterConfig;
    m_TransmitterConfig = nullptr;
  }
  if (m_SensorConfig != nullptr)
  {
    delete [] m_SensorConfig;
    m_SensorConfig = nullptr;
  }
  // Change mode and release mutex
  this->SetState(Setup);

  // Clear error message
  this->SetErrorMessage("");

  return true;
}


ITK_THREAD_RETURN_TYPE mitk::MicroBirdTrackingDevice::ThreadStartTracking(void* pInfoStruct)
{
  /* extract this pointer from Thread Info structure */
  struct itk::MultiThreader::ThreadInfoStruct * pInfo = (struct itk::MultiThreader::ThreadInfoStruct*)pInfoStruct;
  if ((pInfo == nullptr) || (pInfo->UserData == nullptr))
    return ITK_THREAD_RETURN_VALUE;

  MicroBirdTrackingDevice *trackingDevice = (MicroBirdTrackingDevice*)pInfo->UserData;
  if (trackingDevice != nullptr)
    trackingDevice->TrackTools();             // call TrackTools() from the original object

  return ITK_THREAD_RETURN_VALUE;
}


bool mitk::MicroBirdTrackingDevice::StopTracking()
{
  TrackingDevice::StopTracking(); // Call superclass method

  SwitchTransmitter(false); // Switch off transmitter
  InvalidateAll(); // Invalidate all tools
  return true;   // \todo : think about return value
}


bool mitk::MicroBirdTrackingDevice::StartTracking()
{
  if (this->GetState() != Ready)
    return false;

  this->SetState(Tracking);

  /* Switch on transmitter */
  SwitchTransmitter(true);

  /* Update the local copy of m_StopTracking */
  this->m_StopTrackingMutex->Lock();
  this->m_StopTracking = false;
  this->m_StopTrackingMutex->Unlock();

  m_TrackingFinishedMutex->Unlock(); // transfer the execution rights to tracking thread
  m_ThreadID = m_MultiThreader->SpawnThread(this->ThreadStartTracking, this); // start a new thread that executes the TrackTools() method
  mitk::TimeStamp::GetInstance()->Start(this);
  return true;
}


void mitk::MicroBirdTrackingDevice::TrackTools()
{
  if (this->GetState() != Tracking)
    return;

  /* Frequency configuration */
  double updateRate = 1000.0 / m_SystemConfig.measurementRate;
  double measurementDuration = 0.0;


  // lock the TrackingFinishedMutex to signal that the execution rights
  //   are now transfered to the tracking thread
  MutexLockHolder trackingFinishedLockHolder(*m_TrackingFinishedMutex); // keep lock until end of scope

  // Because m_StopTracking is used by two threads, access has to be guarded
  //   by a mutex. To minimize thread locking, a local copy is used here
  bool localStopTracking;

  /* update the local copy of m_StopTracking */
  this->m_StopTrackingMutex->Lock();
  localStopTracking = this->m_StopTracking;
  this->m_StopTrackingMutex->Unlock();

  /* Tracking loop */
  while ((this->GetState() == Tracking) && (localStopTracking == false))
  {
    int errorCode;
    unsigned int nOfAttachedSensors = 0;
    double timeStamp = 0.0;
    int toolNumber = 0; // Numbers for attached sensors only

    for (int sensorID = 0; sensorID < m_SystemConfig.numberSensors; sensorID++) // for each sensor grep data
    {
      if (!m_SensorConfig[sensorID].attached)
        continue;

      // sensor attached so get record
      errorCode = GetAsynchronousRecord(sensorID, pRecord, sizeof(record));
      if (CompareError(errorCode, BIRD_ERROR_SUCCESS))
      { // On SUCCESS, parse sensor information
        nOfAttachedSensors++;
        timeStamp += record.time; // Get timestamp from record
        ToolType* tool = GetMicroBirdTool(toolNumber); /// Get tool (current sensor)
        if (tool != nullptr)
        {
          tool->SetTrackingError(record.quality); // Set tracking error (quality) from record
          mitk::Point3D position;
          position[0] = record.x;
          position[1] = record.y;
          position[2] = record.z;
          tool->SetPosition(position);  // Set position
          mitk::Quaternion orientation(record.q[1], record.q[2], record.q[3],record.q[0]);
          tool->SetOrientation(orientation); // Set orientation as quaternion \todo : verify quaternion order q(r,x,y,z)
          tool->SetDataValid(true); // Set data state to valid
        }
        toolNumber++; // Increment tool number
      }
      else
      { // ERROR while reading sensor information
        HandleError(errorCode);
      }
    }

    /// @todo : is there any synchronisation?
    // Average timestamp: timeStamp/nOfAttachedSensors

    // Compute sleep time
    double sleepTime = updateRate - measurementDuration;
    // Sleep
    if (sleepTime > 0.0 && sleepTime < 500.0)
    {
      // Note: we only have to approximately sleep one measurement cycle,
      //    since the tracker keeps track of the measurement rate itself
      itksys::SystemTools::Delay(sleepTime)
      //Sleep(static_cast<DWORD>(sleepTime));
    }

    // Update the local copy of m_StopTracking
    this->m_StopTrackingMutex->Lock();
    localStopTracking = m_StopTracking;
    this->m_StopTrackingMutex->Unlock();
  }

  // @bug (#1813) : maybe we need to check for localStopTracking=true here?
  //    m_StopTracking should only ever be updated by StopTracking(), so
  //    maybe we should not unlock a mutex that nobody is waiting for?

  return; // returning from this function (and ThreadStartTracking()) this will end the thread
}


mitk::TrackingTool* mitk::MicroBirdTrackingDevice::GetTool(unsigned int toolNumber)
{
  return static_cast<TrackingTool*>(GetMicroBirdTool(toolNumber));
}


mitk::MicroBirdTrackingDevice::ToolType* mitk::MicroBirdTrackingDevice::GetMicroBirdTool(unsigned int toolNumber)
{
  ToolType* t = nullptr;

  MutexLockHolder toolsMutexLockHolder(*m_ToolsMutex); // lock and unlock the mutex
  if (toolNumber < m_Tools.size())
  {
    t = m_Tools.at(toolNumber);
  }
  return t;
}


unsigned int mitk::MicroBirdTrackingDevice::GetToolCount() const
{
  MutexLockHolder toolsMutexLockHolder(*m_ToolsMutex); // lock and unlock the mutex
  return m_Tools.size();
}


bool mitk::MicroBirdTrackingDevice::CompareError(int errorCode, int errorConstant)
{
  return ((errorCode & 0xffff) == errorConstant);
}


void mitk::MicroBirdTrackingDevice::HandleError(int errorCode)
{
  char buffer[1024];
  char* pBuffer = &buffer[0];

  while(!CompareError(errorCode, BIRD_ERROR_SUCCESS))
  {
    // Print error number on screen
    //cout << "MicroBIRD Error Code: " << errorCode << endl;
    // Print error message on screen
    errorCode = GetErrorText(errorCode, pBuffer, sizeof(buffer), SIMPLE_MESSAGE);
    /// @todo : set error message, does it work?
    this->SetErrorMessage(buffer);
  }
}


void mitk::MicroBirdTrackingDevice::InvalidateAll()
{
  MutexLockHolder toolsMutexLockHolder(*m_ToolsMutex); // lock and unlock the mutex
  for (ToolContainerType::iterator iterator = m_Tools.begin(); iterator != m_Tools.end(); ++iterator)
    (*iterator)->SetDataValid(false);
}
