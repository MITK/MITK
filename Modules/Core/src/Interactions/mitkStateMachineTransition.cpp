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
#include "mitkEventFactory.h"
#include "mitkInteractionEventConst.h"
#include "mitkStateMachineAction.h"
#include "mitkStateMachineState.h"

mitk::StateMachineTransition::StateMachineTransition(const std::string &nextStateName,
                                                     const std::string &eventClass,
                                                     const std::string &eventVariant)
  : m_EventClass(eventClass), m_EventVariant(eventVariant), m_NextStateName(nextStateName)
{
  PropertyList::Pointer propertyList = PropertyList::New();
  propertyList->SetStringProperty(InteractionEventConst::xmlParameterEventClass().c_str(), eventClass.c_str());
  m_TransitionEvent = EventFactory::CreateEvent(propertyList);
}

bool mitk::StateMachineTransition::operator==(const StateMachineTransition &transition) const
{
  // Create event based on incoming event type,
  // then try to cast it to the type of event that this transition holds,
  // if this is possible, the variant is checked

  // check for nullptr since a corrupted state machine might cause an empty event
  if (m_TransitionEvent.IsNull())
  {
    return false;
  }
  PropertyList::Pointer propertyList = PropertyList::New();
  propertyList->SetStringProperty(InteractionEventConst::xmlParameterEventClass().c_str(),
                                  transition.m_EventClass.c_str());
  InteractionEvent::Pointer tmpEvent = EventFactory::CreateEvent(propertyList);
  if (tmpEvent.IsNull())
  {
    return false;
  }

  if (m_TransitionEvent->IsSuperClassOf(tmpEvent.GetPointer()))
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
      MITK_WARN << "Event type in Statemachine " << m_EventClass << " is not compatible to configuration class "
                << transition.m_EventClass;
    }
    return false;
  }
}

mitk::StateMachineTransition::~StateMachineTransition()
{
  // needed for correct reference counting of mitkState//, for real???
  m_NextState = nullptr;

  m_Actions.clear();
  m_Conditions.clear();
}

void mitk::StateMachineTransition::AddAction(const StateMachineAction::Pointer &action)
{
  m_Actions.push_back(action);
}

void mitk::StateMachineTransition::AddCondition(const StateMachineCondition &condition)
{
  m_Conditions.push_back(condition);
}

mitk::StateMachineState::Pointer mitk::StateMachineTransition::GetNextState() const
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

const mitk::ConditionVectorType &mitk::StateMachineTransition::GetConditions() const
{
  return m_Conditions;
}

void mitk::StateMachineTransition::SetNextState(const SpStateMachineState &nextState)
{
  m_NextState = nextState;
}
