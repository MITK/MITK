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
#include "mitkApplicationCursor.h"
#include "mitkInteractionEvent.h"
#include "mitkStateMachineAction.h"
#include "mitkStateMachineCondition.h"
#include "mitkStateMachineContainer.h"
#include "mitkStateMachineState.h"
#include "mitkStateMachineTransition.h"
#include "mitkUndoController.h"

mitk::EventStateMachine::EventStateMachine()
  : m_IsActive(true),
    m_UndoController(nullptr),
    m_StateMachineContainer(nullptr),
    m_CurrentState(nullptr),
    m_MouseCursorSet(false)
{
  if (!m_UndoController)
  {
    m_UndoController = new UndoController(UndoController::VERBOSE_LIMITEDLINEARUNDO); // switch to LLU or add LLU

    /**
    * here the Undo mechanism is enabled / disabled for all interactors.
    **/
    m_UndoEnabled = true;
  }
}

bool mitk::EventStateMachine::LoadStateMachine(const std::string &filename, const us::Module *module)
{
  if (m_StateMachineContainer != nullptr)
  {
    m_StateMachineContainer->Delete();
  }
  m_StateMachineContainer = StateMachineContainer::New();

  if (m_StateMachineContainer->LoadBehavior(filename, module))
  {
    m_CurrentState = m_StateMachineContainer->GetStartState();

    for (auto i = m_ConditionDelegatesMap.begin(); i != m_ConditionDelegatesMap.end();
         ++i)
    {
      delete i->second;
    }
    m_ConditionDelegatesMap.clear();

    // clear actions map ,and connect all actions as declared in sub-class
    for (auto i = m_ActionFunctionsMap.begin();
         i != m_ActionFunctionsMap.end();
         ++i)
    {
      delete i->second;
    }
    m_ActionFunctionsMap.clear();
    for (auto i = m_ActionDelegatesMap.begin(); i != m_ActionDelegatesMap.end(); ++i)
    {
      delete i->second;
    }
    m_ActionDelegatesMap.clear();

    ConnectActionsAndFunctions();
    return true;
  }
  else
  {
    MITK_WARN << "Unable to load StateMachine from file: " << filename;
    return false;
  }
}

mitk::EventStateMachine::~EventStateMachine()
{
  if (m_StateMachineContainer != nullptr)
  {
    m_StateMachineContainer->Delete();
  }
}

void mitk::EventStateMachine::AddActionFunction(const std::string &action, mitk::TActionFunctor *functor)
{
  if (!functor)
    return;
  // make sure double calls for same action won't cause memory leaks
  delete m_ActionFunctionsMap[action];
  auto i = m_ActionDelegatesMap.find(action);
  if (i != m_ActionDelegatesMap.end())
  {
    delete i->second;
    m_ActionDelegatesMap.erase(i);
  }
  m_ActionFunctionsMap[action] = functor;
}

void mitk::EventStateMachine::AddActionFunction(const std::string &action, const ActionFunctionDelegate &delegate)
{
  auto i = m_ActionFunctionsMap.find(action);
  if (i != m_ActionFunctionsMap.end())
  {
    delete i->second;
    m_ActionFunctionsMap.erase(i);
  }

  delete m_ActionDelegatesMap[action];
  m_ActionDelegatesMap[action] = delegate.Clone();
}

void mitk::EventStateMachine::AddConditionFunction(const std::string &condition,
                                                   const ConditionFunctionDelegate &delegate)
{
  m_ConditionDelegatesMap[condition] = delegate.Clone();
}

bool mitk::EventStateMachine::HandleEvent(InteractionEvent *event, DataNode *dataNode)
{
  if (!m_IsActive)
    return false;

  if (!FilterEvents(event, dataNode))
  {
    return false;
  }

  // Get the transition that can be executed
  mitk::StateMachineTransition::Pointer transition = GetExecutableTransition(event);

  // check if the current state holds a transition that works with the given event.
  if (transition.IsNotNull())
  {
    // all conditions are fulfilled so we can continue with the actions
    m_CurrentState = transition->GetNextState();

    // iterate over all actions in this transition and execute them
    const ActionVectorType actions = transition->GetActions();
    for (auto it = actions.cbegin(); it != actions.cend(); ++it)
    {
      try
      {
        ExecuteAction(*it, event);
      }
      catch (const std::exception &e)
      {
        MITK_ERROR << "Unhandled excaption caught in ExecuteAction(): " << e.what();
        return false;
      }
      catch (...)
      {
        MITK_ERROR << "Unhandled excaption caught in ExecuteAction()";
        return false;
      }
    }

    return true;
  }
  return false;
}

void mitk::EventStateMachine::ConnectActionsAndFunctions()
{
  MITK_WARN << "ConnectActionsAndFunctions in DataInteractor not implemented.\n DataInteractor will not be able to "
               "process any events.";
}

bool mitk::EventStateMachine::CheckCondition(const StateMachineCondition &condition, const InteractionEvent *event)
{
  bool retVal = false;
  ConditionDelegatesMapType::const_iterator delegateIter = m_ConditionDelegatesMap.find(condition.GetConditionName());
  if (delegateIter != m_ConditionDelegatesMap.cend())
  {
    retVal = delegateIter->second->Execute(event);
  }
  else
  {
    MITK_WARN << "No implementation of condition '" << condition.GetConditionName() << "' has been found.";
  }

  return retVal;
}

void mitk::EventStateMachine::ExecuteAction(StateMachineAction *action, InteractionEvent *event)
{
  if (action == nullptr)
  {
    return;
  }

  // Maps Action-Name to Functor and executes the Functor.
  ActionDelegatesMapType::const_iterator delegateIter = m_ActionDelegatesMap.find(action->GetActionName());
  if (delegateIter != m_ActionDelegatesMap.cend())
  {
    delegateIter->second->Execute(action, event);
  }
  else
  {
    // try the legacy system
    std::map<std::string, TActionFunctor *>::const_iterator functionIter =
      m_ActionFunctionsMap.find(action->GetActionName());
    if (functionIter != m_ActionFunctionsMap.cend())
    {
      functionIter->second->DoAction(action, event);
    }
    else
    {
      MITK_WARN << "No implementation of action '" << action->GetActionName() << "' has been found.";
    }
  }
}

mitk::StateMachineState *mitk::EventStateMachine::GetCurrentState() const
{
  return m_CurrentState.GetPointer();
}

bool mitk::EventStateMachine::FilterEvents(InteractionEvent *interactionEvent, DataNode *dataNode)
{
  if (dataNode == nullptr)
  {
    MITK_WARN << "EventStateMachine: Empty DataNode received along with this Event " << interactionEvent;
    return false;
  }
  bool visible = false;
  if (dataNode->GetBoolProperty("visible", visible, interactionEvent->GetSender()) == false)
  { // property doesn't exist
    return false;
  }
  return visible;
}

mitk::StateMachineTransition *mitk::EventStateMachine::GetExecutableTransition(mitk::InteractionEvent *event)
{
  // Map that will contain all conditions that are possibly used by the
  // transitions
  std::map<std::string, bool> conditionsMap;

  // Get a list of all transitions that match the given event
  const mitk::StateMachineState::TransitionVector transitionList =
    m_CurrentState->GetTransitionList(event->GetNameOfClass(), MapToEventVariant(event));

  // if there are not transitions, we can return nullptr here.
  if (transitionList.empty())
  {
    return nullptr;
  }

  StateMachineState::TransitionVector::const_iterator transitionIter;
  ConditionVectorType::const_iterator conditionIter;
  for (transitionIter = transitionList.cbegin(); transitionIter != transitionList.cend(); ++transitionIter)
  {
    bool allConditionsFulfilled(true);

    // Get all conditions for the current transition
    const ConditionVectorType conditions = (*transitionIter)->GetConditions();
    for (conditionIter = conditions.cbegin(); conditionIter != conditions.cend(); ++conditionIter)
    {
      bool currentConditionFulfilled(false);

      // sequentially check all conditions that we have evaluated above
      const std::string conditionName = (*conditionIter).GetConditionName();

      // Check if the condition has already been evaluated
      if (conditionsMap.find(conditionName) == conditionsMap.cend())
      {
        // if the condition has not been evaluated yet, do it now and store
        // the result in the map
        try
        {
          currentConditionFulfilled = CheckCondition((*conditionIter), event);
          conditionsMap.insert(std::pair<std::string, bool>(conditionName, currentConditionFulfilled));
        }
        catch (const std::exception &e)
        {
          MITK_ERROR << "Unhandled excaption caught in CheckCondition(): " << e.what();
          currentConditionFulfilled = false;
          break;
        }
        catch (...)
        {
          MITK_ERROR << "Unhandled excaption caught in CheckCondition()";
          currentConditionFulfilled = false;
          break;
        }
      }
      else
      {
        // if the condition has been evaluated before, use that result
        currentConditionFulfilled = conditionsMap[conditionName];
      }

      // set 'allConditionsFulfilled' under consideration of a possible
      // inversion of the condition
      if (currentConditionFulfilled == (*conditionIter).IsInverted())
      {
        allConditionsFulfilled = false;
        break;
      }
    }

    // If all conditions are fulfilled, we execute this transition
    if (allConditionsFulfilled)
    {
      return (*transitionIter);
    }
  }

  // We have found no transition that can be executed, return nullptr
  return nullptr;
}

void mitk::EventStateMachine::ResetToStartState()
{
  m_CurrentState = m_StateMachineContainer->GetStartState();
}

void mitk::EventStateMachine::SetMouseCursor(const char *xpm[], int hotspotX, int hotspotY)
{
  // Remove previously set mouse cursor
  if (m_MouseCursorSet)
  {
    ApplicationCursor::GetInstance()->PopCursor();
  }

  ApplicationCursor::GetInstance()->PushCursor(xpm, hotspotX, hotspotY);
  m_MouseCursorSet = true;
}

void mitk::EventStateMachine::ResetMouseCursor()
{
  if (m_MouseCursorSet)
  {
    ApplicationCursor::GetInstance()->PopCursor();
    m_MouseCursorSet = false;
  }
}
