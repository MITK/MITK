#include "mitkStateMachine.h"
#include "mitkStateMachineFactory.h"
#include "mitkStateTransitionOperation.h"
#include "mitkOperationEvent.h"
#include "mitkUndoController.h"
#include "mitkStatusBar.h"
#include "mitkInteractionConst.h"
#include <itkMacro.h>
#include "mitkInteractor.h"


//##ModelId=3E5B2DB301FD
//##Documentation
//## Constructor
//## daclares a new StateMachine and connects
//## it to a StateMachine of Type type;
mitk::StateMachine::StateMachine(const char * type) : m_CurrentState(NULL)
{
  if(type!=NULL)
  {
    m_Type = type;
	  m_CurrentState = mitk::StateMachineFactory::GetStartState(type);
  }
  else
    mitk::StatusBar::DisplayText("Error! Sender: StateMachine; Message: Type of StateMachine is NULL!", 10000);
  
  m_UndoController = new UndoController(LIMITEDLINEARUNDO);//switch to LLU or add LLU
	m_UndoEnabled = true;
}

//##ModelId=3E5B2E660087
std::string mitk::StateMachine::GetType() const
{
	return m_Type;
}

const mitk::State* mitk::StateMachine::GetCurrentState() const
{
  if (m_CurrentState)
    return m_CurrentState;
  return NULL;
}

//##ModelId=3E5B2DE30378
bool mitk::StateMachine::HandleEvent(StateEvent const* stateEvent)
{
  if (m_CurrentState == NULL)
  return false;//m_CurrentState needs to be set first!

  //get the Transition from m_CurrentState which waits for this EventId
  const Transition *tempTransition = m_CurrentState->GetTransition(stateEvent->GetId());
  if (tempTransition == NULL) //no transition in this state for that EventId
  {
    return false;
  }

  //get next State
  State *tempNextState = tempTransition->GetNextState();
  if (tempNextState == NULL) //wrong built up statemachine!
    return false;

  //and ActionId to execute later on
  if ( m_CurrentState->GetId() != tempNextState->GetId() )//statechange only if there is a real statechange
  {
    if ( m_UndoEnabled )	//write to UndoMechanism if Undo is enabled
    {
      //UNDO for this statechange
	  StateTransitionOperation* doOp = new StateTransitionOperation(OpSTATECHANGE, tempNextState);
    StateTransitionOperation* undoOp = new StateTransitionOperation(OpSTATECHANGE, m_CurrentState);
	  OperationEvent *operationEvent = new OperationEvent(((mitk::OperationActor*)(this)), doOp, undoOp);
	  m_UndoController->SetOperationEvent(operationEvent);
    }

            #ifdef INTERDEBUG
            //Debug StateChanges through cout output! Thus very slow!
            std::cout<<this->GetType()<<": Changing from StateId "<<m_CurrentState->GetId()<<" to StateId "<<tempNextState->GetId()<<std::endl;
            std::cout<<this->GetType()<<": Changing from State "<<m_CurrentState->GetName()<<" to State "<<tempNextState->GetName()<<std::endl;
            #endif

    //first following StateChange(or calling ExecuteOperation(tempNextStateOp)), then operation(action)
    m_CurrentState = tempNextState;
  }

  std::vector<Action*>::iterator actionIdIterator = tempTransition->GetActionBeginIterator();
  const std::vector<Action*>::iterator actionIdIteratorEnd = tempTransition->GetActionEndIterator();
  bool ok = true;

  while ( actionIdIterator != actionIdIteratorEnd ) 
  {
    if ( !ExecuteAction(*actionIdIterator, stateEvent) )
    {
      #ifdef INTERDEBUG
      itkWarningMacro( << "Warning: no action defind for " << *actionIdIterator << " in " << m_Type );
      #endif

      ok = false;
    }
    actionIdIterator++;
  }
  return ok;
}

//##ModelId=3EDCAECB0175
void mitk::StateMachine::EnableUndo(bool enable)
{
	m_UndoEnabled = enable;
}

//##ModelId=3EF099EA03C0
void mitk::StateMachine::IncCurrGroupEventId()
{
	mitk::OperationEvent::IncCurrGroupEventId();
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
				mitk::StatusBar::DisplayText("Error! see mitkStateMachine.cpp", 10000);
				return;
			}
#ifdef INTERDEBUG
//Debug StateChanges through cout output! Thus very slow!
std::cout<<this->GetType()<<": Undo: Changing from StateId "<<m_CurrentState->GetId()<<" to StateId "<<stateTransOp->GetState()->GetId()<<std::endl;
std::cout<<this->GetType()<<": Undo: Changing from State "<<m_CurrentState->GetName()<<" to State "<<stateTransOp->GetState()->GetName()<<std::endl;
#endif
			m_CurrentState = stateTransOp->GetState();
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
      //this is just new! and now the m_CurrentState has to be set on a special State
      //that way a delete of a StateMachine can be undone 
      //IMPORTANT: The type has to be the same!!!Done by a higher instance, that creates this!
      mitk::StateTransitionOperation* stateTransOp = dynamic_cast<mitk::StateTransitionOperation *>(operation);
			if (stateTransOp != NULL)
      {
        m_CurrentState = stateTransOp->GetState();
      }
    }
	default:
		;
	}
}
