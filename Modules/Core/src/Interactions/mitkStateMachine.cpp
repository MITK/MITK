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

#include "mitkStateMachine.h"
#include "mitkStateTransitionOperation.h"
#include "mitkInteractionConst.h"
#include "mitkInteractor.h"
#include "mitkTransition.h"
#include "mitkOperationEvent.h"
#include "mitkStateEvent.h"
#include "mitkAction.h"
#include "mitkUndoController.h"
#include <itkMacro.h>
#include "mitkGlobalInteraction.h"
#include <mbilog.h>


/**
* @brief Constructor
* daclares a new StateMachine and connects
* it to a StateMachine of Type type;
* Also the undo mechanism is instanciated and enabled/disabled
**/
mitk::StateMachine::StateMachine(const char * type)
: m_UndoController(NULL), m_Type("")
{
  if(type!=NULL) //no need to throw a warning here, because the statemachine yet here doesn't have to be set up.
  {
    m_Type = type;

    //the statemachine doesn't know yet anything about the number of timesteps of the data. So we initialize it with one element.
    this->InitializeStartStates(1);
  }

  if (!m_UndoController)
  {
    m_UndoController = new UndoController(UndoController::VERBOSE_LIMITEDLINEARUNDO);//switch to LLU or add LLU

    /**
    * here the Undo mechanism is enabled / disabled for all interactors.
    **/
    m_UndoEnabled = true;
  }

  m_TimeStep = 0;
}

mitk::StateMachine::~StateMachine()
{
  //clean up map using deletes
  for ( mitk::StateMachine::ActionFunctionsMapType::iterator iter = m_ActionFunctionsMap.begin();
        iter != m_ActionFunctionsMap.end();
        ++iter )
  {
    delete iter->second;
  }

  delete m_UndoController;
}

std::string mitk::StateMachine::GetType() const
{
  return m_Type;
}

const mitk::State* mitk::StateMachine::GetCurrentState(unsigned int timeStep) const
{
  if (m_CurrentStateVector.size() > timeStep) //the size of the vector has to be one integer higher than the timeStep.
    return m_CurrentStateVector[timeStep].GetPointer();
  return NULL;
}

void mitk::StateMachine::ResetStatemachineToStartState(unsigned int timeStep)
{
  mitk::State* startState = mitk::GlobalInteraction::GetInstance()->GetStartState((const char *)(&m_Type[0]));

  if ( m_UndoEnabled )  //write to UndoMechanism if Undo is enabled
  {
    //UNDO for this statechange;
    StateTransitionOperation* doOp = new StateTransitionOperation(OpSTATECHANGE, startState, timeStep);
    StateTransitionOperation* undoOp = new StateTransitionOperation(OpSTATECHANGE, m_CurrentStateVector[timeStep], timeStep);
    OperationEvent *operationEvent = new OperationEvent(((mitk::OperationActor*)(this)), doOp, undoOp);
    m_UndoController->SetOperationEvent(operationEvent);
  }
  //can be done without calling this->ExecuteOperation()
  m_CurrentStateVector[timeStep] = startState;

}

float mitk::StateMachine::CanHandleEvent(const StateEvent* /*stateEvent*/) const
{
  return 0.5;
}

bool mitk::StateMachine::HandleEvent(StateEvent const* stateEvent)
{
  if (stateEvent == NULL)
    return false;

  if (m_CurrentStateVector.empty())
  {
    STATEMACHINE_ERROR << "Error in mitkStateMachine.cpp: StateMachine not initialized!\n";
    return false;//m_CurrentStateVector needs to be initialized!
  }

  if (m_TimeStep >= m_CurrentStateVector.size())
  {
    STATEMACHINE_ERROR << "Error in mitkStateMachine.cpp: StateMachine not initialized for this time step!\n";
    return false;
  }

  if (m_CurrentStateVector[m_TimeStep].IsNull())
  {
    STATEMACHINE_ERROR << "Error in mitkStateMachine.cpp: StateMachine not initialized with the right temporal information!\n";
    return false;//m_CurrentState needs to be initialized!
  }

  //get the Transition from m_CurrentState which waits for this EventId
  const Transition *tempTransition = m_CurrentStateVector[m_TimeStep]->GetTransition(stateEvent->GetId());
  if (tempTransition == NULL) //no transition in this state for that EventId
  {
    return false;
  }

  //get next State
  State *tempNextState = tempTransition->GetNextState();
  if (tempNextState == NULL) //wrong built up statemachine!
  {
    STATEMACHINE_ERROR << "Error in mitkStateMachine.cpp: StateMachinePattern not defined correctly!\n";
    return false;
  }

  //and ActionId to execute later on
  if ( m_CurrentStateVector[m_TimeStep]->GetId() != tempNextState->GetId() )//statechange only if there is a real statechange
  {
    if ( m_UndoEnabled )  //write to UndoMechanism if Undo is enabled
    {
      //UNDO for this statechange; since we directly change the state, we don't need the do-Operation in case m_UndoEnables == false
      StateTransitionOperation* doOp = new StateTransitionOperation(OpSTATECHANGE, tempNextState, m_TimeStep);
      StateTransitionOperation* undoOp = new StateTransitionOperation(OpSTATECHANGE, m_CurrentStateVector[m_TimeStep], m_TimeStep);
      OperationEvent *operationEvent = new OperationEvent(((mitk::OperationActor*)(this)), doOp, undoOp);
      m_UndoController->SetOperationEvent(operationEvent);
    }

    STATEMACHINE_DEBUG << "from " << m_CurrentStateVector[m_TimeStep]->GetId()
                       << " " << m_CurrentStateVector[m_TimeStep]->GetName()
                       << " to " << tempNextState->GetId() <<" "<<tempNextState->GetName();

    //first following StateChange(or calling ExecuteOperation(tempNextStateOp)), then operation(action)
    m_CurrentStateVector[m_TimeStep] = tempNextState;
  }

  mitk::Transition::ActionVectorIterator actionIdIterator = tempTransition->GetActionBeginIterator();
  mitk::Transition::ActionVectorConstIterator actionIdIteratorEnd = tempTransition->GetActionEndIterator();
  bool ok = true;

  while ( actionIdIterator != actionIdIteratorEnd )
  {
    if ( !ExecuteAction(*actionIdIterator, stateEvent) )
    {
      ok = false;
    }
    actionIdIterator++;
  }
  return ok;
}

void mitk::StateMachine::EnableUndo(bool enable)
{
  m_UndoEnabled = enable;
}

void mitk::StateMachine::IncCurrGroupEventId()
{
  mitk::OperationEvent::IncCurrGroupEventId();
}

/// look up which object method is associated to the given action and call the method
bool mitk::StateMachine::ExecuteAction(Action* action, StateEvent const* stateEvent)
{
  if (!action) return false;

  int actionId = action->GetActionId();

  TStateMachineFunctor* actionFunction = m_ActionFunctionsMap[actionId];
  if (!actionFunction) return false;

  bool retVal = actionFunction->DoAction(action, stateEvent);
  return retVal;
}

void mitk::StateMachine::AddActionFunction(int action, mitk::TStateMachineFunctor* functor)
{
  if (!functor) return;

  // make sure double calls for same action won't cause memory leaks
  delete m_ActionFunctionsMap[action]; // delete NULL does no harm

  m_ActionFunctionsMap[action] = functor;
}


void mitk::StateMachine::ExecuteOperation(Operation* operation)
{
  switch (operation->GetOperationType())
  {
  case OpNOTHING:
    break;
  case OpSTATECHANGE:
    {
      mitk::StateTransitionOperation* stateTransOp = dynamic_cast<mitk::StateTransitionOperation *>(operation);
      if (stateTransOp == NULL)
      {
        STATEMACHINE_WARN<<"Error! see mitkStateMachine.cpp";
        return;
      }
      unsigned int time = stateTransOp->GetTime();
      m_CurrentStateVector[time] = stateTransOp->GetState();
    }
    break;
  case OpTIMECHANGE:
    {
      mitk::StateTransitionOperation* stateTransOp = dynamic_cast<mitk::StateTransitionOperation *>(operation);
      if (stateTransOp == NULL)
      {
        STATEMACHINE_WARN<<"Error! see mitkStateMachine.cpp";
        return;
      }
      m_TimeStep = stateTransOp->GetTime();
    }
    break;
  case OpDELETE:
    {
      //delete this!
      //before all lower statemachines has to be deleted in a action
      //this->Delete();//might not work!!!check itk!
    }
  case OpUNDELETE:
    {
      //now the m_CurrentState has to be set on a special State
      //that way a delete of a StateMachine can be undone
      //IMPORTANT: The type has to be the same!!!Done by a higher instance, that creates this!
      mitk::StateTransitionOperation* stateTransOp = dynamic_cast<mitk::StateTransitionOperation *>(operation);
      if (stateTransOp != NULL)
      {
        unsigned int time = stateTransOp->GetTime();
        m_CurrentStateVector[time] = stateTransOp->GetState();
      }
    }
  default:
    ;
  }
}

void mitk::StateMachine::InitializeStartStates(unsigned int timeSteps)
{
  //get the startstate of the pattern
  State::Pointer startState = mitk::GlobalInteraction::GetInstance()->GetStartState(m_Type.c_str());

  if (startState.IsNull())
  {
    STATEMACHINE_FATAL << "Fatal Error in mitkStateMachine.cpp: Initialization of statemachine unsuccessfull! Initialize GlobalInteraction!\n";
  }

  //clear the vector
  m_CurrentStateVector.clear();
  //add n=timesteps pointers pointing to to the startstate
  for (unsigned int i = 0; i < timeSteps; i++)
    m_CurrentStateVector.push_back(startState);
}

// Check if the vector is long enough to contain the new element
// at the given position. If not, expand it with sufficient pre-initialized
// elements.
//
// NOTE: This method will never REDUCE the vector size; it should only
// be used to make sure that the vector has enough elements to include the
// specified time step.
void mitk::StateMachine::ExpandStartStateVector(unsigned int timeSteps)
{
  size_t oldSize = m_CurrentStateVector.size();

  if ( timeSteps > oldSize )
  {
    State::Pointer startState = mitk::GlobalInteraction::GetInstance()->GetStartState(m_Type.c_str());
    for ( size_t i = oldSize; i < timeSteps; ++i )
      m_CurrentStateVector.insert(m_CurrentStateVector.end(), startState);
  }
}

void mitk::StateMachine::UpdateTimeStep(unsigned int timeStep)
{
  //don't need to fill up the memory if the time is uptodate
  if (timeStep == m_TimeStep)
    return;

  //create an operation that changes the time and send it to undocontroller
  StateTransitionOperation* doOp = new StateTransitionOperation(OpTIMECHANGE, NULL, timeStep);
  if ( m_UndoEnabled )  //write to UndoMechanism if Undo is enabled
  {
    StateTransitionOperation* undoOp = new StateTransitionOperation(OpTIMECHANGE, NULL, m_TimeStep);
    OperationEvent *operationEvent = new OperationEvent(((mitk::OperationActor*)(this)), doOp, undoOp);
    m_UndoController->SetOperationEvent(operationEvent);
  }
  this->ExecuteOperation(doOp);
}

