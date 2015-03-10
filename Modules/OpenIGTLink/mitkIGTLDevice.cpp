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
#include <cstring>

#include <igtlTransformMessage.h>
#include <mitkIGTLMessageCommon.h>

#include <igtl_status.h>

//remove later
#include <igtlTrackingDataMessage.h>

#include <igtlTrackingDataMessage.h>

static const int SOCKET_SEND_RECEIVE_TIMEOUT_MSEC = 1;

typedef itk::MutexLockHolder<itk::FastMutexLock> MutexLockHolder;

mitk::IGTLDevice::IGTLDevice() :
//  m_Data(mitk::DeviceDataUnspecified),
  m_State(mitk::IGTLDevice::Setup),
  m_Name("Unspecified Device"),
  m_StopCommunication(false),
  m_PortNumber(-1),
  m_MultiThreader(NULL), m_ThreadID(0)
{
  m_StopCommunicationMutex = itk::FastMutexLock::New();
  m_StateMutex = itk::FastMutexLock::New();
//  m_LatestMessageMutex = itk::FastMutexLock::New();
  m_CommunicationFinishedMutex = itk::FastMutexLock::New();
  // execution rights are owned by the application thread at the beginning
  m_CommunicationFinishedMutex->Lock();
  m_MultiThreader = itk::MultiThreader::New();
//  m_Data = mitk::DeviceDataUnspecified;
//  m_LatestMessage = igtl::MessageBase::New();

  m_MessageFactory = mitk::IGTLMessageFactory::New();
  m_SendQueue      = mitk::IGTLMessageQueue::New();
  m_ReceiveQueue   = mitk::IGTLMessageQueue::New();
  m_CommandQueue   = mitk::IGTLMessageQueue::New();
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

  m_StateMutex->Lock();
//  MutexLockHolder lock(*m_StateMutex); // lock and unlock the mutex

  if (m_State == state)
  {
    m_StateMutex->Unlock();
    return;
  }
  m_State = state;
  m_StateMutex->Unlock();
  this->Modified();
}


bool mitk::IGTLDevice::TestConnection()
{

  return true;
}

unsigned int mitk::IGTLDevice::ReceivePrivate(igtl::Socket* socket)
{
  // Create a message buffer to receive header
  igtl::MessageHeader::Pointer headerMsg;
  headerMsg = igtl::MessageHeader::New();

  // Initialize receive buffer
  headerMsg->InitPack();

  // Receive generic header from the socket
  int r =
    socket->Receive(headerMsg->GetPackPointer(), headerMsg->GetPackSize(), 1);

  if (r == 0) //connection error
  {
    // an error was received, therefor the communication with this socket
    // must be stopped
    return IGTL_STATUS_NOT_PRESENT;
  }
  else if (r == -1 ) //timeout
  {
    // a timeout was received, this is no error state, thus, do nothing
    return IGTL_STATUS_TIME_OUT;
  }
  else if (r == headerMsg->GetPackSize())
  {
    // Deserialize the header and check the CRC
    int crcCheck = headerMsg->Unpack(1);
    if (crcCheck & igtl::MessageHeader::UNPACK_HEADER)
    {
      // Allocate a time stamp
      igtl::TimeStamp::Pointer ts;
      ts = igtl::TimeStamp::New();

      // Get time stamp
      igtlUint32 sec;
      igtlUint32 nanosec;

      headerMsg->GetTimeStamp(ts);
      ts->GetTimeStamp(&sec, &nanosec);

//      std::cerr << "Time stamp: "
//                << sec << "."
//                << nanosec << std::endl;

//      std::cerr << "Dev type and name: " << headerMsg->GetDeviceType() << " "
//                << headerMsg->GetDeviceName() << std::endl;

//      headerMsg->Print(std::cout);

      //check the type of the received message
      //if it is a GET_, STP_ or RTS_ command push it into the command queue
      //otherwise continue reading the whole message from the socket
      const char* curDevType = headerMsg->GetDeviceType();
      if ( std::strstr( curDevType, "GET_" ) != NULL ||
           std::strstr( curDevType, "STP_" ) != NULL ||
           std::strstr( curDevType, "RTS_" ) != NULL)
      {
        this->m_CommandQueue->PushMessage(headerMsg);
        this->InvokeEvent(CommandReceivedEvent());
        return IGTL_STATUS_OK;
      }

      //Create a message according to the header message
      igtl::MessageBase::Pointer curMessage;
      curMessage = m_MessageFactory->CreateInstance(headerMsg);

      //check if the curMessage is created properly, if not the message type is
      //not supported and the message has to be skipped
      if ( curMessage.IsNull() )
      {
        socket->Skip(headerMsg->GetBodySizeToRead(), 0);
        MITK_ERROR("IGTLDevice") << "The received type is not supported. Please "
                                    "add it to the message factory.";
        return IGTL_STATUS_NOT_FOUND;
      }

      //insert the header to the message and allocate the pack
      curMessage->SetMessageHeader(headerMsg);
      curMessage->AllocatePack();

      // Receive transform data from the socket
      int receiveCheck = 0;
      receiveCheck = socket->Receive(curMessage->GetPackBodyPointer(),
                                     curMessage->GetPackBodySize());

      if ( receiveCheck > 0 )
      {
        int c = curMessage->Unpack(1);
        if ( !(c & igtl::MessageHeader::UNPACK_BODY) )
        {
//            mitkThrow() << "crc error";
          return IGTL_STATUS_CHECKSUM_ERROR;
        }

        //check the type of the received message
        //if it is a command push it into the command queue
        //otherwise into the normal receive queue
        //STP_ commands are handled here because they implemented additional
        //member variables that are not stored in the header message
        if ( std::strstr( curDevType, "STT_" ) != NULL )
        {
          this->m_CommandQueue->PushMessage(curMessage);
          this->InvokeEvent(CommandReceivedEvent());
        }
        else
        {
          this->m_ReceiveQueue->PushMessage(curMessage);
          this->InvokeEvent(MessageReceivedEvent());
        }
        return IGTL_STATUS_OK;
      }
      else
      {
        MITK_ERROR("IGTLDevice") << "Received a valid header but could not "
                                 << "read the whole message.";
        return IGTL_STATUS_UNKNOWN_ERROR;
      }
    }
    else
    {
      //CRC check failed
      return IGTL_STATUS_CHECKSUM_ERROR;
    }
  }
  else
  {
    //Message size information and actual data size don't match.
    //this state is not suppossed to be reached, return unknown error
    return IGTL_STATUS_UNKNOWN_ERROR;
  }
}

void mitk::IGTLDevice::SendMessage(const mitk::IGTLMessage* msg)
{
  this->SendMessage(msg->GetMessage());
}

void mitk::IGTLDevice::SendMessage(igtl::MessageBase::Pointer msg)
{
  //add the message to the queue
  m_SendQueue->PushMessage(msg);
}

unsigned int mitk::IGTLDevice::SendMessagePrivate(igtl::MessageBase::Pointer msg,
                                          igtl::Socket::Pointer socket)
{
  //check the input message
  if ( msg.IsNull() )
  {
    MITK_ERROR("IGTLDevice") << "Could not send message because message is not "
                                "valid. Please check.";
    return false;
  }

  // add the name of this device to the message
  msg->SetDeviceName(this->GetName().c_str());

  // Pack (serialize) and send
  msg->Pack();
  int sendSuccess = socket->Send(msg->GetPackPointer(), msg->GetPackSize());

  if (sendSuccess)
    return IGTL_STATUS_OK;
  else
    return IGTL_STATUS_UNKNOWN_ERROR;
}


void mitk::IGTLDevice::RunCommunication()
{
  if (this->GetState() != Running)
    return;

  // keep lock until end of scope
  MutexLockHolder communicationFinishedLockHolder(*m_CommunicationFinishedMutex);

  // Because m_StopCommunication is used by two threads, access has to be guarded
  // by a mutex. To minimize thread locking, a local copy is used here
  bool localStopCommunication;

  // update the local copy of m_StopCommunication
  this->m_StopCommunicationMutex->Lock();
  localStopCommunication = this->m_StopCommunication;
  this->m_StopCommunicationMutex->Unlock();
  while ((this->GetState() == Running) && (localStopCommunication == false))
  {
    // Check if other igtl devices want to connect with this one. This method
    // is overwritten for igtl servers but is doing nothing in case of a igtl
    // client
    this->Connect();

    // Check if there is something to receive and store it in the message queue
    this->Receive();

    // Check if there is something to send
    this->Send();

    /* Update the local copy of m_StopCommunication */
    this->m_StopCommunicationMutex->Lock();
    localStopCommunication = m_StopCommunication;
    this->m_StopCommunicationMutex->Unlock();

    // time to relax
//    itksys::SystemTools::Delay(1);
  }
  // StopCommunication was called, thus the mode should be changed back to Ready now
  // that the tracking loop has ended.
  this->SetState(Ready);
  MITK_DEBUG("IGTLDevice::RunCommunication") << "Reached end of communication.";
  // returning from this function (and ThreadStartCommunication())
  // this will end the thread
  return;
}




bool mitk::IGTLDevice::StartCommunication()
{
  if (this->GetState() != Ready)
    return false;

  // go to mode Running
  this->SetState(Running);

  // set a timeout for the sending and receiving
  this->m_Socket->SetTimeout(SOCKET_SEND_RECEIVE_TIMEOUT_MSEC);

  // update the local copy of m_StopCommunication
  this->m_StopCommunicationMutex->Lock();
  this->m_StopCommunication = false;
  this->m_StopCommunicationMutex->Unlock();

  // transfer the execution rights to tracking thread
  m_CommunicationFinishedMutex->Unlock();

  // start a new thread that executes the communication
  m_ThreadID =
    m_MultiThreader->SpawnThread(this->ThreadStartCommunication, this);
//  mitk::IGTTimeStamp::GetInstance()->Start(this);
  return true;
}

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
    // StopCommunication was called, thus the mode should be changed back
    // to Ready now that the tracking loop has ended.
    this->SetState(Ready);
  }
  return true;
}

bool mitk::IGTLDevice::CloseConnection()
{
  if (this->GetState() == Setup)
  {
    return true;
  }
  else if (this->GetState() == Running)
  {
    this->StopCommunication();
  }

  m_Socket->CloseSocket();

  /* return to setup mode */
  this->SetState(Setup);

//  this->InvokeEvent(mitk::LostConnectionEvent());

  return true;
}

bool mitk::IGTLDevice::SendRTSMessage(const char* type)
{
  //construct the device type for the return message, it starts with RTS_ and
  //continues with the requested type
  std::string returnType("RTS_");
  returnType.append(type);
  //create a return message
  igtl::MessageBase::Pointer rtsMsg =
      this->m_MessageFactory->CreateInstance(returnType);
  //if retMsg is NULL there is no return message defined and thus it is not
  //necessary to send one back
  if ( rtsMsg.IsNotNull() )
  {
    this->SendMessage(rtsMsg);
    return true;
  }
  else
  {
    return false;
  }
}

void mitk::IGTLDevice::Connect()
{

}

igtl::MessageBase::Pointer mitk::IGTLDevice::GetNextMessage()
{
  //copy the next message into the given msg
  igtl::MessageBase::Pointer msg = this->m_ReceiveQueue->PullMessage();
  return msg;
}

igtl::MessageBase::Pointer mitk::IGTLDevice::GetNextCommand()
{
  //copy the next command into the given msg
  igtl::MessageBase::Pointer msg = this->m_CommandQueue->PullMessage();
  return msg;
}

void mitk::IGTLDevice::EnableInfiniteBufferingMode(
                                          mitk::IGTLMessageQueue::Pointer queue,
                                          bool enable)
{
  queue->EnableInfiniteBuffering(enable);
}

//std::string mitk::IGTLDevice::GetNextMessageInformation()
//{
//  return this->m_ReceiveQueue->GetNextMsgInformationString();
//}

//std::string mitk::IGTLDevice::GetNextMessageDeviceType()
//{
//  return this->m_ReceiveQueue->GetNextMsgDeviceType();
//}

//std::string mitk::IGTLDevice::GetNextCommandInformation()
//{
//  return this->m_CommandQueue->GetNextMsgInformationString();
//}

//std::string mitk::IGTLDevice::GetNextCommandDeviceType()
//{
//  return this->m_CommandQueue->GetNextMsgDeviceType();
//}

ITK_THREAD_RETURN_TYPE mitk::IGTLDevice::ThreadStartCommunication(void* pInfoStruct)
{
  /* extract this pointer from Thread Info structure */
  struct itk::MultiThreader::ThreadInfoStruct * pInfo =
    (struct itk::MultiThreader::ThreadInfoStruct*)pInfoStruct;
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
