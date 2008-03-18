/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 13129 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkSocketClient.h"
#include <itkObjectFactory.h>
#include <itkOutputWindow.h>
#include <itkCommand.h>

namespace mitk
{

  SocketClientImplementation* SocketClient::m_Implementation = NULL;
  SocketClient* SocketClient::m_Instance = NULL;

  void SocketClient::open( const char* ipAdress, unsigned short port )
  {
    if (m_Implementation)
    {
      m_Implementation->open( ipAdress, port );
    }
  }

  void SocketClient::setMaxConnectionAdvance( int maxConnectionAdvance )
  {
    if (m_Implementation)
    {
      m_Implementation->setMaxConnectionAdvance( maxConnectionAdvance );
    }
  }

  bool SocketClient::send( unsigned int messagetype, unsigned int bodySize, char* body )
  {
    if (m_Implementation)
    {
      return m_Implementation->send( messagetype, bodySize, body);
    }
    else
    {
      return FALSE;
    }
  }

  void SocketClient::SetImplementationInstance(SocketClientImplementation* implementation)
  {
    if ( m_Implementation == implementation )
    {
      return;
    }
    m_Implementation = implementation;
  }

  SocketClient* SocketClient::GetInstance()
  {
    if (m_Instance == NULL)
    {
      m_Instance = new SocketClient();
    }

    return m_Instance;
  }

  SocketClient::SocketClient()
  {
  }

  SocketClient::~SocketClient()
  {
  }
}//end namespace mitk
