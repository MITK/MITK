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

#include "mitkIGTLClient.h"
//#include "mitkIGTTimeStamp.h"
//#include "mitkIGTHardwareException.h"
#include <stdio.h>

#include <itksys/SystemTools.hxx>
#include <itkMutexLockHolder.h>

#include <igtlClientSocket.h>

typedef itk::MutexLockHolder<itk::FastMutexLock> MutexLockHolder;


mitk::IGTLClient::IGTLClient() :
IGTLDevice()
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
      "Can only try to open the connection if in setup mode";
    return false;
  }

  std::string hostname = this->GetHostname();
  int portNumber = this->GetPortNumber();

  if (portNumber == -1 || hostname.size() <= 0)
  {
    //port number or hostname was not correct
    return false;
  }

  //create a new client socket
  m_Socket = igtl::ClientSocket::New();

  //try to connect to the igtl server
  int response = dynamic_cast<igtl::ClientSocket*>(m_Socket.GetPointer())->
    ConnectToServer(hostname.c_str(), portNumber);

  //check the response
  if ( response != 0 )
  {
    mitkThrowException(mitk::Exception) <<
      "The client could not connect to " << hostname << " port: " << portNumber;
    return false;
  }

  // everything is initialized and connected so the communication can be started
  this->SetState(Ready);

  return true;
}
