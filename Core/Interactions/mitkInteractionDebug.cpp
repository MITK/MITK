#include "mitkInteractionDebug.h"
#include "SocketClient.h"
#include <string.h>

namespace mitk {

const int InteractionDebug::OPEN_CONNECTION = 1;
const int InteractionDebug::NEW_STATE_MACHINE = 2;
const int InteractionDebug::EVENT = 3;
const int InteractionDebug::TRANSITION = 4;
const int InteractionDebug::DELETE_STATE_MACHINE = 5;

InteractionDebug* InteractionDebug::m_Instance = NULL;;
char* InteractionDebug::m_FileName = NULL;;


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

  m_SocketClient->send( NEW_STATE_MACHINE, size, m_Buffer );
}

/**
  *
  */
bool InteractionDebug::NewStateMachine( const char* name, const StateMachine* stateMachine )
{
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

  return m_SocketClient->send( NEW_STATE_MACHINE, size, m_Buffer );
}

/**
  *
  */
bool InteractionDebug::Event( const StateMachine* stateMachine, unsigned int EventId )
{
  // Instance Address
  char* wb = m_Buffer;
	*((unsigned int*) wb) = (unsigned int) stateMachine;
	wb += 4;

  // eventID
	*((unsigned int*) wb) = EventId;
	
  return m_SocketClient->send( EVENT, 8, m_Buffer );
}

/**
  *
  */
bool InteractionDebug::Transition( const StateMachine* stateMachine, int transitionId )
{
  // Instance Address
  char* wb = m_Buffer;
	*((unsigned int*) wb) = (unsigned int) stateMachine;
	wb += 4;

  // eventID
	*((unsigned int*) wb) = transitionId;
	
  return m_SocketClient->send( EVENT, 8, m_Buffer );
}

/**
  *
  */
bool InteractionDebug::DeleteStateMachine( const StateMachine* stateMachine )
{
  // Instance Address
  char* wb = m_Buffer;
	*((unsigned int*) wb) = (unsigned int) stateMachine;
	
  return m_SocketClient->send( EVENT, 4, m_Buffer );
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

} // mitk
