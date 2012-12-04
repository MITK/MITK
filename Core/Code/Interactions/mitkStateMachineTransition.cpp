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

#include "mitkStateMachineTransition.h"
#include "mitkStateMachineAction.h"
#include "mitkStateMachineState.h"

mitk::StateMachineTransition::StateMachineTransition(std::string nextStateName, std::string eventClass, std::string eventVariant) :
     m_EventClass(eventClass), m_EventVariant(eventVariant), m_NextStateName(nextStateName)
{
}

bool mitk::StateMachineTransition::operator ==(const StateMachineTransition& transition)
{
  return (this->m_EventClass==transition.m_EventClass && this->m_EventVariant==transition.m_EventVariant);
}

mitk::StateMachineTransition::~StateMachineTransition()
{
  //needed for correct reference counting of mitkState//, for real???
  m_NextState = NULL;
  m_Actions.clear();
}

void mitk::StateMachineTransition::AddAction(StateMachineAction::Pointer action)
{
  m_Actions.push_back(action);
}

mitk::StateMachineState::Pointer mitk::StateMachineTransition::GetNextState()
{
  return m_NextState;
}

std::string mitk::StateMachineTransition::GetNextStateName() const
{
  return m_NextStateName;
}

std::vector<mitk::StateMachineAction::Pointer> mitk::StateMachineTransition::GetActions() const
{
  return m_Actions;
}

bool mitk::StateMachineTransition::MatchesEvent(std::string eventClass, std::string eventVariant) const
{
  return (m_EventClass == eventClass && m_EventVariant == eventVariant);
}

void mitk::StateMachineTransition::SetNextState(SpStateMachineState nextState)
{
  m_NextState = nextState;
}


