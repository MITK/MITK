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

#ifndef INTERACTION_DEBUG_H
#define INTERACTION_DEBUG_H
#include "mitkStateMachine.h"
#include "mitkSocketClient.h"

namespace mitk{

class MITK_CORE_EXPORT InteractionDebug
{
  static InteractionDebug* m_Instance;
  static char* m_FileName;

  char m_Buffer[255];
  
  static const int OPEN_CONNECTION;
  static const int NEW_STATE_MACHINE;
  static const int EVENT;  
  static const int TRANSITION;
  static const int ACTION;
  static const int DELETE_STATE_MACHINE;
  
  unsigned int GetHashValue();
  InteractionDebug();

public:
  void OpenConection();
  bool NewStateMachine( const char* name, const StateMachine* stateMachine );
  bool Event( const StateMachine* stateMachine, unsigned int EventId );
  bool Transition( const StateMachine* stateMachine, const char* transitionName );
  bool Action( const StateMachine* stateMachine, const char* transitionName, unsigned int action );
  bool DeleteStateMachine( const StateMachine* stateMachine );
  void setMaxConnectionAdvance(int maxConectionAdvance);
  static InteractionDebug* GetInstance();
  static void SetXMLFileName( const char* fileName );

  void sendCounter();
};

} // mitk

#endif // INTERACTION_DEBUG_H
