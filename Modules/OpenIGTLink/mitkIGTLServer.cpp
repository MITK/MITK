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

void mitk::IGTLServer::Connect()
{
  igtl::Socket::Pointer socket;
  //check if another igtl device wants to connect to this socket
  socket =
    ((igtl::ServerSocket*)(this->m_Socket.GetPointer()))->WaitForConnection(10);
  //if there is a new connection the socket is not null
  if ( socket.IsNotNull() )
  {
    //add the new client socket to the list of registered clients
    this->m_RegisteredClients.push_back(socket);
    //inform observers about this new client
    this->InvokeEvent(NewClientConnectionEvent());
  }
}

void mitk::IGTLServer::Receive()
{
  //the server can be connected with several clients, therefore it has to check
  //all registered clients
  std::list<igtl::Socket::Pointer>::iterator it;
  std::list<igtl::Socket::Pointer>::iterator it_end =
      this->m_RegisteredClients.end();
  for ( it = this->m_RegisteredClients.begin(); it != it_end; ++it )
  {
    //maybe there should be a check here if the current socket is still active
    this->ReceivePrivate(*it);
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
  std::list<igtl::Socket::Pointer>::iterator it;
  std::list<igtl::Socket::Pointer>::iterator it_end =
      this->m_RegisteredClients.end();
  for ( it = this->m_RegisteredClients.begin(); it != it_end; ++it )
  {
    //maybe there should be a check here if the current socket is still active
    this->SendMessagePrivate(curMessage.GetPointer(), *it);
  }
}



void mitk::IGTLServer::StopCommunicationWithSocket(igtl::Socket* client)
{
  std::list<igtl::Socket::Pointer>::iterator it =
      this->m_RegisteredClients.begin();
  std::list<igtl::Socket::Pointer>::iterator itEnd =
      this->m_RegisteredClients.end();

  for (; it != itEnd; ++it )
  {
    if ( (*it).GetPointer() == client )
    {
      this->m_RegisteredClients.remove(*it);
      break;
    }
  }
}
