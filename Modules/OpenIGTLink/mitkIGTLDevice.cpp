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

#include "mitkIGTLDevice.h"
//#include "mitkIGTTimeStamp.h"
#include <itkMutexLockHolder.h>
#include <itksys/SystemTools.hxx>

typedef itk::MutexLockHolder<itk::FastMutexLock> MutexLockHolder;


mitk::IGTLDevice::IGTLDevice() :
//  m_Data(mitk::DeviceDataUnspecified),
  m_State(mitk::IGTLDevice::Setup),
  m_StopCommunication(false),
  m_PortNumber(-1),
  m_MultiThreader(NULL), m_ThreadID(0)

{
  m_StopCommunicationMutex = itk::FastMutexLock::New();
  m_StateMutex = itk::FastMutexLock::New();
  m_CommunicationFinishedMutex = itk::FastMutexLock::New();
  // execution rights are owned by the application thread at the beginning
  m_CommunicationFinishedMutex->Lock();
  m_MultiThreader = itk::MultiThreader::New();
//  m_Data = mitk::DeviceDataUnspecified;
}


mitk::IGTLDevice::~IGTLDevice()
{
  /* stop communication and disconnect from igtl device */
  if (GetState() == Running)
  {
    this->StopCommunication();
  }
  if (GetState() == Ready)
  {
    this->CloseConnection();
  }
  /* cleanup tracking thread */
  if ((m_ThreadID != 0) && (m_MultiThreader.IsNotNull()))
  {
    m_MultiThreader->TerminateThread(m_ThreadID);
  }
  m_MultiThreader = NULL;
}

mitk::IGTLDevice::IGTLDeviceState mitk::IGTLDevice::GetState() const
{
  MutexLockHolder lock(*m_StateMutex);
  return m_State;
}


void mitk::IGTLDevice::SetState( IGTLDeviceState state )
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

//mitk::IGTLDeviceData mitk::IGTLDevice::GetData() const{
//  return m_Data;
//}


//void mitk::IGTLDevice::SetData(mitk::IGTLDeviceData data){
//  m_Data = data;
//}


bool mitk::IGTLDevice::StopCommunication()
{
  if (this->GetState() == Running) // Only if the object is in the correct state
  {
    // m_StopCommunication is used by two threads, so we have to ensure correct
    // thread handling
    m_StopCommunicationMutex->Lock();
    m_StopCommunication = true;
    m_StopCommunicationMutex->Unlock();
    // we have to wait here that the other thread recognizes the STOP-command
    // and executes it
    m_CommunicationFinishedMutex->Lock();
//    mitk::IGTTimeStamp::GetInstance()->Stop(this); // notify realtime clock
    // StopTracking was called, thus the mode should be changed back
    // to Ready now that the tracking loop has ended.
    this->SetState(Ready);
  }
  return true;
}


bool mitk::IGTLDevice::SendMessage(igtl::MessageBase::Pointer msg)
{
//  if (input == NULL)
//    return SERIALSENDERROR;

//  std::string message;

//  if (addCRC == true)
//    message = *input + CalcCRC(input) + std::string(1, CR);
//  else
//    message = *input + std::string(1, CR);

//  //unsigned int messageLength = message.length() + 1; // +1 for CR

//  // Clear send buffer
//  this->ClearSendBuffer();
//  // Send the date to the device
//  MutexLockHolder lock(*m_SerialCommunicationMutex); // lock and unlock the mutex
//  long returnvalue = m_SerialCommunication->Send(message);

//  if (returnvalue == 0)
//    return SERIALSENDERROR;
//  else
//    return NDIOKAY;
  return true;
}

//mitk::NDIErrorCode mitk::IGTLDevice::Receive(std::string* answer, unsigned int numberOfBytes)
//{
//  if (answer == NULL)
//    return SERIALRECEIVEERROR;

//  MutexLockHolder lock(*m_SerialCommunicationMutex); // lock and unlock the mutex
//  long returnvalue = m_SerialCommunication->Receive(*answer, numberOfBytes);  // never read more bytes than the device has send, the function will block until enough bytes are send...

//  if (returnvalue == 0)
//    return SERIALRECEIVEERROR;
//  else
//    return NDIOKAY;
//}


bool mitk::IGTLDevice::TestConnection()
{
//  if (this->GetState() != Setup)
//  {
//    return mitk::TrackingSystemNotSpecified;
//  }

//  m_SerialCommunication = mitk::SerialCommunication::New();
//  //m_DeviceProtocol =  mitk::NDIProtocol::New();
//  //m_DeviceProtocol->SetTrackingDevice(this);
//  //m_DeviceProtocol->UseCRCOn();
//  /* init local com port to standard com settings for a NDI tracking device:
//  9600 baud, 8 data bits, no parity, 1 stop bit, no hardware handshake
//  */
//  if (m_DeviceName.empty())
//    m_SerialCommunication->SetPortNumber(m_PortNumber);
//  else
//    m_SerialCommunication->SetDeviceName(m_DeviceName);

//  m_SerialCommunication->SetBaudRate(mitk::SerialCommunication::BaudRate9600);
//  m_SerialCommunication->SetDataBits(mitk::SerialCommunication::DataBits8);
//  m_SerialCommunication->SetParity(mitk::SerialCommunication::None);
//  m_SerialCommunication->SetStopBits(mitk::SerialCommunication::StopBits1);
//  m_SerialCommunication->SetSendTimeout(5000);
//  m_SerialCommunication->SetReceiveTimeout(5000);
//  if (m_SerialCommunication->OpenConnection() == 0) // error
//  {
//    m_SerialCommunication = NULL;
//    return mitk::TrackingSystemNotSpecified;
//  }

//  /* Reset Tracking device by sending a serial break for 500ms */
//  m_SerialCommunication->SendBreak(400);

//  /* Read answer from tracking device (RESETBE6F) */
//  static const std::string reset("RESETBE6F\r");
//  std::string answer = "";
//  this->Receive(&answer, reset.length());  // read answer (should be RESETBE6F)
//  this->ClearReceiveBuffer();     // flush the receive buffer of all remaining data (carriage return, strings other than reset
//  if (reset.compare(answer) != 0)  // check for RESETBE6F
//  {
//    m_SerialCommunication->CloseConnection();
//    m_SerialCommunication = NULL;
//    mitkThrowException(mitk::IGTHardwareException) << "Hardware Reset of tracking device did not work";
//  }

//  /* Now the tracking device is reset, start initialization */
//  NDIErrorCode returnvalue;

//  /* initialize the tracking device */
//  //returnvalue = m_DeviceProtocol->INIT();
//  //if (returnvalue != NDIOKAY)
//  //{
//  //  this->SetErrorMessage("Could not initialize the tracking device");
//  //  return mitk::TrackingSystemNotSpecified;
//  //}


//    mitk::TrackingDeviceType deviceType;
//    returnvalue = m_DeviceProtocol->VER(deviceType);
//    if ((returnvalue != NDIOKAY) || (deviceType == mitk::TrackingSystemNotSpecified))
//    {
//      m_SerialCommunication = NULL;
//      return mitk::TrackingSystemNotSpecified;
//    }
//    m_SerialCommunication = NULL;
//    return deviceType;
  return true;
}


bool mitk::IGTLDevice::CloseConnection()
{
  if (this->GetState() != Setup)
  {
//    //init before closing to force the field generator from aurora to switch itself off
//    m_DeviceProtocol->INIT();
//    /* close the serial connection */
//    m_SerialCommunication->CloseConnection();
//    /* invalidate all tools */
//    this->InvalidateAll();
//    /* return to setup mode */
//    this->SetState(Setup);
//    m_SerialCommunication = NULL;
  }
  return true;
}

ITK_THREAD_RETURN_TYPE mitk::IGTLDevice::ThreadStartCommunication(void* pInfoStruct)
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
  IGTLDevice *igtlDevice = (IGTLDevice*)pInfo->UserData;
  if (igtlDevice != NULL)
  {
    igtlDevice->RunCommunication();
  }
  igtlDevice->m_ThreadID = 0;  // erase thread id because thread will end.
  return ITK_THREAD_RETURN_VALUE;
}

void mitk::IGTLDevice::RunCommunication()
{
  if (this->GetState() != Running)
    return;

  // keep lock until end of scope
  MutexLockHolder communicationFinishedLockHolder(*m_CommunicationFinishedMutex);

  // Because m_StopCommunication is used by two threads, access has to be guarded by
  // a mutex. To minimize thread locking, a local copy is used here
  bool localStopCommunication;

  // update the local copy of m_StopCommunication
  this->m_StopCommunicationMutex->Lock();
  localStopCommunication = this->m_StopCommunication;
  this->m_StopCommunicationMutex->Unlock();
  while ((this->GetState() == Running) && (localStopCommunication == false))
  {
    //POLLLING

//    m_MarkerPointsMutex->Lock(); // lock points data structure
//    returnvalue = this->m_DeviceProtocol->POS3D(&m_MarkerPoints); // update points data structure with new position data from tracking device
//    m_MarkerPointsMutex->Unlock();
//    if (!((returnvalue == NDIOKAY) || (returnvalue == NDICRCERROR) || (returnvalue == NDICRCDOESNOTMATCH))) // right now, do not stop on crc errors
//    {
//      std::cout << "Error in POS3D: could not read data. Possibly no markers present." << std::endl;
//    }
    /* Update the local copy of m_StopCommunication */
    this->m_StopCommunicationMutex->Lock();
    localStopCommunication = m_StopCommunication;
    this->m_StopCommunicationMutex->Unlock();

    itksys::SystemTools::Delay(1);
  }
  // StopTracking was called, thus the mode should be changed back to Ready now
  // that the tracking loop has ended.
  this->SetState(Ready);
  // returning from this function (and ThreadStartCommunication())
  // this will end the thread
  return;
}




bool mitk::IGTLDevice::StartCommunication()
{
  if (this->GetState() != Ready)
    return false;

  this->SetState(Running);      // go to mode Tracking
  // update the local copy of m_StopCommunication
  this->m_StopCommunicationMutex->Lock();
  this->m_StopCommunication = false;
  this->m_StopCommunicationMutex->Unlock();

  // transfer the execution rights to tracking thread
  m_CommunicationFinishedMutex->Unlock();

  // start a new thread that executes the TrackTools() method
  m_ThreadID =
    m_MultiThreader->SpawnThread(this->ThreadStartCommunication, this);
//  mitk::IGTTimeStamp::GetInstance()->Start(this);
  return true;
}
