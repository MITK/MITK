#include "StateMachine.h"

//##ModelId=3E5B2DB301FD
mitk::StateMachine::StateMachine(std::string type)
: m_Type(type), m_CurrentState(0)
{}

//##ModelId=3E5B2DE30378
bool mitk::StateMachine::HandleEvent(StateEvent const* stateEvent)
{
	const Transition *tempTransition = m_CurrentState->GetTransition(stateEvent->GetId());
	
	State *tempState = tempTransition->GetNextState();
	if (tempState == NULL)
		return false;
	
	int tempSideEffectId = tempTransition->GetSideEffectId();

	//remember UNDO!!!!!
	/*first StateChange, then operation*/
	m_CurrentState = tempState;

	//remember UNDO!!!!!
	bool ok = ExecuteSideEffect(tempSideEffectId);
	return ok;
}

//##ModelId=3E5B2E660087
std::string mitk::StateMachine::GetName() const
{
	return m_Type;
}

//##ModelId=3E5B2E170228
bool mitk::StateMachine::ExecuteSideEffect(int sideEffectId)
{
	return true;
}

