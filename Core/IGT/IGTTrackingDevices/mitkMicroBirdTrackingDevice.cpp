/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2008-07-03 12:25:58 +0200 (Do, 03 Jul 2008) $
Version:   $Revision: 14720 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkMicrobirdTrackingDevice.h"

#include <itksys/SystemTools.hxx>

// Flat transmitter needs measurement rate: 40.5
// Mid-range transmitter needs measurement rate: 68.3;


mitk::MicroBirdTrackingDevice::MicroBirdTrackingDevice() :
TrackingDevice(),
m_ErrorMessage(""),
m_ThreadID(0),
m_pl(50), // 50 Hz for Europe
m_metric(true),
m_agcModeBoth(true),
m_measurementRate(68.3), // 68.3 for mid-range transmitter, 40.5 for flat transmitter
m_TransmitterConfig(NULL),
m_SensorConfig(NULL)
{

  // Set the tracker type
  this->m_Type = AscensionMicroBird;

  // Set the tracking volume
  this->m_TrackingVolume->setTrackingDeviceType(this->m_Type);

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
  // empty
}

bool mitk::MicroBirdTrackingDevice::OpenConnection()
{
  // Grab mode mutex
  this->m_ModeMutex->Lock();

  // Check whether in setup mode
  if (this->m_Mode != Setup)
  {
    this->SetErrorMessage("Can only try to open the connection if in setup mode");
    this->m_ModeMutex->Unlock();
    return false;
  }

  // Holds error code
  int errorCode;

  // Debug: info message
  cout << "Initializing PCIBird - MicroBIRD" << endl;

  // Initialize the PCIBIRD driver and DLL
  errorCode = InitializeBIRDSystem(); // this function can take a few seconds
  if(!errorCompare(errorCode, BIRD_ERROR_SUCCESS))
  {
    errorHandler(errorCode);
    this->m_ModeMutex->Unlock();
    return false;
  }

  /// @todo : check for transmitter serial numbers here?
  // Serial numbers could be compared to known ones for some simple
  //    parameters sanity check (measurement frequency etc.)

  // Get system configuration
  errorCode = GetBIRDSystemConfiguration(&m_SystemConfig);
  if(!errorCompare(errorCode, BIRD_ERROR_SUCCESS))
  {
    errorHandler(errorCode);
    this->m_ModeMutex->Unlock();
    return false;
  }

  // use metric measurements in mm
  errorCode = SetSystemParameter(METRIC, &m_metric, sizeof(m_metric)); 
  if(!errorCompare(errorCode, BIRD_ERROR_SUCCESS))
  {
    errorHandler(errorCode);
    this->m_ModeMutex->Unlock();
    return false;
  }

  // Set the measurement rate to m_measurementRate
  if (30 < m_measurementRate && m_measurementRate < 80)
  {
    errorCode = SetSystemParameter(MEASUREMENT_RATE, &m_measurementRate, sizeof(m_measurementRate)); 
    if(!errorCompare(errorCode, BIRD_ERROR_SUCCESS))
    {
      errorHandler(errorCode);
      this->m_ModeMutex->Unlock();
      return false;
    }
  }
  else
  {
    std::cout << "Warning: measurement rate " << m_measurementRate << " failed sanity check." << std::endl;
  }

  // Set power line frequency
  if (50 <= m_pl && m_pl <= 60)
  {
    errorCode = SetSystemParameter(POWER_LINE_FREQUENCY, &m_pl, sizeof(m_pl));
    if(!errorCompare(errorCode, BIRD_ERROR_SUCCESS))
    {
      errorHandler(errorCode);
      this->m_ModeMutex->Unlock();
      return false;
    }
  }
  else
  {
    std::cout << "Warning: power line frequency " << m_pl << " failed sanity check." << std::endl;
  }

  // Set AGC mode
  m_agc = m_agcModeBoth ? TRANSMITTER_AND_SENSOR_AGC : SENSOR_AGC_ONLY;
  errorCode = SetSystemParameter(AGC_MODE, &m_agc, sizeof(m_agc));
  if(!errorCompare(errorCode, BIRD_ERROR_SUCCESS))
  {
    errorHandler(errorCode);
    this->m_ModeMutex->Unlock();
    return false;
  }

  // Get system configuration
  errorCode = GetBIRDSystemConfiguration(&m_SystemConfig);
  if(!errorCompare(errorCode, BIRD_ERROR_SUCCESS))
  {
    errorHandler(errorCode);
    return false;
  }

  // Get sensor information
  m_SensorConfig = new SENSOR_CONFIGURATION[m_SystemConfig.numberSensors];
  for(int i = 0; i < m_SystemConfig.numberSensors; i++)
  {
    errorCode = GetSensorConfiguration(i, &(m_SensorConfig[i]));
    if(!errorCompare(errorCode, BIRD_ERROR_SUCCESS))
    {
      errorHandler(errorCode);
    }

    // Initialize the quality parameter structure
    QUALITY_PARAMETERS qualityParameters; // = { 164, 0, 32, 3072 };
    GetSensorParameter(i, QUALITY, &qualityParameters, sizeof(qualityParameters));

    // Set data format to matrix format
    //DATA_FORMAT_TYPE tempBuffer = DOUBLE_POSITION_MATRIX_TIME_Q;
    // Set data format to quaternion format
    DATA_FORMAT_TYPE tempBuffer = DOUBLE_POSITION_QUATERNION_TIME_Q;

    // Set data format for sensor
    DATA_FORMAT_TYPE *pTempBuffer = &tempBuffer;
    errorCode = SetSensorParameter(i, DATA_FORMAT, pTempBuffer, sizeof(tempBuffer));	
    if(!errorCompare(errorCode, BIRD_ERROR_SUCCESS))
    {
      errorHandler(errorCode);
    }
  }

  m_ToolsMutex->Lock();
  // Initialise tools vector
  for(int i = 0; i < m_SystemConfig.numberSensors; i++)
  {
    if(m_SensorConfig[i].attached)
    {
      std::cout << " * sensor " << i << " attached.\n";
      m_Tools.push_back(MicroBirdTool::New());
    }
    else
    {
      std::cout << " * sensor " << i << " not attached.\n";
    }
  }
  m_ToolsMutex->Unlock();

  // Get transmitter configuration
  m_TransmitterConfig = new TRANSMITTER_CONFIGURATION[m_SystemConfig.numberTransmitters];
  for(int i = 0; i < m_SystemConfig.numberTransmitters; i++)
  {
    errorCode = GetTransmitterConfiguration(i, &(m_TransmitterConfig[i]));
    if(!errorCompare(errorCode, BIRD_ERROR_SUCCESS))
    {
      errorHandler(errorCode);
    }
  }

  // Switch off transmitter
  SwitchTransmitter(true);
  SwitchTransmitter(false);

  /// @todo : set up error scaling?

  // finish  - now all tools should be added, initalized and enabled, so that tracking can be started
  this->SetMode(Ready);
  this->SetErrorMessage("");
  this->m_ModeMutex->Unlock();

  // Debug: info message
  cout << "... initialisation finished." << endl;

  // Return success
  return true; 
}

bool mitk::MicroBirdTrackingDevice::SwitchTransmitter(bool switchOn)
{
  // Debug message
  //cout << " * switching transmitter " << (switchOn ? "on" : "off" ) << endl;

  if (switchOn)
  {
    // Search for the first attached transmitter and turn it on
    for(short id = 0; id < m_SystemConfig.numberTransmitters; id++)
    {
      if(m_TransmitterConfig[id].attached)
      {
        // Transmitter selection is a system function.
        // Using the SELECT_TRANSMITTER parameter we send the id of the
        // transmitter that we want to run with the SetSystemParameter() call
        int errorCode = SetSystemParameter(SELECT_TRANSMITTER, &id, sizeof(id));
        if(!errorCompare(errorCode, BIRD_ERROR_SUCCESS))
        {
          errorHandler(errorCode);
        }
        else
        {
          cout << " * transmitter " << id << " switched on" << endl;
        }
        break;
      }
    }
  }
  else
  {
    // Transmitter selection is a system function,
    // Note: a selector of -1 switches off the current transmitter
    short TRANSMITTER_OFF = -1;
    int errorCode = SetSystemParameter(SELECT_TRANSMITTER, &TRANSMITTER_OFF, sizeof(TRANSMITTER_OFF));
    if(errorCompare(errorCode, BIRD_ERROR_SUCCESS))
    {
      cout << " * transmitter switched off" << endl;
    }
    if(errorCompare(errorCode, BIRD_ERROR_NO_TRANSMITTER_RUNNING))
    {
      cout << " * no transmitter running" << endl;
    }
    else
    {
      errorHandler(errorCode);
      return false;
    }
  }

  // Return success
  return true;
}
bool mitk::MicroBirdTrackingDevice::CloseConnection()
{
  // Debug: info message
  cout << "Closing connection..." << endl;

  // Grab mode mutex
  this->m_ModeMutex->Lock();

  // Switch off the transmitter
  SwitchTransmitter(false);

  // Close connection
  int errorCode = CloseBIRDSystem(); // this function can take a few seconds

  // Error checking
  if(!errorCompare(errorCode, BIRD_ERROR_SUCCESS))
  {
    errorHandler(errorCode);
  }

  // Delete configuration
  if (m_TransmitterConfig)
  {
    delete [] m_TransmitterConfig;
  }
  if (m_SensorConfig)
  {
    delete [] m_SensorConfig;
  }

  // Change mode and release mutex
  this->SetMode(Setup);
  m_ModeMutex->Unlock();

  // Clear error message
  this->SetErrorMessage("");

  // Debug: info message
  cout << "... connection closed." << endl;

  return true; 
}


ITK_THREAD_RETURN_TYPE mitk::MicroBirdTrackingDevice::ThreadStartTracking(void* pInfoStruct)
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
  MicroBirdTrackingDevice *trackingDevice = (MicroBirdTrackingDevice*)pInfo->UserData;
  if (trackingDevice != NULL)
  {
    trackingDevice->TrackTools();             // call TrackTools() from the original object
  }
  return ITK_THREAD_RETURN_VALUE;
}

bool mitk::MicroBirdTrackingDevice::StopTracking()
{
  // Debug: info message
  cout << "Stopping tracking..." << endl;

  // Call superclass method
  TrackingDevice::StopTracking();

  // Switch off transmitter
  SwitchTransmitter(false);
  // Invalidate all tools
  InvalidateAll();

  // Debug: info message
  cout << "... tracking stopped." << endl;
  
  /// @todo : think about return value
  return true;
}

bool mitk::MicroBirdTrackingDevice::StartTracking()
{
  // Debug: info message
  cout << "Starting tracking..." << endl;

  // Grab mode mutex
  this->m_ModeMutex->Lock();
  if (m_Mode != Ready)
  {
    this->m_ModeMutex->Unlock();
    cout << "... already tracking?" << endl;
    return false;
  }

  // Go to mode Tracking
  this->SetMode(Tracking);

  // Switch on transmitter
  SwitchTransmitter(true);

  // Update the local copy of m_StopTracking
  this->m_StopTrackingMutex->Lock();
  this->m_StopTracking = false;
  this->m_StopTrackingMutex->Unlock();

  // transfer the execution rights to tracking thread
  m_TrackingFinishedMutex->Unlock();

  // start a new thread that executes the TrackTools() method
  m_ThreadID = m_MultiThreader->SpawnThread(this->ThreadStartTracking, this);

  // Release mode mutex
  this->m_ModeMutex->Unlock();

  // Debug: info message
  cout << "... tracking started." << endl;

  return true;
}

void mitk::MicroBirdTrackingDevice::TrackTools()
{
  // lock the TrackingFinishedMutex to signal that the execution rights
  //   are now transfered to the tracking thread
  m_TrackingFinishedMutex->Lock();

  // Because m_StopTracking is used by two threads, access has to be guarded
  //   by a mutex. To minimize thread locking, a local copy is used here 
  bool localStopTracking;

  // update the local copy of m_StopTracking
  this->m_StopTrackingMutex->Lock();
  localStopTracking = this->m_StopTracking;
  this->m_StopTrackingMutex->Unlock();

  // Frequency configuration	
  double updateRate = 1000.0 / m_SystemConfig.measurementRate;
  double measurementDuration = 0.0;

  // Tracking loop
  while ((this->GetMode() == Tracking) && (localStopTracking == false))
  {
    // Update tools here
    int errorCode;

    unsigned int nOfAttachedSensors = 0;
    double timeStamp = 0.0;

    // Invalidate all data
    // We don't invalidate tools, because the MicroBird does not support hotplug anyway
    //    and we always get /some/ data...
    //InvalidateAll();

    // Numbers for attached sensors only
    int toolNumber = 0;

    // for each sensor grep data
    for(int sensorID = 0; sensorID < m_SystemConfig.numberSensors; sensorID++)
    {
      if(m_SensorConfig[sensorID].attached)
      {
        // sensor attached so get record
        errorCode = GetAsynchronousRecord(sensorID, pRecord, sizeof(record));
        if(!errorCompare(errorCode, BIRD_ERROR_SUCCESS))
        {
          errorHandler(errorCode);
        }
        else
        {
          nOfAttachedSensors++;

          // Get timestamp from record
          double currentTime = record.time;
          timeStamp += currentTime;

          /// Get tool (current sensor)
          MicroBirdTool::Pointer tool = GetMicroBirdTool(toolNumber);

          // Set tracking error (quality) from record
          tool->SetTrackingError(record.quality);

          // Set position
          tool->SetPosition(record.x, record.y, record.z);

          // Set orientation as quaternion
          /// @todo : verify quaternion order q(r,x,y,z)
          tool->SetQuaternion(record.q[0], record.q[1], record.q[2], record.q[3]);

          // Set data state to valid
          tool->SetDataValid(true);

          // Increment tool number
          toolNumber++;

        }	// if(!errorCompare(errorCode, BIRD_ERROR_SUCCESS))
      } // if(m_SensorConfig[sensorID].attached)
    } // for(sensorID)

    /// @todo : is there any synchronisation?
    // Average timestamp: timeStamp/nOfAttachedSensors

    // Compute sleep time
    double sleepTime = updateRate - measurementDuration;
    // Sleep
    if (sleepTime>0.0 && sleepTime<500.0)
    {
      // Note: we only have to approximately sleep one measurement cycle,
      //    since the tracker keeps track of the measurement rate itself
      Sleep(static_cast<DWORD>(sleepTime));
    }

    // Update the local copy of m_StopTracking
    this->m_StopTrackingMutex->Lock();  
    localStopTracking = m_StopTracking;
    this->m_StopTrackingMutex->Unlock();
  }

  // @bug (#1813) : maybe we need to check for localStopTracking=true here?
  //    m_StopTracking should only ever be updated by StopTracking(), so
  //    maybe we should not unlock a mutex that nobody is waiting for?

  // transfer control back to main thread (last action here)
  m_TrackingFinishedMutex->Unlock();

  // returning from this function (and ThreadStartTracking()) this will end the thread
  return;
}

mitk::TrackingTool* mitk::MicroBirdTrackingDevice::GetTool(unsigned int toolNumber)
{
  return static_cast<TrackingTool*>(GetMicroBirdTool(toolNumber));
}

mitk::MicroBirdTool* mitk::MicroBirdTrackingDevice::GetMicroBirdTool(unsigned int toolNumber)
{
  mitk::MicroBirdTool* t = NULL;

  m_ToolsMutex->Lock();
  if (toolNumber < m_Tools.size())
  {
    t = m_Tools.at(toolNumber);
  }
  m_ToolsMutex->Unlock();
  return t;
}

unsigned int mitk::MicroBirdTrackingDevice::GetToolCount() const
{
  unsigned int s = 0;
  m_ToolsMutex->Lock();
  s = m_Tools.size();
  m_ToolsMutex->Unlock();
  return s;
}

bool mitk::MicroBirdTrackingDevice::errorCompare(int errorCode, int errorConstant)
{
  return ((errorCode & 0xffff) == errorConstant);
}

void mitk::MicroBirdTrackingDevice::errorHandler(int errorCode)
{
  char			buffer[1024];
  char			*pBuffer = &buffer[0];

  while(!errorCompare(errorCode, BIRD_ERROR_SUCCESS))
  {
    // Print error number on screen
    //cout << "MicroBIRD Error Code: " << errorCode << endl;
    // Print error message on screen
    errorCode = GetErrorText(errorCode, pBuffer, sizeof(buffer), SIMPLE_MESSAGE);
    cout << "MicroBIRD Error Text: " << buffer << endl;

    /// @todo : set error message, does it work?
    this->SetErrorMessage(buffer);
  }
}

void mitk::MicroBirdTrackingDevice::InvalidateAll()
{
  m_ToolsMutex->Lock();
  for (ToolContainerType::iterator iterator = m_Tools.begin(); iterator != m_Tools.end(); ++iterator)
    (*iterator)->SetDataValid(false);
  m_ToolsMutex->Unlock();
}
