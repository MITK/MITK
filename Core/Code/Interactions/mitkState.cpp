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


#include "mitkState.h"


mitk::State::State(std::string stateName, int stateId)
: m_Name(stateName), m_Id(stateId)
{
}

mitk::State::~State()
{
  //delete all transitions
  while (!m_Transitions.empty())
  {
    //copy first
    mitk::Transition* tempTransition = m_Transitions.begin()->second;
    //deleting it from map
    m_Transitions.erase(m_Transitions.begin());
    //deleting transition
    delete tempTransition;
  }
}


bool mitk::State::AddTransition( Transition* transition )
{
  std::pair<TransMapIter,bool> ok = m_Transitions.insert(TransitionMap::value_type( transition->GetEventId(), transition ));
  return (bool) ok.second;
}

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
      return (*tempTrans).second.GetPointer();
    }
    else
      return NULL;
  }
}

std::string mitk::State::GetName() const
{
  return m_Name;
}


int mitk::State::GetId() const
{
  return m_Id;
}


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
