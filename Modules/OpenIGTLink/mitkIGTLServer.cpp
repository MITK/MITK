/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkIGTLServer.h"
#include <cstdio>

#include <itksys/SystemTools.hxx>

#include <igtlServerSocket.h>
#include <igtlTrackingDataMessage.h>
#include <igtlImageMessage.h>
#include <mitkIGTLStatus.h>

mitk::IGTLServer::IGTLServer(bool ReadFully) :
IGTLDevice(ReadFully)
{
}

mitk::IGTLServer::~IGTLServer()
{
}

bool mitk::IGTLServer::OpenConnection()
{
  if (this->GetState() != Setup)
  {
    mitkThrowException(mitk::Exception) <<
      "Can only try to create a server if in setup mode";
    return false;
  }

  int portNumber = this->GetPortNumber();

  if (portNumber == -1)
  {
    //port number was not correct
    return false;
  }

  //create a new server socket
  m_Socket = igtl::ServerSocket::New();

  //try to create the igtl server
  int response = dynamic_cast<igtl::ServerSocket*>(m_Socket.GetPointer())->
    CreateServer(portNumber);

  //check the response
  if (response != 0)
  {
    mitkThrowException(mitk::Exception) <<
      "The server could not be created. Port: " << portNumber;
    return false;
  }

  // everything is initialized and connected so the communication can be started
  this->SetState(Ready);

  return true;
}

bool mitk::IGTLServer::CloseConnection()
{
  //remove all registered clients
  m_SentListMutex.lock();
  m_ReceiveListMutex.lock();
  SocketListType allRegisteredSockets(m_RegisteredClients);
  m_SentListMutex.unlock();
  m_ReceiveListMutex.unlock();
  this->StopCommunicationWithSocket(allRegisteredSockets);

  return mitk::IGTLDevice::CloseConnection();
}

void mitk::IGTLServer::Connect()
{
  igtl::Socket::Pointer socket;
  //check if another igtl device wants to connect to this socket
  socket =
    ((igtl::ServerSocket*)(this->m_Socket.GetPointer()))->WaitForConnection(1);
  //if there is a new connection the socket is not null
  if (socket.IsNotNull())
  {
    //add the new client socket to the list of registered clients
    m_SentListMutex.lock();
    m_ReceiveListMutex.lock();
    this->m_RegisteredClients.push_back(socket);
    m_SentListMutex.unlock();
    m_ReceiveListMutex.unlock();
    //inform observers about this new client
    this->InvokeEvent(NewClientConnectionEvent());
    MITK_INFO("IGTLServer") << "Connected to a new client: " << socket;
  }
}

void mitk::IGTLServer::Receive()
{
  unsigned int status = IGTL_STATUS_OK;
  SocketListType socketsToBeRemoved;

  //the server can be connected with several clients, therefore it has to check
  //all registered clients
  SocketListIteratorType it;
  m_ReceiveListMutex.lock();
  auto it_end = this->m_RegisteredClients.end();
  for (it = this->m_RegisteredClients.begin(); it != it_end; ++it)
  {
    //it is possible that ReceivePrivate detects that the current socket is
    //already disconnected. Therefore, it is necessary to remove this socket
    //from the registered clients list
    status = this->ReceivePrivate(*it);
    if (status == IGTL_STATUS_NOT_PRESENT)
    {
      //remember this socket for later, it is not a good idea to remove it
      //from the list directly because we iterate over the list at this point
      socketsToBeRemoved.push_back(*it);
      MITK_WARN("IGTLServer") << "Lost connection to a client socket. ";
    }
    else if (status != 1)
    {
      MITK_DEBUG("IGTLServer") << "IGTL Message with status: " << status;
    }
  }
  m_ReceiveListMutex.unlock();
  if (socketsToBeRemoved.size() > 0)
  {
    //remove the sockets that are not connected anymore
    this->StopCommunicationWithSocket(socketsToBeRemoved);
    //inform observers about loosing the connection to these sockets
    this->InvokeEvent(LostConnectionEvent());
  }
}

void mitk::IGTLServer::Send()
{
  //get the latest message from the queue
  mitk::IGTLMessage::Pointer curMessage = this->m_MessageQueue->PullSendMessage();

  // there is no message => return
  if (curMessage.IsNull())
    return;

  //the server can be connected with several clients, therefore it has to check
  //all registered clients
  //sending a message to all registered clients might not be the best solution,
  //it could be better to store the client together with the requested type. Then
  //the data would be send to the appropriate client and to noone else.
  //(I know it is no excuse but PLUS is doing exactly the same, they broadcast
  //everything)
  m_SentListMutex.lock();
  SocketListIteratorType it;
  auto it_end =
    this->m_RegisteredClients.end();
  for (it = this->m_RegisteredClients.begin(); it != it_end; ++it)
  {
    //maybe there should be a check here if the current socket is still active
    this->SendMessagePrivate(curMessage, *it);
    MITK_DEBUG("IGTLServer") << "Sent IGTL Message";
  }
  m_SentListMutex.unlock();
}

void mitk::IGTLServer::StopCommunicationWithSocket(
  SocketListType& toBeRemovedSockets)
{
  for (auto i = toBeRemovedSockets.begin(); i != toBeRemovedSockets.end(); i++)
    this->StopCommunicationWithSocket(*i);
}

void mitk::IGTLServer::StopCommunicationWithSocket(igtl::Socket* client)
{
  m_SentListMutex.lock();
  m_ReceiveListMutex.lock();
  auto i = m_RegisteredClients.begin();
  auto end = m_RegisteredClients.end();
  while (i != end)
  {
    if ((*i) == client)
    {
      //    //close the socket
      (*i)->CloseSocket();
      //and remove it from the list
      i = this->m_RegisteredClients.erase(i);
      MITK_INFO("IGTLServer") << "Removed client socket from server client list.";
      break;
    }
    else
    {
      ++i;
    }
  }
  m_SentListMutex.unlock();
  m_ReceiveListMutex.unlock();
}

unsigned int mitk::IGTLServer::GetNumberOfConnections()
{
  return this->m_RegisteredClients.size();
}
