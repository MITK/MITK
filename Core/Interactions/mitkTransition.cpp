#include "Transition.h"

//##ModelId=3E5A3A77006F
mitk::Transition::Transition(std::string tName, int tId, int tnextStateId, int tEventId, int tSideEffectId)
: name(tName), id(tId), nextState(NULL), eventId(tEventId), sideEffectId(tSideEffectId)
{
}

//##ModelId=3E5A3BAE03B5
std::string mitk::Transition::GetName() const
{
	return name;
}

//##ModelId=3E5A3BCE00E0
State* mitk::Transition::GetNextState() const
{
	return nextState;
}

//##ModelId=3E5A3BF100F5
int mitk::Transition::GetSideEffectId() const
{
	return sideEffectId;
}

//##ModelId=3E5A3C1D0289
bool mitk::Transition::IsEvent(int tEventId) const
{
	if (tEventId == eventId)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//##ModelId=3E6370080067
mitk::Transition::setNextState(State* tState) const
{
	nextState = tState;
}

