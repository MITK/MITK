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

#include "mitkStateMachine.h"
#include "mitkStateMachineFactory.h"
#include "mitkStateTransitionOperation.h"
#include "mitkInteractionConst.h"
#include "mitkInteractor.h"
#include "mitkTransition.h"
#include "mitkOperationEvent.h"
#include "mitkStateEvent.h"
#include "mitkAction.h"
#include "mitkUndoController.h"
#include <itkMacro.h>


/**
* @brief Constructor
* daclares a new StateMachine and connects
* it to a StateMachine of Type type;
**/
mitk::StateMachine::StateMachine(const char * type)
{
  if(type!=NULL)
  {
    m_Type = type;
    
    //the statemachine doesn't know yet anything about the number of timesteps of the data. So we initialize it with one element.
    this->InitializeStartStates(1);
  }
  /*else
    itkWarningMacro("Error from "<<this->GetNameOfClass()<<"; Message: Type of StateMachine is NULL!");*/
  //\*todo: check the process with BaseControllers, cause they are always instantiated with type ==NULL! So here we can't check and warn the user.
  //\*todo: @todo:why are the basecontrollers statemachines then??? <ingmar 18.12.06>
  
  m_UndoController = new UndoController(UndoController::VERBOSE_LIMITEDLINEARUNDO);//switch to LLU or add LLU
  m_UndoEnabled = true;

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
  mitk::State* startState = mitk::StateMachineFactory::GetStartState((const char *)(&m_Type[0]));

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

bool mitk::StateMachine::HandleEvent(StateEvent const* stateEvent)
{
  if (stateEvent == NULL)
    return false;

  if (m_CurrentStateVector.empty())
    return false;//m_CurrentStateVector needs to be initialized!

  if (m_TimeStep >= m_CurrentStateVector.size())
    return false;

  if (m_CurrentStateVector[m_TimeStep].IsNull())
    return false;//m_CurrentState needs to be initailized!

  //get the Transition from m_CurrentState which waits for this EventId
  const Transition *tempTransition = m_CurrentStateVector[m_TimeStep]->GetTransition(stateEvent->GetId());
  if (tempTransition == NULL) //no transition in this state for that EventId
  {
    return false;
  }

  //get next State
  State *tempNextState = tempTransition->GetNextState();
  if (tempNextState == NULL) //wrong built up statemachine!
    return false;

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
        itkWarningMacro("Error! see mitkStateMachine.cpp");
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
        itkWarningMacro("Error! see mitkStateMachine.cpp");
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
  State::Pointer startState = mitk::StateMachineFactory::GetStartState((const char *)(&m_Type[0]));
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
  unsigned int oldSize = m_CurrentStateVector.size();

  if ( timeSteps > oldSize )
  {
    State::Pointer startState = mitk::StateMachineFactory::GetStartState((const char *)(&m_Type[0]));
    for ( unsigned int i = oldSize; i < timeSteps; ++i )
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

