#include "State.h"

//##ModelId=3E5B2A9203BD
mitk::State::State(std::string stateName, int stateId)
: name(stateName), id(stateId)
{
}

//##ModelId=3E5B2B2E0304
bool mitk::State::AddTransition(std::string transitionName, int transitionId, int nextStateId, int eventId, int sideEffectId) const
{
	Transition temptransition(transitionName, transitionId, nextStateId, eventId, sideEffectId);
	pair<iterator, bool> temppair = transitions[eventId].insert(temptransition);
	return (bool) temppair.second;
}

//##ModelId=3E5B2B9000AC
Transition* mitk::State::GetTransition(int eventId) const
{
	Transition *tempTransition = transitions.find(eventId);
	if( tempTransition != transitions.end() ) 
        return tempTransition;
	else
		return NULL;
}

//##ModelId=3E5B2C0503D5
string mitk::State::GetName() const
{
	return name;
}

//##ModelId=3E5B2C14016A
int mitk::State::GetId() const
{
	return id;
}

//##ModelId=3E64B4360017
bool mitk::State::locateTransition(int tEventId) const
{
	if( transitions.find(eventId) != transitions.end() ) 
        return true;
	else
		return false;
}