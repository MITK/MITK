/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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


#include "mitkState.h"
#include "mitkTransition.h"


//##ModelId=3E5B2A9203BD
mitk::State::State(std::string stateName, int stateId)
: m_Name(stateName), m_Id(stateId)
{
}

//##ModelId=3E5B2B2E0304
bool mitk::State::AddTransition( Transition* transition )
{
  std::pair<TransMapIter,bool> ok = m_Transitions.insert(TransitionMap::value_type( transition->GetEventId(), transition ));
  return (bool) ok.second;
}

//##ModelId=3E5B2B9000AC
const mitk::Transition* mitk::State::GetTransition(int eventId) const
{

  TransitionMap::const_iterator tempTrans = m_Transitions.find(eventId);
  if( tempTrans != m_Transitions.end() )
    return (*tempTrans).second;
  else //can a Transition with ID 0 be found?
  {
    tempTrans = m_Transitions.find(0);
    if ( tempTrans != m_Transitions.end() )//found transition 0 (= transmitt all events to other local StateMachines)
    {
      return (*tempTrans).second;
    }
    else
      return NULL;
  }
}

//##ModelId=3E5B2C0503D5
std::string mitk::State::GetName() const
{
  return m_Name;
}

//##ModelId=3E5B2C14016A
int mitk::State::GetId() const
{
  return m_Id;
}


//##ModelId=3E7757280208
//##Documentation
//## gives all next States back. To parse through all States.
std::set<int> mitk::State::GetAllNextStates() const
{
  std::set<int> tempset;

  for (TransMapConstIter i= m_Transitions.begin(); i != m_Transitions.end(); i++)
  {
    tempset.insert( (i->second)->GetNextStateId() );
  }
  return tempset;
}

//##ModelId=3E64B4360017
//##Documentation
//## to check, if this Event has a Transition. 
//## for menu Behavior e.g.
bool mitk::State::IsValidEvent(int eventId) const
{
  if( m_Transitions.find(eventId) != m_Transitions.end() )
    return true;
  else
    return false;
}

//##ModelId=3E68C573013F
//##Documentation
//## searches dedicated States of all Transitions and 
//## sets *nextState of these Transitions.
//## allStates is a List of all build States of that StateMachine
bool mitk::State::ConnectTransitions(StateMap *allStates)
{
  for (TransMapIter i= m_Transitions.begin(); i != m_Transitions.end(); i++)
  {
    StateMapIter sIter = allStates->find(((*i).second)->GetNextStateId());
    if( sIter != allStates->end() )
      ((*i).second)->SetNextState((*sIter).second);
    else
      return false;//State not found!
  }
  return true;
}
