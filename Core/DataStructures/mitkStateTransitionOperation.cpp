#include "StateTransitionOperation.h"

//##ModelId=3E78311303E7
mitk::StateTransitionOperation::StateTransitionOperation(OperationType operationType, State* state)
: mitk::Operation(operationType), m_State(state)
{}

//##ModelId=3E78307601D8
mitk::State* mitk::StateTransitionOperation::GetState()
{
	return m_State;
}