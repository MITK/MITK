/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkIGTLClient.h"
//#include "mitkIGTTimeStamp.h"
//#include "mitkIGTHardwareException.h"
#include "igtlTrackingDataMessage.h"
#include <cstdio>

#include <itksys/SystemTools.hxx>
#include <itkMutexLockHolder.h>

#include <igtlClientSocket.h>
#include <igtl_status.h>

typedef itk::MutexLockHolder<itk::FastMutexLock> MutexLockHolder;

mitk::IGTLClient::IGTLClient(bool ReadFully) :
IGTLDevice(ReadFully)
{
}

mitk::IGTLClient::~IGTLClient()
{
}

bool mitk::IGTLClient::OpenConnection()
{
  if (this->GetState() != Setup)
  {
    mitkThrowException(mitk::Exception) <<
      "Can only try to open the connection if in setup mode. State was " << this->GetState();
    return false;
  }

  std::string hostname = this->GetHostname();
  int portNumber = this->GetPortNumber();

  if (portNumber == -1 || hostname.size() <= 0)
  {
    //port number or hostname was not correct
    MITK_WARN << "Port number or hostname was not correct";
    return false;
  }

  //create a new client socket
  m_Socket = igtl::ClientSocket::New();

  //try to connect to the igtl server
  int response = dynamic_cast<igtl::ClientSocket*>(m_Socket.GetPointer())->
    ConnectToServer(hostname.c_str(), portNumber);

  //check the response
  if (response != 0)
  {
    MITK_ERROR << "The client could not connect to " << hostname << " port: " << portNumber;
    return false;
  }

  // everything is initialized and connected so the communication can be started
  this->SetState(Ready);

  //inform observers about this new client
  this->InvokeEvent(NewClientConnectionEvent());

  return true;
}

void mitk::IGTLClient::Receive()
{
  //MITK_INFO << "Trying to receive message";
  //try to receive a message, if the socket is not present anymore stop the
  //communication
  unsigned int status = this->ReceivePrivate(this->m_Socket);
  if (status == IGTL_STATUS_NOT_PRESENT)
  {
    this->StopCommunicationWithSocket(this->m_Socket);
    //inform observers about loosing the connection to this socket
    this->InvokeEvent(LostConnectionEvent());
    MITK_WARN("IGTLClient") << "Lost connection to server socket.";
  }
}

void mitk::IGTLClient::Send()
{
  mitk::IGTLMessage::Pointer mitkMessage;

  //get the latest message from the queue
  mitkMessage = this->m_MessageQueue->PullSendMessage();

  // there is no message => return
  if (mitkMessage.IsNull())
    return;

  if (!this->SendMessagePrivate(mitkMessage, this->m_Socket))
  {
    MITK_WARN("IGTLDevice") << "Could not send the message.";
  }
}

void mitk::IGTLClient::StopCommunicationWithSocket(igtl::Socket* /*socket*/)
{
  m_StopCommunicationMutex->Lock();
  m_StopCommunication = true;
  m_StopCommunicationMutex->Unlock();
}

unsigned int mitk::IGTLClient::GetNumberOfConnections()
{
  return this->m_Socket->GetConnected();
}
