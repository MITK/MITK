#include "StateMachine.h"
#include "StateMachineFactory.h"
#include "StateTransitionOperation.h"
#include "OperationEvent.h"
#include "UndoController.h"
#include "mitkInteractionConst.h"



//##ModelId=3E5B2DB301FD
//##Documentation
//## Constructor
//## daclares a new StateMachine and connects 
//## it to a StateMachine of Type type;
mitk::StateMachine::StateMachine(std::string type)
: m_Type(type)
{
	m_CurrentState = mitk::StateMachineFactory::GetStartState(type);
	m_UndoController = new UndoController(LIMITEDLINEARUNDO);//switch to LLU or add LLU
	m_UndoEnabled = true;
}

//##ModelId=3E5B2E660087
std::string mitk::StateMachine::GetType() const
{
	return m_Type;
}

//##ModelId=3E5B2DE30378
bool mitk::StateMachine::HandleEvent(StateEvent const* stateEvent, int objectEventId, int groupEventId)
{
	if (m_CurrentState == NULL)
		return false;//m_CurrentState needs to be set first!

//get Transition from m_Current State with equals ID
	const Transition *tempTransition = m_CurrentState->GetTransition(stateEvent->GetId());
	if (tempTransition == NULL) return false;
//get next State
	State *tempNextState = tempTransition->GetNextState();
	if (tempNextState == NULL) return false;
//and SideEffectId to execute later on
	int tempSideEffectId = tempTransition->GetSideEffectId();

	if ( ( m_CurrentState->GetId() != tempNextState->GetId() ) && ( m_UndoEnabled ) )	//write to UndoMechanism if there is a real statechange and Undo is enabled
	{
		//UNDO for this statechange
		StateTransitionOperation* doOp = new StateTransitionOperation(OpSTATECHANGE, tempNextState);
		StateTransitionOperation* undoOp = new StateTransitionOperation(OpSTATECHANGE, m_CurrentState);
		OperationEvent *operationEvent = new OperationEvent(((mitk::OperationActor*)(this)), 
															doOp, undoOp,
															objectEventId ,groupEventId);
		m_UndoController->SetOperationEvent(operationEvent);
	}

	//first following StateChange(or calling ExecuteOperation(tempNextStateOp)), then operation(SideEffect)
	m_CurrentState = tempNextState;	

	//Undo is handled in ExecuteSideEffect(...)
	bool ok = ExecuteSideEffect(tempSideEffectId, stateEvent, objectEventId, groupEventId);
		
	//Doku: if the operation doesn't work, then we have already changed the state. 
	//Ether go further without the success of the operation or undo the statechange.
	if (!ok && m_UndoEnabled)
	{
		ok = m_UndoController->Undo(true);//fine undo!
	}
	else if (!ok && !m_UndoEnabled)
	{
		std::cout<<"Error! Sender: StateMachine; Message: Operation could not be done!"<<std::endl;
	}

	return ok;
}
//##ModelId=3EDCAECB0175
void mitk::StateMachine::EnableUndo(bool enable)
{
	m_UndoEnabled = enable;
}

//##ModelId=3EF099EA03C0
int mitk::StateMachine::IncCurrGroupEventId()
{
	return mitk::OperationEvent::IncCurrGroupEventId();
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
				std::cout<<"Error! see StateMachine.cpp"<<std::endl;
				return;
			}
			m_CurrentState = stateTransOp->GetState();
		}
		break;
	default:
		NULL;
	}
}