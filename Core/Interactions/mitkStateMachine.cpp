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
bool mitk::StateMachine::HandleEvent(StateEvent const* stateEvent)
{
	if (m_CurrentState == NULL)
		return false;//m_CurrentState needs to be set first!

//get Transition from m_Current State with equals ID
	const Transition *tempTransition = m_CurrentState->GetTransition(stateEvent->GetId());
	if (tempTransition == NULL)
		return false;
//get next State
	State *tempState = tempTransition->GetNextState();
	if (tempState == NULL)
		return false;
//and SideEffectId to execute later on
	int tempSideEffectId = tempTransition->GetSideEffectId();

	if (m_UndoEnabled)	//write to UndoMechanism
	{
		//UNDO StateTransitionOperation
		StateTransitionOperation* undoSTO = new StateTransitionOperation(mitk::STATETRANSITION, m_CurrentState);
		StateTransitionOperation* redoSTO = new StateTransitionOperation(mitk::STATETRANSITION, tempState);//tempStateZeiger? mögl. Fehlerquelle
	
		OperationEvent *operationEvent = new OperationEvent(((mitk::OperationActor*)(this)), undoSTO, redoSTO, 
																OperationEvent::GenerateObjectEventId(), 
																OperationEvent::GenerateGroupEventId());

		
		m_UndoController->SwitchUndoModel(LIMITEDLINEARUNDO);
		m_UndoController->SetOperationEvent(operationEvent);
	}

	//first following StateChange, then operation(SideEffect)
	m_CurrentState = tempState;
	bool ok = ExecuteSideEffect(tempSideEffectId, stateEvent);//Undo in ExecuteSideEffect(...)
	
	//Doku: if the operation doesn't work, then we have already changed the state. 
	//Ether go further without the success of the operation or undo the statechange.
	if (!ok && m_UndoEnabled)
	{
		ok = m_UndoController->Undo();
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

