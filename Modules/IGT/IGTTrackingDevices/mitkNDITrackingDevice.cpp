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

#include "mitkNDITrackingDevice.h"
#include "mitkTimeStamp.h"
#include <stdio.h>

#include <itksys/SystemTools.hxx>
#include <itkMutexLockHolder.h>

typedef itk::MutexLockHolder<itk::FastMutexLock> MutexLockHolder;


const unsigned char CR = 0xD; // == '\r' - carriage return
const unsigned char LF = 0xA; // == '\n' - line feed


mitk::NDITrackingDevice::NDITrackingDevice() :
TrackingDevice(),m_DeviceName(""), m_PortNumber(mitk::SerialCommunication::COM5), m_BaudRate(mitk::SerialCommunication::BaudRate9600),
m_DataBits(mitk::SerialCommunication::DataBits8), m_Parity(mitk::SerialCommunication::None), m_StopBits(mitk::SerialCommunication::StopBits1),
m_HardwareHandshake(mitk::SerialCommunication::HardwareHandshakeOff), m_NDITrackingVolume(Standard),
m_IlluminationActivationRate(Hz20), m_DataTransferMode(TX), m_6DTools(), m_ToolsMutex(NULL), 
m_SerialCommunication(NULL), m_SerialCommunicationMutex(NULL), m_DeviceProtocol(NULL),
m_MultiThreader(NULL), m_ThreadID(0), m_OperationMode(ToolTracking6D), m_MarkerPointsMutex(NULL), m_MarkerPoints()
{
  this->m_Type = TrackingSystemNotSpecified; //NDIPolaris;          //  = 0; //set the type = 0 (=Polaris, default)
  m_6DTools.clear();
  m_SerialCommunicationMutex = itk::FastMutexLock::New();
  m_DeviceProtocol = NDIProtocol::New();
  m_DeviceProtocol->SetTrackingDevice(this);
  m_DeviceProtocol->UseCRCOn();
  m_MultiThreader = itk::MultiThreader::New();
  m_ToolsMutex = itk::FastMutexLock::New();
  m_MarkerPointsMutex = itk::FastMutexLock::New();
  m_MarkerPoints.reserve(50);   // a maximum of 50 marker positions can be reported by the tracking device
}


bool mitk::NDITrackingDevice::UpdateTool(mitk::TrackingTool* tool)
{
  if (this->GetMode() != Setup)
  {
    mitk::NDIPassiveTool* ndiTool = dynamic_cast<mitk::NDIPassiveTool*>(tool);
    if (ndiTool == NULL)
      return false;

    std::string portHandle = ndiTool->GetPortHandle();

    //return false if the SROM Data has not been set
    if (ndiTool->GetSROMData() == NULL)
      return false;

    NDIErrorCode returnvalue;
    returnvalue = m_DeviceProtocol->PVWR(&portHandle, ndiTool->GetSROMData(), ndiTool->GetSROMDataLength());
    if (returnvalue != NDIOKAY)
      return false;
    returnvalue = m_DeviceProtocol->PINIT(&portHandle);
    if (returnvalue != NDIOKAY)
      return false;
    returnvalue = m_DeviceProtocol->PENA(&portHandle, ndiTool->GetTrackingPriority()); // Enable tool
    if (returnvalue != NDIOKAY)
      return false;

    return true;
  }
  else
  {
    return false;
  }
}


mitk::NDITrackingDevice::~NDITrackingDevice()
{
  /* stop tracking and disconnect from tracking device */
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
  /* free serial communication interface */
  if (m_SerialCommunication.IsNotNull())
  {
    m_SerialCommunication->ClearReceiveBuffer();
    m_SerialCommunication->ClearSendBuffer();
    m_SerialCommunication->CloseConnection();
    m_SerialCommunication = NULL;
  }
}


void mitk::NDITrackingDevice::SetPortNumber(const PortNumber _arg)
{
  if (this->GetMode() != Setup)
    return;
  itkDebugMacro("setting PortNumber to " << _arg);
  if (this->m_PortNumber != _arg)
  {
    this->m_PortNumber = _arg;
    this->Modified();
  }
}


void mitk::NDITrackingDevice::SetDeviceName(std::string _arg)
{
  if (this->GetMode() != Setup)
    return;
  itkDebugMacro("setting eviceName to " << _arg);
  if (this->m_DeviceName != _arg)
  {
    this->m_DeviceName = _arg;
    this->Modified();
  }
}


void mitk::NDITrackingDevice::SetBaudRate(const BaudRate _arg)
{
  if (this->GetMode() != Setup)
    return;
  itkDebugMacro("setting BaudRate to " << _arg);
  if (this->m_BaudRate != _arg)
  {
    this->m_BaudRate = _arg;
    this->Modified();
  }
}


void mitk::NDITrackingDevice::SetDataBits(const DataBits _arg)
{
  if (this->GetMode() != Setup)
    return;
  itkDebugMacro("setting DataBits to " << _arg);
  if (this->m_DataBits != _arg)
  {
    this->m_DataBits = _arg;
    this->Modified();
  }
}


void mitk::NDITrackingDevice::SetParity(const Parity _arg)
{
  if (this->GetMode() != Setup)
    return;
  itkDebugMacro("setting Parity to " << _arg);
  if (this->m_Parity != _arg)
  {
    this->m_Parity = _arg;
    this->Modified();
  }
}


void mitk::NDITrackingDevice::SetStopBits(const StopBits _arg)
{
  if (this->GetMode() != Setup)
    return;
  itkDebugMacro("setting StopBits to " << _arg);
  if (this->m_StopBits != _arg)
  {
    this->m_StopBits = _arg;
    this->Modified();
  }
}


void mitk::NDITrackingDevice::SetHardwareHandshake(const HardwareHandshake _arg)
{
  if (this->GetMode() != Setup)
    return;
  itkDebugMacro("setting HardwareHandshake to " << _arg);
  if (this->m_HardwareHandshake != _arg)
  {
    this->m_HardwareHandshake = _arg;
    this->Modified();
  }
}


void mitk::NDITrackingDevice::SetIlluminationActivationRate(const IlluminationActivationRate _arg)
{
  if (this->GetMode() == Tracking)
    return;
  itkDebugMacro("setting IlluminationActivationRate to " << _arg);
  if (this->m_IlluminationActivationRate != _arg)
  {
    this->m_IlluminationActivationRate = _arg;
    this->Modified();
    if (this->GetMode() == Ready)   // if the connection to the tracking system is established, send the new rate to the tracking device too
      m_DeviceProtocol->IRATE(this->m_IlluminationActivationRate);
  }
}


void mitk::NDITrackingDevice::SetDataTransferMode(const DataTransferMode _arg)
{
  itkDebugMacro("setting DataTransferMode to " << _arg);
  if (this->m_DataTransferMode != _arg)
  {
    this->m_DataTransferMode = _arg;
    this->Modified();
  }
}


mitk::NDIErrorCode mitk::NDITrackingDevice::Send(const std::string* input, bool addCRC)
{
  if (input == NULL)
    return SERIALSENDERROR;

  std::string message;

  if (addCRC == true)
    message = *input + CalcCRC(input) + std::string(1, CR);
  else
    message = *input + std::string(1, CR);

  //unsigned int messageLength = message.length() + 1; // +1 for CR

  // Clear send buffer
  this->ClearSendBuffer();
  // Send the date to the device
  MutexLockHolder lock(*m_SerialCommunicationMutex); // lock and unlock the mutex
  long returnvalue = m_SerialCommunication->Send(message);

  if (returnvalue == 0)
    return SERIALSENDERROR;
  else
    return NDIOKAY;
}


mitk::NDIErrorCode mitk::NDITrackingDevice::Receive(std::string* answer, unsigned int numberOfBytes)
{
  if (answer == NULL)
    return SERIALRECEIVEERROR;

  MutexLockHolder lock(*m_SerialCommunicationMutex); // lock and unlock the mutex
  long returnvalue = m_SerialCommunication->Receive(*answer, numberOfBytes);  // never read more bytes than the device has send, the function will block until enough bytes are send...

  if (returnvalue == 0)
    return SERIALRECEIVEERROR;
  else
    return NDIOKAY;
}


mitk::NDIErrorCode mitk::NDITrackingDevice::ReceiveByte(char* answer)
{
  if (answer == NULL)
    return SERIALRECEIVEERROR;

  std::string m;

  MutexLockHolder lock(*m_SerialCommunicationMutex); // lock and unlock the mutex  
  
  long returnvalue = m_SerialCommunication->Receive(m, 1);

  if ((returnvalue == 0) ||(m.size() != 1))
    return SERIALRECEIVEERROR;

  *answer = m.at(0);
  return NDIOKAY;
}


mitk::NDIErrorCode mitk::NDITrackingDevice::ReceiveLine(std::string* answer)
{
  if (answer == NULL)
    return SERIALRECEIVEERROR;

  std::string m;

  MutexLockHolder lock(*m_SerialCommunicationMutex); // lock and unlock the mutex  

  do
  {
    long returnvalue = m_SerialCommunication->Receive(m, 1);
    if ((returnvalue == 0) ||(m.size() != 1))
      return SERIALRECEIVEERROR;
    *answer += m;
  } while (m.at(0) != LF);
  return NDIOKAY;
}


void mitk::NDITrackingDevice::ClearSendBuffer()
{
  MutexLockHolder lock(*m_SerialCommunicationMutex); // lock and unlock the mutex
  m_SerialCommunication->ClearSendBuffer();
}


void mitk::NDITrackingDevice::ClearReceiveBuffer()
{
  MutexLockHolder lock(*m_SerialCommunicationMutex); // lock and unlock the mutex
  m_SerialCommunication->ClearReceiveBuffer();
}


const std::string mitk::NDITrackingDevice::CalcCRC(const std::string* input)
{

  if (input == NULL)
    return "";
  /* the crc16 calculation code is taken from the NDI API guide example code section */
  static int oddparity[16] = {0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0};
  unsigned int data;  // copy of the input string's current character
  unsigned int crcValue = 0;  // the crc value is stored here
  unsigned int* puCRC16 = &crcValue;  // the algorithm uses a pointer to crcValue, so it's easier to provide that than to change the algorithm
  for (unsigned int i = 0; i < input->length(); i++)
  {
    data = (*input)[i];
    data = (data ^ (*(puCRC16) & 0xff)) & 0xff;
    *puCRC16 >>= 8;
    if (oddparity[data & 0x0f] ^ oddparity[data >> 4])
    {
      *(puCRC16) ^= 0xc001;
    }
    data <<= 6;
    *puCRC16 ^= data;
    data <<= 1;
    *puCRC16 ^= data;
  }
  // crcValue contains now the CRC16 value. Convert it to a string and return it
  char returnvalue[13];
  sprintf(returnvalue,"%04X", crcValue);  // 4 hexadecimal digit with uppercase format
  return std::string(returnvalue);
}


bool mitk::NDITrackingDevice::OpenConnection()
{
  
  //this->m_ModeMutex->Lock();
  if (this->GetMode() != Setup)
  {
    this->SetErrorMessage("Can only try to open the connection if in setup mode");
    return false;
  }

  //

  m_SerialCommunication = mitk::SerialCommunication::New();

  /* init local com port to standard com settings for a NDI tracking device:
  9600 baud, 8 data bits, no parity, 1 stop bit, no hardware handshake
  */
  if (m_DeviceName.empty())
    m_SerialCommunication->SetPortNumber(m_PortNumber);
  else
    m_SerialCommunication->SetDeviceName(m_DeviceName);
  m_SerialCommunication->SetBaudRate(mitk::SerialCommunication::BaudRate9600);
  m_SerialCommunication->SetDataBits(mitk::SerialCommunication::DataBits8);
  m_SerialCommunication->SetParity(mitk::SerialCommunication::None);
  m_SerialCommunication->SetStopBits(mitk::SerialCommunication::StopBits1);
  m_SerialCommunication->SetSendTimeout(5000);
  m_SerialCommunication->SetReceiveTimeout(5000);
  if (m_SerialCommunication->OpenConnection() == 0) // 0 == ERROR_VALUE
  {
    this->SetErrorMessage("Can not open serial port");
    m_SerialCommunication->CloseConnection();
    m_SerialCommunication = NULL;
    return false;
  }

  /* Reset Tracking device by sending a serial break for 500ms */
  m_SerialCommunication->SendBreak(400);

  /* Read answer from tracking device (RESETBE6F) */
  static const std::string reset("RESETBE6F\r");
  std::string answer = "";
  this->Receive(&answer, reset.length());  // read answer (should be RESETBE6F)
  this->ClearReceiveBuffer();     // flush the receive buffer of all remaining data (carriage return, strings other than reset
  if (reset.compare(answer) != 0)  // check for RESETBE6F
  {
    this->SetErrorMessage("Hardware Reset of tracking device did not work");
    if (m_SerialCommunication.IsNotNull())
    {
      m_SerialCommunication->CloseConnection();
      m_SerialCommunication = NULL;
    }
    return false;
  }

  /* Now the tracking device is reset, start initialization */
  NDIErrorCode returnvalue;

  /* set device com settings to new values and wait for the device to change them */
  returnvalue = m_DeviceProtocol->COMM(m_BaudRate, m_DataBits, m_Parity, m_StopBits, m_HardwareHandshake);

  if (returnvalue != NDIOKAY)
  {
    this->SetErrorMessage("Could not set comm settings in trackingdevice");
    return false;
  }

  //after changing COMM wait at least 100ms according to NDI Api documentation page 31
  itksys::SystemTools::Delay(500);

  /* now change local com settings accordingly */
  m_SerialCommunication->CloseConnection();
  m_SerialCommunication->SetBaudRate(m_BaudRate);
  m_SerialCommunication->SetDataBits(m_DataBits);
  m_SerialCommunication->SetParity(m_Parity);
  m_SerialCommunication->SetStopBits(m_StopBits);
  m_SerialCommunication->SetHardwareHandshake(m_HardwareHandshake);
  m_SerialCommunication->SetSendTimeout(5000);
  m_SerialCommunication->SetReceiveTimeout(5000);
  m_SerialCommunication->OpenConnection();
  /* initialize the tracking device */
  returnvalue = m_DeviceProtocol->INIT();
  if (returnvalue != NDIOKAY)
  {
    this->SetErrorMessage("Could not initialize the tracking device");
    return false;
  }

  if (this->GetType() == mitk::TrackingSystemNotSpecified)  // if the type of tracking device is not specified, try to query the connected device
  {
    mitk::TrackingDeviceType deviceType;
    returnvalue = m_DeviceProtocol->VER(deviceType);
    if ((returnvalue != NDIOKAY) || (deviceType == mitk::TrackingSystemNotSpecified))
    {
      this->SetErrorMessage("Could not determine tracking device type. Please set manually and try again.");
      return false;
    }
    this->SetType(deviceType);
  }

  /****  Optional Polaris specific code, Work in progress
  // start diagnostic mode
  returnvalue = m_DeviceProtocol->DSTART();
  if (returnvalue != NDIOKAY)
  {
  this->SetErrorMessage("Could not start diagnostic mode");
  return false;
  }
  else    // we are in diagnostic mode
  {
  // initialize extensive IR checking
  returnvalue = m_DeviceProtocol->IRINIT();
  if (returnvalue != NDIOKAY)
  {
  this->SetErrorMessage("Could not initialize intense infrared light checking");
  return false;
  }
  bool intenseIR = false;
  returnvalue = m_DeviceProtocol->IRCHK(&intenseIR);
  if (returnvalue != NDIOKAY)
  {
  this->SetErrorMessage("Could not execute intense infrared light checking");
  return false;
  }
  if (intenseIR == true)
  // do something - warn the user, raise exception, write to protocol or similar
  std::cout << "Warning: Intense infrared light detected. Accurate tracking will probably not be possible.\n";

  // stop diagnictic mode
  returnvalue = m_DeviceProtocol->DSTOP();
  if (returnvalue != NDIOKAY)
  {
  this->SetErrorMessage("Could not stop diagnostic mode");
  return false;
  }
  }
  *** end of optional polaris code ***/

  /**
  * now add tools to the tracking system
  **/

  /* First, check if the tracking device has port handles that need to be freed and free them */
  returnvalue = FreePortHandles();
  // non-critical, therefore no error handling

  /**
  * POLARIS: initialize the tools that were added manually
  **/
  {
    
    MutexLockHolder toolsMutexLockHolder(*m_ToolsMutex); // lock and unlock the mutex
    std::string portHandle;
    Tool6DContainerType::iterator endIt = m_6DTools.end();
    for(Tool6DContainerType::iterator it = m_6DTools.begin(); it != endIt; ++it)
    {
      /* get a port handle for the tool */
      returnvalue = m_DeviceProtocol->PHRQ(&portHandle);
      if (returnvalue == NDIOKAY)
      {
        (*it)->SetPortHandle(portHandle.c_str());
        /* now write the SROM file of the tool to the tracking system using PVWR */
        if (this->m_Type == NDIPolaris)
        {
          returnvalue = m_DeviceProtocol->PVWR(&portHandle, (*it)->GetSROMData(), (*it)->GetSROMDataLength());
          if (returnvalue != NDIOKAY)
          {
            this->SetErrorMessage((std::string("Could not write SROM file for tool '") + (*it)->GetToolName() + std::string("' to tracking device")).c_str());
            return false;
          }
          returnvalue = m_DeviceProtocol->PINIT(&portHandle);
          if (returnvalue != NDIOKAY)
          {
            this->SetErrorMessage((std::string("Could not initialize tool '") + (*it)->GetToolName()).c_str());
            return false;
          }
          if ((*it)->IsEnabled() == true)
          {
            returnvalue = m_DeviceProtocol->PENA(&portHandle, (*it)->GetTrackingPriority()); // Enable tool
            if (returnvalue != NDIOKAY)
            {
              this->SetErrorMessage((std::string("Could not enable port '") + portHandle +
                std::string("' for tool '")+ (*it)->GetToolName() + std::string("'")).c_str());
              return false;
            }
          }
        }
      }
    }  
  } // end of toolsmutexlockholder scope

  /* check for wired tools and add them too */
  if (this->DiscoverWiredTools() == false)  // query the tracking device for wired tools and add them to our tool list
    return false; // \TODO: could we continue anyways?


  /*POLARIS: set the illuminator activation rate */
  if (this->m_Type == NDIPolaris)
  {
    returnvalue = m_DeviceProtocol->IRATE(this->m_IlluminationActivationRate);
    if (returnvalue != NDIOKAY)
    {
      this->SetErrorMessage("Could not set the illuminator activation rate");
      return false;
    }
  }
  /* finish  - now all tools should be added, initialized and enabled, so that tracking can be started */
  this->SetMode(Ready);
  this->SetErrorMessage("");
  return true;
}


mitk::TrackingDeviceType mitk::NDITrackingDevice::TestConnection()
{
  if (this->GetMode() != Setup)
  {
    return mitk::TrackingSystemNotSpecified;
  }

  m_SerialCommunication = mitk::SerialCommunication::New();
  //m_DeviceProtocol =  mitk::NDIProtocol::New();
  //m_DeviceProtocol->SetTrackingDevice(this);
  //m_DeviceProtocol->UseCRCOn();
  /* init local com port to standard com settings for a NDI tracking device:
  9600 baud, 8 data bits, no parity, 1 stop bit, no hardware handshake
  */
  if (m_DeviceName.empty())
    m_SerialCommunication->SetPortNumber(m_PortNumber);
  else
    m_SerialCommunication->SetDeviceName(m_DeviceName);

  m_SerialCommunication->SetBaudRate(mitk::SerialCommunication::BaudRate9600);
  m_SerialCommunication->SetDataBits(mitk::SerialCommunication::DataBits8);
  m_SerialCommunication->SetParity(mitk::SerialCommunication::None);
  m_SerialCommunication->SetStopBits(mitk::SerialCommunication::StopBits1);
  m_SerialCommunication->SetSendTimeout(5000);
  m_SerialCommunication->SetReceiveTimeout(5000);
  if (m_SerialCommunication->OpenConnection() == 0) // error
  {
    m_SerialCommunication = NULL;
    return mitk::TrackingSystemNotSpecified;
  }

  /* Reset Tracking device by sending a serial break for 500ms */
  m_SerialCommunication->SendBreak(400);

  /* Read answer from tracking device (RESETBE6F) */
  static const std::string reset("RESETBE6F\r");
  std::string answer = "";
  this->Receive(&answer, reset.length());  // read answer (should be RESETBE6F)
  this->ClearReceiveBuffer();     // flush the receive buffer of all remaining data (carriage return, strings other than reset
  if (reset.compare(answer) != 0)  // check for RESETBE6F
  {
    this->SetErrorMessage("Hardware Reset of tracking device did not work");
    m_SerialCommunication->CloseConnection();
    m_SerialCommunication = NULL;
    return mitk::TrackingSystemNotSpecified;
  }

  /* Now the tracking device is reset, start initialization */
  NDIErrorCode returnvalue;

  /* initialize the tracking device */
  //returnvalue = m_DeviceProtocol->INIT();
  //if (returnvalue != NDIOKAY)
  //{
  //  this->SetErrorMessage("Could not initialize the tracking device");
  //  return mitk::TrackingSystemNotSpecified;
  //}

  
    mitk::TrackingDeviceType deviceType;
    returnvalue = m_DeviceProtocol->VER(deviceType);
    if ((returnvalue != NDIOKAY) || (deviceType == mitk::TrackingSystemNotSpecified))
    {
      m_SerialCommunication = NULL;
      return mitk::TrackingSystemNotSpecified;
    }
    m_SerialCommunication = NULL;
    return deviceType;
}


bool mitk::NDITrackingDevice::CloseConnection()
{
  if (this->GetMode() != Setup)
  {
    //init before closing to force the field generator from aurora to switch itself off
    m_DeviceProtocol->INIT();
    /* close the serial connection */
    m_SerialCommunication->CloseConnection();
    /* invalidate all tools */
    this->InvalidateAll();
    /* return to setup mode */
    this->SetMode(Setup);
    this->SetErrorMessage("");
    m_SerialCommunication = NULL;
  }
  return true;
}


ITK_THREAD_RETURN_TYPE mitk::NDITrackingDevice::ThreadStartTracking(void* pInfoStruct)
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
  NDITrackingDevice *trackingDevice = (NDITrackingDevice*)pInfo->UserData;
  if (trackingDevice != NULL)
  {
    if (trackingDevice->GetOperationMode() == ToolTracking6D)
      trackingDevice->TrackTools();             // call TrackTools() from the original object
    else if (trackingDevice->GetOperationMode() == MarkerTracking3D)
      trackingDevice->TrackMarkerPositions();   // call TrackMarkerPositions() from the original object
    else if (trackingDevice->GetOperationMode() == ToolTracking5D)
      trackingDevice->TrackMarkerPositions(); // call TrackMarkerPositions() from the original object
    else if (trackingDevice->GetOperationMode() == HybridTracking)
    {
      trackingDevice->TrackToolsAndMarkers();
    }
  }
  trackingDevice->m_ThreadID = 0;  // erase thread id, now that this thread will end.
  return ITK_THREAD_RETURN_VALUE;
}


bool mitk::NDITrackingDevice::StartTracking()
{
  if (this->GetMode() != Ready)
    return false;

  this->SetMode(Tracking);      // go to mode Tracking
  this->m_StopTrackingMutex->Lock();  // update the local copy of m_StopTracking
  this->m_StopTracking = false;
  this->m_StopTrackingMutex->Unlock();

  m_TrackingFinishedMutex->Unlock(); // transfer the execution rights to tracking thread

  m_ThreadID = m_MultiThreader->SpawnThread(this->ThreadStartTracking, this);    // start a new thread that executes the TrackTools() method
  mitk::TimeStamp::GetInstance()->Start(this);
  return true;
}


void mitk::NDITrackingDevice::TrackTools()
{
  if (this->GetMode() != Tracking)
    return;

  NDIErrorCode returnvalue;
  returnvalue = m_DeviceProtocol->TSTART();
  if (returnvalue != NDIOKAY)
    return;

  /* lock the TrackingFinishedMutex to signal that the execution rights are now transfered to the tracking thread */
  MutexLockHolder trackingFinishedLockHolder(*m_TrackingFinishedMutex); // keep lock until end of scope

  bool localStopTracking;       // Because m_StopTracking is used by two threads, access has to be guarded by a mutex. To minimize thread locking, a local copy is used here
  this->m_StopTrackingMutex->Lock();  // update the local copy of m_StopTracking
  localStopTracking = this->m_StopTracking;
  this->m_StopTrackingMutex->Unlock();
  while ((this->GetMode() == Tracking) && (localStopTracking == false))
  {
    if (this->m_DataTransferMode == TX)
    {
      returnvalue = this->m_DeviceProtocol->TX();
      if (!((returnvalue == NDIOKAY) || (returnvalue == NDICRCERROR) || (returnvalue == NDICRCDOESNOTMATCH))) // right now, do not stop on crc errors
        break;
    }
    else
    {
      returnvalue = this->m_DeviceProtocol->BX();
      if (returnvalue != NDIOKAY)
        break;
    }
    /* Update the local copy of m_StopTracking */
    this->m_StopTrackingMutex->Lock();
    localStopTracking = m_StopTracking;
    this->m_StopTrackingMutex->Unlock();
  }
  /* StopTracking was called, thus the mode should be changed back to Ready now that the tracking loop has ended. */

  returnvalue = m_DeviceProtocol->TSTOP();
  if (returnvalue != NDIOKAY)
  {
    /* insert error handling/notification here */ 
    ; // how can this thread tell the application, that an error has occurred? 
  }
  return;       // returning from this function (and ThreadStartTracking()) this will end the thread and transfer control back to main thread by releasing trackingFinishedLockHolder
}


void mitk::NDITrackingDevice::TrackMarkerPositions()
{
  if (m_OperationMode == ToolTracking6D)
    return;

  if (this->GetMode() != Tracking)
    return;

  NDIErrorCode returnvalue;

  returnvalue = m_DeviceProtocol->DSTART();   // Start Diagnostic Mode
  if (returnvalue != NDIOKAY)
    return;

  bool localStopTracking;       // Because m_StopTracking is used by two threads, access has to be guarded by a mutex. To minimize thread locking, a local copy is used here
  this->m_StopTrackingMutex->Lock();  // update the local copy of m_StopTracking
  localStopTracking = this->m_StopTracking;
  this->m_StopTrackingMutex->Unlock();
  while ((this->GetMode() == Tracking) && (localStopTracking == false))
  {
    m_MarkerPointsMutex->Lock();                                    // lock points data structure
    returnvalue = this->m_DeviceProtocol->POS3D(&m_MarkerPoints); // update points data structure with new position data from tracking device
    m_MarkerPointsMutex->Unlock();
    if (!((returnvalue == NDIOKAY) || (returnvalue == NDICRCERROR) || (returnvalue == NDICRCDOESNOTMATCH))) // right now, do not stop on crc errors
    {
      std::cout << "Error in POS3D: could not read data. Possibly no markers present." << std::endl;
    }
    /* Update the local copy of m_StopTracking */
    this->m_StopTrackingMutex->Lock();
    localStopTracking = m_StopTracking;
    this->m_StopTrackingMutex->Unlock();
  }
  /* StopTracking was called, thus the mode should be changed back to Ready now that the tracking loop has ended. */
  returnvalue = m_DeviceProtocol->DSTOP();
  if (returnvalue != NDIOKAY)
    return;     // how can this thread tell the application, that an error has occured?

  this->SetMode(Ready);
  return;       // returning from this function (and ThreadStartTracking()) this will end the thread
}


void mitk::NDITrackingDevice::TrackToolsAndMarkers()
{
  if (m_OperationMode != HybridTracking)
    return;

  NDIErrorCode returnvalue;

  returnvalue = m_DeviceProtocol->TSTART();   // Start Diagnostic Mode
  if (returnvalue != NDIOKAY)
    return;

  bool localStopTracking;       // Because m_StopTracking is used by two threads, access has to be guarded by a mutex. To minimize thread locking, a local copy is used here
  this->m_StopTrackingMutex->Lock();  // update the local copy of m_StopTracking
  localStopTracking = this->m_StopTracking;
  this->m_StopTrackingMutex->Unlock();
  while ((this->GetMode() == Tracking) && (localStopTracking == false))
  {
    m_MarkerPointsMutex->Lock();                                     // lock points data structure
    returnvalue = this->m_DeviceProtocol->TX(true, &m_MarkerPoints); // update points data structure with new position data from tracking device
    m_MarkerPointsMutex->Unlock();
    if (!((returnvalue == NDIOKAY) || (returnvalue == NDICRCERROR) || (returnvalue == NDICRCDOESNOTMATCH))) // right now, do not stop on crc errors
    {
      std::cout << "Error in TX: could not read data. Possibly no markers present." << std::endl;
    }
    /* Update the local copy of m_StopTracking */
    this->m_StopTrackingMutex->Lock();
    localStopTracking = m_StopTracking;
    this->m_StopTrackingMutex->Unlock();
  }
  /* StopTracking was called, thus the mode should be changed back to Ready now that the tracking loop has ended. */

  returnvalue = m_DeviceProtocol->TSTOP();
  if (returnvalue != NDIOKAY)
    return;     // how can this thread tell the application, that an error has occurred?

  this->SetMode(Ready);
  return;       // returning from this function (and ThreadStartTracking()) this will end the thread
}


mitk::TrackingTool* mitk::NDITrackingDevice::GetTool(unsigned int toolNumber) const
{
  MutexLockHolder toolsMutexLockHolder(*m_ToolsMutex); // lock and unlock the mutex
  if (toolNumber < m_6DTools.size())
    return m_6DTools.at(toolNumber);
  return NULL;
}


mitk::TrackingTool* mitk::NDITrackingDevice::GetToolByName(std::string name) const
{
  MutexLockHolder toolsMutexLockHolder(*m_ToolsMutex); // lock and unlock the mutex
  Tool6DContainerType::const_iterator end = m_6DTools.end();
  for (Tool6DContainerType::const_iterator iterator = m_6DTools.begin(); iterator != end; ++iterator)
    if (name.compare((*iterator)->GetToolName()) == 0)
      return *iterator;
  return NULL;
}


mitk::NDIPassiveTool* mitk::NDITrackingDevice::GetInternalTool(std::string portHandle)
{
  MutexLockHolder toolsMutexLockHolder(*m_ToolsMutex); // lock and unlock the mutex
  Tool6DContainerType::iterator end = m_6DTools.end();
  for (Tool6DContainerType::iterator iterator = m_6DTools.begin(); iterator != end; ++iterator)
    if (portHandle.compare((*iterator)->GetPortHandle()) == 0)
      return *iterator;
  return NULL;
}


unsigned int mitk::NDITrackingDevice::GetToolCount() const
{
  MutexLockHolder toolsMutexLockHolder(*m_ToolsMutex); // lock and unlock the mutex
  return m_6DTools.size();
}


bool mitk::NDITrackingDevice::Beep(unsigned char count)
{
  if (this->GetMode() != Setup)
  {
    return (m_DeviceProtocol->BEEP(count) == NDIOKAY);
  }
  else
  {
    return false;
  }
}

mitk::TrackingTool* mitk::NDITrackingDevice::AddTool( const char* toolName, const char* fileName, TrackingPriority p /*= NDIPassiveTool::Dynamic*/ )
{
  mitk::NDIPassiveTool::Pointer t = mitk::NDIPassiveTool::New();
  if (t->LoadSROMFile(fileName) == false)
    return NULL;
  t->SetToolName(toolName);
  t->SetTrackingPriority(p);
  if (this->InternalAddTool(t) == false)
    return NULL;
  return t.GetPointer();
}


bool mitk::NDITrackingDevice::InternalAddTool(mitk::NDIPassiveTool* tool)
{
  if (tool == NULL)
    return false;
  NDIPassiveTool::Pointer p = tool;
  /* if the connection to the tracking device is already established, add the new tool to the device now */
  if (this->GetMode() == Ready)
  {
    /* get a port handle for the tool */
    std::string newPortHandle;
    NDIErrorCode returnvalue;
    returnvalue = m_DeviceProtocol->PHRQ(&newPortHandle);
    if (returnvalue == NDIOKAY)
    {
      p->SetPortHandle(newPortHandle.c_str());
      /* now write the SROM file of the tool to the tracking system using PVWR */
      returnvalue = m_DeviceProtocol->PVWR(&newPortHandle, p->GetSROMData(), p->GetSROMDataLength());
      if (returnvalue != NDIOKAY)
      {
        this->SetErrorMessage((std::string("Could not write SROM file for tool '") + p->GetToolName() + std::string("' to tracking device")).c_str());
        return false;
      }
      /* initialize the port handle */
      returnvalue = m_DeviceProtocol->PINIT(&newPortHandle);
      if (returnvalue != NDIOKAY)
      {
        this->SetErrorMessage((std::string("Could not initialize port '") + newPortHandle +
          std::string("' for tool '")+ p->GetToolName() + std::string("'")).c_str());
        return false;
      }
      /* enable the port handle */
      if (p->IsEnabled() == true)
      {
        returnvalue = m_DeviceProtocol->PENA(&newPortHandle, p->GetTrackingPriority()); // Enable tool
        if (returnvalue != NDIOKAY)
        {
          this->SetErrorMessage((std::string("Could not enable port '") + newPortHandle +
            std::string("' for tool '")+ p->GetToolName() + std::string("'")).c_str());
          return false;
        }
      }
    }
    /* now that the tool is added to the device, add it to list too */
    m_ToolsMutex->Lock();
    this->m_6DTools.push_back(p);
    m_ToolsMutex->Unlock();
    this->Modified();
    return true;
  }
  else if (this->GetMode() == Setup)
  {
    /* In Setup mode, we only add it to the list, so that OpenConnection() can add it later */
    m_ToolsMutex->Lock();
    this->m_6DTools.push_back(p);
    m_ToolsMutex->Unlock();
    this->Modified();
    return true;
  }
  else  // in Tracking mode, no tools can be added
    return false;
}


bool mitk::NDITrackingDevice::RemoveTool(mitk::TrackingTool* tool)
{
  mitk::NDIPassiveTool* ndiTool = dynamic_cast<mitk::NDIPassiveTool*>(tool);
  if (ndiTool == NULL)
    return false;

  std::string portHandle = ndiTool->GetPortHandle();
  /* a valid portHandle has length 2. If a valid handle exists, the tool is already added to the tracking device, so we have to remove it there
  if the connection to the tracking device has already been established.
  */
  if ((portHandle.length() == 2) && (this->GetMode() == Ready))  // do not remove a tool in tracking mode
  {
    NDIErrorCode returnvalue;
    returnvalue = m_DeviceProtocol->PHF(&portHandle);
    if (returnvalue != NDIOKAY)
      return false;
    /* Now that the tool is removed from the tracking device, remove it from our tool list too */
    MutexLockHolder toolsMutexLockHolder(*m_ToolsMutex); // lock and unlock the mutex (scope is inside the if-block
    Tool6DContainerType::iterator end = m_6DTools.end();
    for (Tool6DContainerType::iterator iterator = m_6DTools.begin(); iterator != end; ++iterator)
    {
      if (iterator->GetPointer() == ndiTool)
      {
        m_6DTools.erase(iterator);
        this->Modified();
        return true;
      }
    }
    return false;
  }
  else if (this->GetMode() == Setup)  // in Setup Mode, we are not connected to the tracking device, so we can just remove the tool from the tool list
  {
    MutexLockHolder toolsMutexLockHolder(*m_ToolsMutex); // lock and unlock the mutex
    Tool6DContainerType::iterator end = m_6DTools.end();
    for (Tool6DContainerType::iterator iterator = m_6DTools.begin(); iterator != end; ++iterator)
    {
      if ((*iterator).GetPointer() == ndiTool)
      {
        m_6DTools.erase(iterator);
        this->Modified();
        return true;
      }
    }
    return false;
  }
  return false;
}


void mitk::NDITrackingDevice::InvalidateAll()
{
  MutexLockHolder toolsMutexLockHolder(*m_ToolsMutex); // lock and unlock the mutex
  Tool6DContainerType::iterator end = m_6DTools.end();
  for (Tool6DContainerType::iterator iterator = m_6DTools.begin(); iterator != end; ++iterator)
    (*iterator)->SetDataValid(false);
}


bool mitk::NDITrackingDevice::SetOperationMode(OperationMode mode)
{
  if (GetMode() == Tracking)
    return false;

  m_OperationMode = mode;
  return true;
}


mitk::OperationMode mitk::NDITrackingDevice::GetOperationMode()
{
  return m_OperationMode;
}


bool mitk::NDITrackingDevice::GetMarkerPositions(MarkerPointContainerType* markerpositions)
{
  m_MarkerPointsMutex->Lock();
  *markerpositions = m_MarkerPoints;  // copy the internal vector to the one provided
  m_MarkerPointsMutex->Unlock();
  return (markerpositions->size() != 0)  ;
}


bool mitk::NDITrackingDevice::DiscoverWiredTools()
{
  /* First, check for disconnected tools and remove them */
  this->FreePortHandles();
  
  /* check for new tools, add and initialize them */
  NDIErrorCode returnvalue;
  std::string portHandle;
  returnvalue = m_DeviceProtocol->PHSR(OCCUPIED, &portHandle);

  if (returnvalue != NDIOKAY)
  {
    this->SetErrorMessage("Could not obtain a list of port handles that are connected");
    return false;     // ToDo: Is this a fatal error?
  }

  /* if there are port handles that need to be initialized, initialize them. Furthermore instantiate tools for each handle that has no tool yet. */
  std::string ph;
  
  /* we need to rember the ports which are occupied to be able to readout the serial numbers of the connected tools later*/
  std::vector<int> occupiedPorts;

  for (unsigned int i = 0; i < portHandle.size(); i += 2)
  {
    ph = portHandle.substr(i, 2);
    if (this->GetInternalTool(ph) != NULL) // if we already have a tool with this handle
      continue;                            // then skip the initialization
    
    //instantiate an object for each tool that is connected
    mitk::NDIPassiveTool::Pointer newTool = mitk::NDIPassiveTool::New();
    newTool->SetPortHandle(ph.c_str());
    newTool->SetTrackingPriority(mitk::NDIPassiveTool::Dynamic);

    
    
    //set a name for identification
    newTool->SetToolName((std::string("Port ") + ph).c_str());

    returnvalue = m_DeviceProtocol->PINIT(&ph);
    if (returnvalue != NDIINITIALIZATIONFAILED) //if the initialization failed (AURORA) it can not be enabled. A srom file will have to be specified manually first. Still return true to be able to continue
    {
      if (returnvalue != NDIOKAY)
      {
        this->SetErrorMessage((std::string("Could not initialize port '") + ph +
          std::string("' for tool '")+ newTool->GetToolName() + std::string("'")).c_str());
        return false;
      }
      /* enable the port handle */
      returnvalue = m_DeviceProtocol->PENA(&ph, newTool->GetTrackingPriority()); // Enable tool
      if (returnvalue != NDIOKAY)
      {
        this->SetErrorMessage((std::string("Could not enable port '") + ph +
          std::string("' for tool '")+ newTool->GetToolName() + std::string("'")).c_str());
        return false;
      }
    }
    //we have to temporarily unlock m_ModeMutex here to avoid a deadlock with another lock inside InternalAddTool() 
    if (this->InternalAddTool(newTool) == false)
      this->SetErrorMessage("Error while adding new tool");
    else occupiedPorts.push_back(i);
  }
  
  
  //after initialization readout serial numbers of tools
  for (unsigned int i = 0; i < this->GetToolCount(); i += 2)
    {
    ph = portHandle.substr(occupiedPorts.at(i), 2);
    std::string portInfo;
    NDIErrorCode returnvaluePort = m_DeviceProtocol->PHINF(ph, &portInfo);
    if ((returnvaluePort==NDIOKAY) && (portInfo.size()>31)) dynamic_cast<mitk::NDIPassiveTool*>(this->GetTool(i))->SetSerialNumber(portInfo.substr(23,8));
    }

  return true;
}


mitk::NDIErrorCode mitk::NDITrackingDevice::FreePortHandles()
{
  /*  first search for port handles that need to be freed: e.g. because of a reset of the tracking system */
  NDIErrorCode returnvalue = NDIOKAY;
  std::string portHandle;
  returnvalue = m_DeviceProtocol->PHSR(FREED, &portHandle);
  if (returnvalue != NDIOKAY)
  {
    this->SetErrorMessage("Could not obtain a list of port handles that need to be freed");
    return returnvalue;     // ToDo: Is this a fatal error?
  }

  /* if there are port handles that need to be freed, free them */
  if (portHandle.empty() == true)
    return returnvalue;

  std::string ph;
  for (unsigned int i = 0; i < portHandle.size(); i += 2)
  {
    ph = portHandle.substr(i, 2);

    mitk::NDIPassiveTool* t = this->GetInternalTool(ph);
    if (t != NULL)  // if we have a tool for the port handle that needs to be freed
    {
      if (this->RemoveTool(t) == false)  // remove it (this will free the port too)
        returnvalue = NDIERROR;
    }
    else  // we don't have a tool, the port handle exists only in the tracking device
    {
      returnvalue = m_DeviceProtocol->PHF(&ph);  // free it there
      // What to do if port handle could not be freed? This seems to be a non critical error
      if (returnvalue != NDIOKAY)
      {
        this->SetErrorMessage("Could not free all port handles");
        //        return false; // could not free all Handles
      }
    }
  }
  return returnvalue;
}
