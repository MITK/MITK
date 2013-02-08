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
#include "mitkEventFactory.h"
#include "mitkInteractionEventConst.h"

mitk::StateMachineTransition::StateMachineTransition(std::string nextStateName, std::string eventClass, std::string eventVariant) :
    m_EventClass(eventClass), m_EventVariant(eventVariant), m_NextStateName(nextStateName)
{
  PropertyList::Pointer propertyList = PropertyList::New();
  propertyList->SetStringProperty(xmlParameterEventClass.c_str(), eventClass.c_str());
  m_TransitionEvent = EventFactory::CreateEvent(propertyList);
}

bool mitk::StateMachineTransition::operator ==(const StateMachineTransition& transition)
{
  // Create event based on incoming event type,
  // then try to cast it to the type of event that this transition holds,
  // if this is possible, the variant is checked

  // check for NULL since a corrupted state machine might cause an empty event
  if (m_TransitionEvent.IsNull())
  {
    return false;
  }
  PropertyList::Pointer propertyList = PropertyList::New();
  propertyList->SetStringProperty(xmlParameterEventClass.c_str(), transition.m_EventClass.c_str());
  InteractionEvent::Pointer tmpEvent = EventFactory::CreateEvent(propertyList);

  if (tmpEvent->IsSubClassOf(m_TransitionEvent.GetPointer()))
  {
    return (this->m_EventVariant == transition.m_EventVariant);
  }
  else
  {
    // if event variants match, but super class condition is violated
    // this means that the configuration file, implements a class that does not
    // support the type in the state machine.
    if (this->m_EventVariant == transition.m_EventVariant)
    {
      MITK_WARN<< "Event type in Statemachine " << m_EventClass << " is not compatible to configuration class " << transition.m_EventClass;
    }
    return false;
  }
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

void mitk::StateMachineTransition::SetNextState(SpStateMachineState nextState)
{
  m_NextState = nextState;
}
