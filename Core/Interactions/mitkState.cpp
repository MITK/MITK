#include "State.h"

//##ModelId=3E5B2A9203BD
mitk::State::State(std::string stateName, int stateId)
: mName(stateName), m_Id(stateId)
{
}

//##ModelId=3E5B2B2E0304
bool mitk::State::AddTransition(std::string transitionName, int transitionId, int nextStateId, int eventId, int sideEffectId) const
{
	Transition temptransition(transitionName, transitionId, nextStateId, eventId, sideEffectId);
	pair<iterator, bool> temppair = m_Transitions[eventId].insert(temptransition);//adding the transition, recieving protocol
	return (bool) temppair.second;
}

//##ModelId=3E5B2B9000AC
Transition* mitk::State::GetTransition(int eventId) const
{
	Transition *tempTransition = m_Transitions.find(eventId);
	if( tempTransition != m_Transitions.end() )
        return tempTransition;
	else
		return NULL;
}

//##ModelId=3E5B2C0503D5
std::string mitk::State::GetName() const
{
	return m_Name;
}

//##ModelId=3E5B2C14016A
int mitk::State::GetId() const
{
	return m_Id;
}

//##ModelId=3E64B4360017
bool mitk::State::locateTransition(int EventId) const
{
	if( m_Transitions.find(eventId) != m_Transitions.end() )
        return true;
	else
		return false;
}