/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkIGTLDevice.h"
//#include "mitkIGTException.h"
//#include "mitkIGTTimeStamp.h"
#include <itkMutexLockHolder.h>
#include <itksys/SystemTools.hxx>
#include <cstring>

#include <igtlTransformMessage.h>
#include <mitkIGTLMessageCommon.h>

#include <igtl_status.h>

//remove later
#include <igtlTrackingDataMessage.h>

//TODO: Which timeout is acceptable and also needed to transmit image data? Is there a maximum data limit?
static const int SOCKET_SEND_RECEIVE_TIMEOUT_MSEC = 100;
typedef itk::MutexLockHolder<itk::FastMutexLock> MutexLockHolder;

mitk::IGTLDevice::IGTLDevice(bool ReadFully) :
//  m_Data(mitk::DeviceDataUnspecified),
m_State(mitk::IGTLDevice::Setup),
m_Name("Unspecified Device"),
m_StopCommunication(false),
m_Hostname("127.0.0.1"),
m_PortNumber(-1),
m_LogMessages(false),
m_MultiThreader(nullptr), m_SendThreadID(0), m_ReceiveThreadID(0), m_ConnectThreadID(0)
{
  m_ReadFully = ReadFully;
  m_StopCommunicationMutex = itk::FastMutexLock::New();
  m_StateMutex = itk::FastMutexLock::New();
  //  m_LatestMessageMutex = itk::FastMutexLock::New();
  m_SendingFinishedMutex = itk::FastMutexLock::New();
  m_ReceivingFinishedMutex = itk::FastMutexLock::New();
  m_ConnectingFinishedMutex = itk::FastMutexLock::New();
  // execution rights are owned by the application thread at the beginning
  m_SendingFinishedMutex->Lock();
  m_ReceivingFinishedMutex->Lock();
  m_ConnectingFinishedMutex->Lock();
  m_MultiThreader = itk::MultiThreader::New();
  //  m_Data = mitk::DeviceDataUnspecified;
  //  m_LatestMessage = igtl::MessageBase::New();

  m_MessageFactory = mitk::IGTLMessageFactory::New();
  m_MessageQueue = mitk::IGTLMessageQueue::New();
}

mitk::IGTLDevice::~IGTLDevice()
{
  /* stop communication and disconnect from igtl device */
  if (GetState() == Running)
  {
    this->StopCommunication();
    this->CloseConnection();
  }
  else if (GetState() == Ready)
  {
    this->CloseConnection();
  }
  /* cleanup tracking thread */
  if (m_MultiThreader.IsNotNull())
  {
    if ((m_SendThreadID != 0))
    {
      m_MultiThreader->TerminateThread(m_SendThreadID);
    }
    if ((m_ReceiveThreadID != 0))
    {
      m_MultiThreader->TerminateThread(m_ReceiveThreadID);
    }
    if ((m_ConnectThreadID != 0))
    {
      m_MultiThreader->TerminateThread(m_ConnectThreadID);
    }
  }
  m_MultiThreader = nullptr;
}

mitk::IGTLDevice::IGTLDeviceState mitk::IGTLDevice::GetState() const
{
  MutexLockHolder lock(*m_StateMutex);
  return m_State;
}

void mitk::IGTLDevice::SetState(IGTLDeviceState state)
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
    socket->Receive(headerMsg->GetPackPointer(), headerMsg->GetPackSize(), 0);

  //MITK_INFO << "Server received r = " << r;

  //MITK_INFO << "Received r = " << r;

  if (r == 0) //connection error
  {
    // an error was received, therefore the communication with this socket
    // must be stoppedy
    return IGTL_STATUS_NOT_PRESENT;
  }
  else if (r == -1) //timeout
  {
    // a timeout was received, this is no error state, thus, do nothing
    return IGTL_STATUS_TIME_OUT;
  }
  else if (r == headerMsg->GetPackSize())
  {
    // Deserialize the header and check the CRC
    // ERROR HERE: This probably means the header data is corrupted...
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

      // headerMsg->Print(std::cout);

      //check the type of the received message
      //if it is a GET_, STP_ or RTS_ command push it into the command queue
      //otherwise continue reading the whole message from the socket
      const char* curDevType = headerMsg->GetDeviceType();
      if (std::strstr(curDevType, "GET_") != nullptr ||
        std::strstr(curDevType, "STP_") != nullptr ||
        std::strstr(curDevType, "RTS_") != nullptr)
      {
        this->m_MessageQueue->PushCommandMessage(headerMsg);
        this->InvokeEvent(CommandReceivedEvent());
        return IGTL_STATUS_OK;
      }

      //Create a message according to the header message
      igtl::MessageBase::Pointer curMessage;
      curMessage = m_MessageFactory->CreateInstance(headerMsg);

      //check if the curMessage is created properly, if not the message type is
      //not supported and the message has to be skipped
      if (curMessage.IsNull())
      {
        socket->Skip(headerMsg->GetBodySizeToRead(), 0);
        //  MITK_ERROR("IGTLDevice") << "The received type is not supported. Please "
        //                              "add it to the message factory.";
        return IGTL_STATUS_NOT_FOUND;
      }

      //insert the header to the message and allocate the pack
      curMessage->SetMessageHeader(headerMsg);
      curMessage->AllocatePack();

      // Receive transform data from the socket
      int receiveCheck = 0;
      receiveCheck = socket->Receive(curMessage->GetPackBodyPointer(),
        curMessage->GetPackBodySize(), m_ReadFully);

      if (receiveCheck > 0)
      {
        int c = curMessage->Unpack(1);
        if (!(c & igtl::MessageHeader::UNPACK_BODY))
        {
          return IGTL_STATUS_CHECKSUM_ERROR;
        }

        //check the type of the received message
        //if it is a command push it into the command queue
        //otherwise into the normal receive queue
        //STP_ commands are handled here because they implemented additional
        //member variables that are not stored in the header message
        if (std::strstr(curDevType, "STT_") != nullptr)
        {
          this->m_MessageQueue->PushCommandMessage(curMessage);
          this->InvokeEvent(CommandReceivedEvent());
        }
        else
        {
          if(m_LogMessages)
            MITK_INFO << "Received Message: " << mitk::IGTLMessage::New(curMessage)->ToString();
          this->m_MessageQueue->PushMessage(curMessage);
          this->InvokeEvent(MessageReceivedEvent());
        }
        return IGTL_STATUS_OK;
      }
      else
      {
        MITK_WARN("IGTLDevice") << "Received a valid header but could not "
          << "read the whole message.";
        return IGTL_STATUS_UNKNOWN_ERROR;
      }
    }
    else
    {
      //CRC check failed
      MITK_WARN << "CRC Check failed";
      return IGTL_STATUS_CHECKSUM_ERROR;
    }
  }
  else
  {
    //Message size information and actual data size don't match.
    //this state is not suppossed to be reached, return unknown error
    MITK_WARN << "IGTL status unknown";
    return IGTL_STATUS_UNKNOWN_ERROR;
  }
}

void mitk::IGTLDevice::SendMessage(mitk::IGTLMessage::Pointer msg)
{
  m_MessageQueue->PushSendMessage(msg);
}

unsigned int mitk::IGTLDevice::SendMessagePrivate(mitk::IGTLMessage::Pointer msg,
  igtl::Socket::Pointer socket)
{
  //check the input message
  if (msg.IsNull())
  {
    MITK_ERROR("IGTLDevice") << "Could not send message because message is not "
      "valid. Please check.";
    return false;
  }

  igtl::MessageBase* sendMessage = msg->GetMessage();

  // Pack (serialize) and send
  sendMessage->Pack();

  int sendSuccess = socket->Send(sendMessage->GetPackPointer(), sendMessage->GetPackSize());

  if (sendSuccess)
  {
    if (m_LogMessages) { MITK_INFO << "Send IGTL message: " << msg->ToString(); }
    this->InvokeEvent(MessageSentEvent());
    return IGTL_STATUS_OK;
  }
  else
  {
    return IGTL_STATUS_UNKNOWN_ERROR;
  }
}

void mitk::IGTLDevice::RunCommunication(void (IGTLDevice::*ComFunction)(void), itk::FastMutexLock* mutex)
{
  if (this->GetState() != Running)
    return;

  try
  {
    // keep lock until end of scope
    MutexLockHolder communicationFinishedLockHolder(*mutex);

    // Because m_StopCommunication is used by two threads, access has to be guarded
    // by a mutex. To minimize thread locking, a local copy is used here
    bool localStopCommunication;

    // update the local copy of m_StopCommunication
    this->m_StopCommunicationMutex->Lock();
    localStopCommunication = this->m_StopCommunication;
    this->m_StopCommunicationMutex->Unlock();
    while ((this->GetState() == Running) && (localStopCommunication == false))
    {
      (this->*ComFunction)();

      /* Update the local copy of m_StopCommunication */
      this->m_StopCommunicationMutex->Lock();
      localStopCommunication = m_StopCommunication;
      this->m_StopCommunicationMutex->Unlock();

      // time to relax, this sets the maximum ever possible framerate to 1000 Hz
      itksys::SystemTools::Delay(1);
    }
  }
  catch (...)
  {
    mutex->Unlock();
    this->StopCommunication();
    MITK_ERROR("IGTLDevice::RunCommunication") << "Error while communicating. Thread stopped.";
    //mitkThrowException(mitk::IGTException) << "Error while communicating. Thread stopped.";
  }
  // StopCommunication was called, thus the mode should be changed back to Ready now
  // that the tracking loop has ended.
  //this->SetState(Ready); //this is done elsewhere
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
  m_SendingFinishedMutex->Unlock();
  m_ReceivingFinishedMutex->Unlock();
  m_ConnectingFinishedMutex->Unlock();

  // start new threads that execute the communication
  m_SendThreadID =
    m_MultiThreader->SpawnThread(this->ThreadStartSending, this);
  m_ReceiveThreadID =
    m_MultiThreader->SpawnThread(this->ThreadStartReceiving, this);
  m_ConnectThreadID =
    m_MultiThreader->SpawnThread(this->ThreadStartConnecting, this);
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
    m_SendingFinishedMutex->Lock();
    m_ReceivingFinishedMutex->Lock();
    m_ConnectingFinishedMutex->Lock();
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
  //if retMsg is nullptr there is no return message defined and thus it is not
  //necessary to send one back
  if (rtsMsg.IsNotNull())
  {
    this->SendMessage(mitk::IGTLMessage::New(rtsMsg));
    return true;
  }
  else
  {
    return false;
  }
}

void mitk::IGTLDevice::Connect()
{
  MITK_DEBUG << "mitk::IGTLDevice::Connect();";
}

igtl::ImageMessage::Pointer mitk::IGTLDevice::GetNextImage2dMessage()
{
  return this->m_MessageQueue->PullImage2dMessage();
}

igtl::ImageMessage::Pointer mitk::IGTLDevice::GetNextImage3dMessage()
{
  return this->m_MessageQueue->PullImage3dMessage();
}

igtl::TransformMessage::Pointer mitk::IGTLDevice::GetNextTransformMessage()
{
  return this->m_MessageQueue->PullTransformMessage();
}

igtl::TrackingDataMessage::Pointer mitk::IGTLDevice::GetNextTrackingDataMessage()
{
  igtl::TrackingDataMessage::Pointer msg = this->m_MessageQueue->PullTrackingMessage();
  return msg;
}

igtl::StringMessage::Pointer mitk::IGTLDevice::GetNextStringMessage()
{
  return this->m_MessageQueue->PullStringMessage();
}

igtl::MessageBase::Pointer mitk::IGTLDevice::GetNextMiscMessage()
{
  return this->m_MessageQueue->PullMiscMessage();
}

igtl::MessageBase::Pointer mitk::IGTLDevice::GetNextCommand()
{
  return m_MessageQueue->PullCommandMessage();
}
void mitk::IGTLDevice::EnableNoBufferingMode(bool enable)
{
  m_MessageQueue->EnableNoBufferingMode(enable);
}

void mitk::IGTLDevice::EnableNoBufferingMode(
  mitk::IGTLMessageQueue::Pointer queue,
  bool enable)
{
  queue->EnableNoBufferingMode(enable);
}

ITK_THREAD_RETURN_TYPE mitk::IGTLDevice::ThreadStartSending(void* pInfoStruct)
{
  /* extract this pointer from Thread Info structure */
  struct itk::MultiThreader::ThreadInfoStruct * pInfo =
    (struct itk::MultiThreader::ThreadInfoStruct*)pInfoStruct;
  if (pInfo == nullptr)
  {
    return ITK_THREAD_RETURN_VALUE;
  }
  if (pInfo->UserData == nullptr)
  {
    return ITK_THREAD_RETURN_VALUE;
  }
  IGTLDevice *igtlDevice = (IGTLDevice*)pInfo->UserData;
  if (igtlDevice != nullptr)
  {
    igtlDevice->RunCommunication(&mitk::IGTLDevice::Send, igtlDevice->m_SendingFinishedMutex);
  }
  igtlDevice->m_SendThreadID = 0;  // erase thread id because thread will end.
  return ITK_THREAD_RETURN_VALUE;
}

ITK_THREAD_RETURN_TYPE mitk::IGTLDevice::ThreadStartReceiving(void* pInfoStruct)
{
  /* extract this pointer from Thread Info structure */
  struct itk::MultiThreader::ThreadInfoStruct * pInfo =
    (struct itk::MultiThreader::ThreadInfoStruct*)pInfoStruct;
  if (pInfo == nullptr)
  {
    return ITK_THREAD_RETURN_VALUE;
  }
  if (pInfo->UserData == nullptr)
  {
    return ITK_THREAD_RETURN_VALUE;
  }
  IGTLDevice *igtlDevice = (IGTLDevice*)pInfo->UserData;
  if (igtlDevice != nullptr)
  {
    igtlDevice->RunCommunication(&mitk::IGTLDevice::Receive,
      igtlDevice->m_ReceivingFinishedMutex);
  }
  igtlDevice->m_ReceiveThreadID = 0;  // erase thread id because thread will end.
  return ITK_THREAD_RETURN_VALUE;
}

ITK_THREAD_RETURN_TYPE mitk::IGTLDevice::ThreadStartConnecting(void* pInfoStruct)
{
  /* extract this pointer from Thread Info structure */
  struct itk::MultiThreader::ThreadInfoStruct * pInfo =
    (struct itk::MultiThreader::ThreadInfoStruct*)pInfoStruct;
  if (pInfo == nullptr)
  {
    return ITK_THREAD_RETURN_VALUE;
  }
  if (pInfo->UserData == nullptr)
  {
    return ITK_THREAD_RETURN_VALUE;
  }
  IGTLDevice *igtlDevice = (IGTLDevice*)pInfo->UserData;
  if (igtlDevice != nullptr)
  {
    igtlDevice->RunCommunication(&mitk::IGTLDevice::Connect,
      igtlDevice->m_ConnectingFinishedMutex);
  }
  igtlDevice->m_ConnectThreadID = 0;  // erase thread id because thread will end.
  return ITK_THREAD_RETURN_VALUE;
}
