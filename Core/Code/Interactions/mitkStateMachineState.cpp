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

#include "mitkStateMachineState.h"

mitk::StateMachineState::StateMachineState(std::string stateName, std::string stateMode) :
    m_Name(stateName),  m_StateMode(stateMode)
{
}

std::string mitk::StateMachineState::GetMode()
{
  return m_StateMode;
}

mitk::StateMachineState::~StateMachineState()
{
  m_Transitions.clear();
}

bool mitk::StateMachineState::AddTransition(StateMachineTransition::Pointer transition)
{
  for (TransitionVector::iterator it = m_Transitions.begin(); it != m_Transitions.end(); ++it)
  {
    if (transition.GetPointer() == (*it).GetPointer())
      return false;
  }
  m_Transitions.push_back(transition);
  return true;
}

mitk::StateMachineTransition::Pointer mitk::StateMachineState::GetTransition(std::string eventClass, std::string eventVariant)
{
  mitk::StateMachineTransition::Pointer t = mitk::StateMachineTransition::New("", eventClass, eventVariant);
  for (TransitionVector::iterator it = m_Transitions.begin(); it != m_Transitions.end(); ++it)
  {
    if (**it == *t) // do not switch it and t, order matters, see  mitk::StateMachineTransition == operator
      return *it;
  }
  return NULL;
}

std::string mitk::StateMachineState::GetName()
{
  return m_Name;
}

//##Documentation
//## Post-processing step, when builing StateMachine from XML.
//## Parse all transitions and find the State that matches the String-Name.

bool mitk::StateMachineState::ConnectTransitions(StateMap *allStates)
{
  for (TransitionVector::iterator transIt = m_Transitions.begin(); transIt != m_Transitions.end(); ++transIt)
  {
    bool found = false;
    for (StateMap::iterator stateIt = allStates->begin(); stateIt != allStates->end(); ++stateIt)
    {
      if ((*stateIt)->GetName() == (*transIt)->GetNextStateName())
      {
        (*transIt)->SetNextState(*stateIt);
        found = true;
        break;
      }
    }
    if (!found)
    {
      MITK_WARN<< "Target State not found in StateMachine.";
      return false; // only reached if no state matching the string is found
    }
  }
  return true;
}
