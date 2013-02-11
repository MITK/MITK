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

#include "mitkEventStateMachine.h"
#include "mitkStateMachineContainer.h"
#include "mitkInteractionEvent.h"
#include "mitkStateMachineAction.h"
#include "mitkStateMachineTransition.h"
#include "mitkStateMachineState.h"
// us
#include "mitkModule.h"
#include "mitkModuleResource.h"
#include "mitkModuleResourceStream.h"
#include "mitkModuleRegistry.h"

mitk::EventStateMachine::EventStateMachine()
{
  m_StateMachineContainer = NULL;
  m_CurrentState = NULL;
}

bool mitk::EventStateMachine::LoadStateMachine(std::string filename, std::string moduleName)
{
  if (m_StateMachineContainer != NULL)
  {
    m_StateMachineContainer->Delete();
  }
  m_StateMachineContainer = StateMachineContainer::New();

  if (m_StateMachineContainer->LoadBehavior(filename,moduleName))
  {
    m_CurrentState = m_StateMachineContainer->GetStartState();

    // clear actions map ,and connect all actions as declared in sub-class
    m_ActionFunctionsMap.clear();
    ConnectActionsAndFunctions();
    return true;
  }
  else
  {
    MITK_WARN<< "Unable to load StateMachine from file: " << filename;
    return false;
  }
}

mitk::EventStateMachine::~EventStateMachine()
{
  if (m_StateMachineContainer != NULL)
  {
    m_StateMachineContainer->Delete();
  }
}

void mitk::EventStateMachine::AddActionFunction(std::string action, mitk::TActionFunctor* functor)
{
  if (!functor)
    return;
// make sure double calls for same action won't cause memory leaks
  delete m_ActionFunctionsMap[action];
  m_ActionFunctionsMap[action] = functor;
}

bool mitk::EventStateMachine::HandleEvent(InteractionEvent* event, DataNode* dataNode)
{
  if (!FilterEvents(event, dataNode))
  {
    return false;
  }
  // check if the current state holds a transition that works with the given event.
  StateMachineTransition::Pointer transition = m_CurrentState->GetTransition(event->GetEventClass(), MapToEventVariant(event));

  if (transition.IsNotNull())
  {
    // iterate over all actions in this transition and execute them
    ActionVectorType actions = transition->GetActions();
    bool success = false;
    for (ActionVectorType::iterator it = actions.begin(); it != actions.end(); ++it)
    {

      success |= ExecuteAction(*it, event); // treat an event as handled if at least one of the actions is executed successfully
    }
    if (success || actions.empty())  // an empty action list is always successful
    {
      // perform state change
      m_CurrentState = transition->GetNextState();
      //MITK_INFO<< "StateChange: " << m_CurrentState->GetName();
    }
    return success;
  }
  else
  {
    return false; // no transition found that matches event
  }
}

void mitk::EventStateMachine::ConnectActionsAndFunctions()
{
  MITK_WARN<< "ConnectActionsAndFunctions in DataInteractor not implemented.\n DataInteractor will not be able to process any events.";
}

bool mitk::EventStateMachine::ExecuteAction(StateMachineAction* action, InteractionEvent* event)
{

  if (action == NULL)
  {
    return false;
  }
  // Maps Action-Name to Functor and executes DoAction on Functor.
  TActionFunctor* actionFunction = m_ActionFunctionsMap[action->GetActionName()];
  if (actionFunction == NULL)
  {
    return false;
  }
  bool retVal = actionFunction->DoAction(action, event);
  return retVal;
}

mitk::StateMachineState* mitk::EventStateMachine::GetCurrentState()
{
  return m_CurrentState.GetPointer();
}

bool mitk::EventStateMachine::FilterEvents(InteractionEvent* interactionEvent, DataNode* dataNode)
{
  if (dataNode == NULL) {
    MITK_WARN << "EventStateMachine: Empty DataNode received along with this Event " << interactionEvent;
    return false;
  }
  bool visible = false;
  if (dataNode->GetPropertyList()->GetBoolProperty("visible", visible) == false)
  { //property doesn't exist
    return false;
  }
  return visible;
}
