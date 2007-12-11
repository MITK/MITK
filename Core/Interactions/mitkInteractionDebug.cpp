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
#include "SocketClient.h"
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
  m_SocketClient = new SocketClient();
  m_SocketClient->open( "127.0.0.1", 34768 );
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
	*((unsigned int*) wb) = (unsigned int) GetHashValue();
	wb += 4;
  
  int size = strlen( m_FileName );

  *((unsigned int*) wb) = (unsigned int) size;
	wb += 4;

  strcpy( wb, m_FileName );
  size += 8;

  sendCounter();
  // std::cout << "Open Connection file name: " << m_FileName << std::endl;
  m_SocketClient->send( OPEN_CONNECTION, size, m_Buffer );
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
	*((unsigned int*) wb) = (unsigned int) stateMachine;
	wb += 4;

  // Length of the name
  int size = strlen(name);

	*((unsigned int*) wb) = (unsigned int) size;
	wb += 4;

  // name
  strcpy(wb,name);

  size += 8;

  sendCounter();
  // std::cout << "NEW_STATE_MACHINE: instance: " << (unsigned int) stateMachine << " Type: " << name << std::endl;
  return m_SocketClient->send( NEW_STATE_MACHINE, size, m_Buffer );
}

/**
  *
  */
bool InteractionDebug::Event( const StateMachine* stateMachine, unsigned int EventId )
{
  if ( stateMachine == NULL || stateMachine->GetType().empty() )
    return false;

  // Instance Address
  char* wb = m_Buffer;
	*((unsigned int*) wb) = (unsigned int) stateMachine;
	wb += 4;

  // eventID
	*((unsigned int*) wb) = EventId;
	
  sendCounter();
  // std::cout << "EVENT: instance: " << (unsigned int) stateMachine << " EventId: " << EventId << std::endl;
  return m_SocketClient->send( EVENT, 8, m_Buffer );
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
	*((unsigned int*) wb) = (unsigned int) stateMachine;
	wb += 4;

  // transitionName
  unsigned int size = strlen( transitionName );
	*((unsigned int*) wb) = size;

  wb += 4;

  for ( unsigned int i=0; i<size; i++, wb++ )
    *wb = transitionName[i];

  size += 8;
	
  sendCounter();
  // std::cout << "TRANSITION: instance: " << (unsigned int) stateMachine << " size: " << size << " transitionName: " << transitionName << std::endl;  
  return m_SocketClient->send( TRANSITION, size, m_Buffer );
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
	*((unsigned int*) wb) = (unsigned int) stateMachine;
	wb += 4;

  // transitionName
  unsigned int size = strlen( transitionName );
	*((unsigned int*) wb) = size;

	wb += 4;

  for ( unsigned int i=0; i<size; i++, wb++ )
    *wb = transitionName[i];

  wb += 4;

  *((unsigned int*) wb) = action;

  size += 12;

  sendCounter();
  // std::cout << "ACTION: instance: " << (unsigned int) stateMachine << " size: " << size << "action: " << action << " transitionName " << transitionName << std::endl;
  return m_SocketClient->send( ACTION, size, m_Buffer );
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
	*((unsigned int*) wb) = (unsigned int) stateMachine;

  sendCounter();
  // std::cout << "DELETE_STATE_MACHINE: instance: << (unsigned int) stateMachine" << std::endl;
  return m_SocketClient->send( DELETE_STATE_MACHINE, 4, m_Buffer );
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
  static unsigned int m_Counter = 0;
  static char my_Buffer[4];
  m_Counter++;

  char* wb = my_Buffer;
	*((unsigned int*) wb) = (unsigned int) m_Counter;

  std::cout << "Counter: " <<  m_Counter << std::endl;
  m_SocketClient->send( 7, 4, wb );
}

} // mitk
