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

#include "mitkIGTLServer.h"
#include <stdio.h>

#include <itksys/SystemTools.hxx>
#include <itkMutexLockHolder.h>

#include <igtlServerSocket.h>
#include <igtl_status.h>


mitk::IGTLServer::IGTLServer() :
IGTLDevice()
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

  if (portNumber == -1 )
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
  if ( response != 0 )
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
  SocketListType allRegisteredSockets = m_RegisteredClients;
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
  if ( socket.IsNotNull() )
  {
    //add the new client socket to the list of registered clients
    this->m_RegisteredClients.push_back(socket);
    //inform observers about this new client
    this->InvokeEvent(NewClientConnectionEvent());
    MITK_INFO("IGTLServer") << "Connected to a new client.";
  }
}

void mitk::IGTLServer::Receive()
{
  unsigned int status = IGTL_STATUS_OK;
  SocketListType socketsToBeRemoved;

  //the server can be connected with several clients, therefore it has to check
  //all registered clients
  SocketListIteratorType it;
  auto it_end = this->m_RegisteredClients.end();
  for ( it = this->m_RegisteredClients.begin(); it != it_end; ++it )
  {
    //it is possible that ReceivePrivate detects that the current socket is
    //already disconnected. Therefore, it is necessary to remove this socket
    //from the registered clients list
    status = this->ReceivePrivate(*it);
    if ( status == IGTL_STATUS_NOT_PRESENT )
    {
      //remember this socket for later, it is not a good idea to remove it
      //from the list directly because we iterator over the list at this point
      socketsToBeRemoved.push_back(*it);
      MITK_WARN("IGTLServer") << "Lost connection to a client socket. ";
    }
  }
  if ( socketsToBeRemoved.size() > 0 )
  {
    //remove the sockets that are not connected anymore
    this->StopCommunicationWithSocket(socketsToBeRemoved);
    //inform observers about loosing the connection to these sockets
    this->InvokeEvent(LostConnectionEvent());
  }
}

void mitk::IGTLServer::Send()
{
  igtl::MessageBase::Pointer curMessage;

  //get the latest message from the queue
  curMessage = this->m_SendQueue->PullMessage();

  // there is no message => return
  if ( curMessage.IsNull() )
    return;

  //the server can be connected with several clients, therefore it has to check
  //all registered clients
  //sending a message to all registered clients might not be the best solution,
  //it could be better to store the client together with the requested type. Then
  //the data would be send to the appropriate client and to noone else.
  //(I know it is no excuse but PLUS is doing exactly the same, they broadcast
  //everything)
  SocketListIteratorType it;
  auto it_end =
      this->m_RegisteredClients.end();
  for ( it = this->m_RegisteredClients.begin(); it != it_end; ++it )
  {
    //maybe there should be a check here if the current socket is still active
    this->SendMessagePrivate(curMessage.GetPointer(), *it);
  }
}

void mitk::IGTLServer::StopCommunicationWithSocket(
    SocketListType& toBeRemovedSockets)
{
  auto it    = toBeRemovedSockets.begin();
  auto itEnd = toBeRemovedSockets.end();
  for (; it != itEnd; ++it )
  {
    this->StopCommunicationWithSocket(*it);
  }
}

void mitk::IGTLServer::StopCommunicationWithSocket(igtl::Socket* client)
{
  auto it    = this->m_RegisteredClients.begin();
  auto itEnd = this->m_RegisteredClients.end();

  for (; it != itEnd; ++it )
  {
    if ( (*it) == client )
    {
      //close the socket
      (*it)->CloseSocket();
      //and remove it from the list
      this->m_RegisteredClients.remove(*it);
      break;
    }
  }
  MITK_INFO("IGTLServer") << "Removed client socket from server client list.";
}

unsigned int mitk::IGTLServer::GetNumberOfConnections()
{
  return this->m_RegisteredClients.size();
}
