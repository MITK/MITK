#include "StateMachine.h"

//##ModelId=3E5B2DB301FD
mitk::StateMachine::StateMachine(std::string type)
: m_Type(type)  
{}

//##ModelId=3E5B2DE30378
bool mitk::StateMachine::SetEvent(StateEvent const* stateEvent)
{
	return true;
}

//##ModelId=3E5B2E660087
std::string mitk::StateMachine::GetName() const
{
	return m_Type;
}

//##ModelId=3E5B2E170228
bool mitk::StateMachine::Operation(int sideEffectId)
{
	return true;
}

