#include "Transition.h"

//##ModelId=3E5A3A77006F
mitk::Transition::Transition(std::string name, int nextStateId, int eventId, int sideEffectId)
: m_Name(name), m_NextStateId(nextStateId), m_EventId(eventId), m_SideEffectId(sideEffectId)
{
	 m_NextState = NULL;
}

//##ModelId=3E5A3BAE03B5
std::string mitk::Transition::GetName() const
{
	return m_Name;
}

//##ModelId=3E5A3BCE00E0
mitk::State* mitk::Transition::GetNextState() const
{
	return m_NextState;
}

int mitk::Transition::GetNextStateId() const
{
	return m_NextStateId;
}

//##ModelId=3E5A3BF100F5
int mitk::Transition::GetSideEffectId() const
{
	return m_SideEffectId;
}

//##ModelId=3E5A3C1D0289
bool mitk::Transition::IsEvent(int eventId) const
{
	if (eventId == m_EventId)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//##ModelId=3E6370080067
void mitk::Transition::setNextState(State* state)
{
	m_NextState = state;
}

