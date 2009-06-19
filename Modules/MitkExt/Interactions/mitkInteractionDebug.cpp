/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
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

#include "mitkInteractionDebug.h"
#include "mitkSocketClient.h"
#include <string.h>

namespace mitk {

  const int InteractionDebug::OPEN_CONNECTION = 1;
  const int InteractionDebug::NEW_STATE_MACHINE = 2;
  const int InteractionDebug::EVENT = 3;
  const int InteractionDebug::TRANSITION = 4;
  const int InteractionDebug::ACTION = 5;
  const int InteractionDebug::DELETE_STATE_MACHINE = 6;

  InteractionDebug* InteractionDebug::m_Instance = NULL;;
  char* InteractionDebug::m_FileName = NULL;

  /**
    *
    */
  InteractionDebug::InteractionDebug()
  {
    SocketClient::GetInstance()->open( "127.0.0.1", 34768 );
  }


  /**
    *
    */
  unsigned int InteractionDebug::GetHashValue()
  {
    return 0;
  }

  /**
    *
    */
  void InteractionDebug::OpenConection()
  {
    char* wb = m_Buffer;
    *((unsigned long*) wb) = (unsigned long) GetHashValue();
    wb += sizeof(long);
    
    int size = strlen( m_FileName );

    *((unsigned long*) wb) = (unsigned long) size;
    wb += sizeof(long);

    strcpy( wb, m_FileName );
    size += 2*sizeof(long);

    sendCounter();
    // LOG_INFO << "Open Connection file name: " << m_FileName << std::endl;
    SocketClient::GetInstance()->send( OPEN_CONNECTION, size, m_Buffer );
  }

  /**
    *
    */
  bool InteractionDebug::NewStateMachine( const char* name, const StateMachine* stateMachine )
  {
    if ( name == NULL )
      return false;  

    // Instance Address
    char* wb = m_Buffer;
    *((unsigned long*) wb) = (unsigned long) stateMachine;
    wb += sizeof(long);

    // Length of the name
    int size = strlen(name);

    *((unsigned long*) wb) = (unsigned long) size;
    wb += sizeof(long);

    // name
    strcpy(wb,name);

    size += 2*sizeof(long);

    sendCounter();
    // LOG_INFO << "NEW_STATE_MACHINE: instance: " << (unsigned int) stateMachine << " Type: " << name << std::endl;
    return SocketClient::GetInstance()->send( NEW_STATE_MACHINE, size, m_Buffer );
  }

  /**
    *
    */
  bool InteractionDebug::Event( const StateMachine* stateMachine, unsigned int EventId )
  {
    if ( EventId != 520)
    {
      if ( stateMachine == NULL || stateMachine->GetType().empty() )
        return false;

      // Instance Address
      char* wb = m_Buffer;
      *((unsigned long*) wb) = (unsigned long) stateMachine;
      wb += sizeof(long);

      // eventID
      *((unsigned long*) wb) = EventId;
      
      sendCounter();
      // LOG_INFO << "EVENT: instance: " << (unsigned int) stateMachine << " EventId: " << EventId << std::endl;
      return SocketClient::GetInstance()->send( EVENT, 2*sizeof(long), m_Buffer );
    }
    return true;
  }

  /**
    *
    */
  bool InteractionDebug::Transition( const StateMachine* stateMachine, const char* transitionName )
  {
    if ( stateMachine == NULL || stateMachine->GetType().empty() )
      return false;

    // Instance Address
    char* wb = m_Buffer;
    *((unsigned long*) wb) = (unsigned long) stateMachine;
    wb += sizeof(long);

    // transitionName
    unsigned long size = strlen( transitionName );
    *((unsigned long*) wb) = size;

    wb += sizeof(long);

    for ( unsigned long i=0; i<size; i++, wb++ )
      *wb = transitionName[i];

    size += 2*sizeof(long);
    
    //sendCounter();
    //LOG_INFO << "TRANSITION: instance: " << (unsigned int) stateMachine << " size: " << size << " transitionName: " << transitionName << std::endl;  
    return SocketClient::GetInstance()->send( TRANSITION, size, m_Buffer );
  }

  /**
    *
    */
  bool InteractionDebug::Action( const StateMachine* stateMachine, const char* transitionName, unsigned int action )
  {
    if ( stateMachine == NULL || stateMachine->GetType().empty() )
      return false;

    // Instance Address
    char* wb = m_Buffer;
    *((unsigned long*) wb) = (unsigned long) stateMachine;
    wb += sizeof(long);

    // transitionName
    unsigned long size = strlen( transitionName );
    *((unsigned long*) wb) = size;

    wb += sizeof(long);

    for ( unsigned long i=0; i<size; i++, wb++ )
      *wb = transitionName[i];

    wb += sizeof(long);

    *((unsigned long*) wb) = action;

    size += 3*sizeof(long);

    sendCounter();
    // LOG_INFO << "ACTION: instance: " << (unsigned int) stateMachine << " size: " << size << "action: " << action << " transitionName " << transitionName << std::endl;
    return SocketClient::GetInstance()->send( ACTION, size, m_Buffer );
  }

  /**
    *
    */
  bool InteractionDebug::DeleteStateMachine( const StateMachine* stateMachine )
  {
    if ( stateMachine == NULL || stateMachine->GetType().empty() )
      return false;

    // Instance Address
    char* wb = m_Buffer;
    *((unsigned long*) wb) = (unsigned long) stateMachine;

    sendCounter();
    // LOG_INFO << "DELETE_STATE_MACHINE: instance: << (unsigned int) stateMachine" << std::endl;
    return SocketClient::GetInstance()->send( DELETE_STATE_MACHINE, sizeof(long), m_Buffer );
  }

  /**
    *
    */
  InteractionDebug* InteractionDebug::GetInstance()
  {
    if (m_Instance==NULL)
      m_Instance = new InteractionDebug();

    return m_Instance;
  }

  /**
    *
    */
  void InteractionDebug::SetXMLFileName( const char* fileName )
  {
    int size = strlen(fileName) + 1;
    m_FileName = new char[size];
    strcpy(m_FileName,fileName);
  }

  void InteractionDebug::sendCounter()
  {
    static unsigned long m_Counter = 0;
    static char my_Buffer[sizeof(long)];
    m_Counter++;

    char* wb = my_Buffer;
    *((unsigned long*) wb) = (unsigned long) m_Counter;

    bool success = SocketClient::GetInstance()->send( 7, sizeof(long), wb );
    if (success)
    {
      LOG_INFO << "Counter: " <<  m_Counter << std::endl;
    }
  }

  void InteractionDebug::setMaxConnectionAdvance(int maxConnectionAdvance)
  {
    SocketClient::GetInstance()->setMaxConnectionAdvance(maxConnectionAdvance);
  }

} // mitk
