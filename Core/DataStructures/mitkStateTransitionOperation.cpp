#include "StateTransitionOperation.h"

//##ModelId=3E78311303E7
mitk::StateTransitionOperation::StateTransitionOperation(OperationType operationType, int execId, State* state)
: mitk::Operation(operationType, execId), m_State(state)
{}

//##ModelId=3E78307601D8
mitk::State* mitk::StateTransitionOperation::GetState()
{
	return m_State;
}

//##ModelId=3EDDD7640192
void mitk::StateTransitionOperation::Execute(int eventId)//wrong with eventId!!!
{
	//Beispiel für ein Statewechsel durch eventId 
	const mitk::Transition *transition = m_State->GetTransition(eventId);
	m_State = transition->GetNextState();
}

//##ModelId=3E96C2EE00DD
void mitk::StateTransitionOperation::Execute()
{}
