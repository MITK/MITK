#include "State.h"

//##ModelId=3E5B2A9203BD
mitk::State::State(std::string stateName, int stateId)
: m_Name(stateName), m_Id(stateId)
{
}

//##ModelId=3E5B2B2E0304
bool mitk::State::AddTransition(std::string transitionName, int nextStateId, int eventId, int sideEffectId) const
{
/*	Transition tempTransition(transitionName, nextStateId, eventId, sideEffectId);
	m_Transitions.insert(std::pair<int,Transition>(eventId, tempTransition));
##################################################################################################
	incorect!
	
	
	
	*/		
	return (bool) true;//ToDo: checking the status of insert

}

//##ModelId=3E5B2B9000AC
mitk::Transition* mitk::State::GetTransition(int eventId) const
{
/*
	Transition *tempTransition = m_Transitions.find(eventId);
	if( tempTransition != m_Transitions.end() )
        return tempTransition;
	else
		return NULL;
		################################################################
		incorect
		*/
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
bool mitk::State::LocateTransition(int eventId) const
{
	if( m_Transitions.find(eventId) != m_Transitions.end() )
        return true;
	else
		return false;
}

//##ModelId=3E68C573013F
//##Documentation
//## searches dedicated States of all Transitions and 
//## sets *nextState of these Transitions.
//## allStates is a List of all build States of that StateMachine
bool mitk::State::ConnectTransitions(StateMap *allStates) const
{
/*	std::iterator tempTransition = m_Transitions.begin();
	
	for (int i = 0; i < m_Transitions.size(); i++, tempTransition++)
	{
        State *tempState = allStates.find(tempTransition.GetNextStateId());
        if( tempState != allStates.end() )
            tempTransition.setNextState(tempState);
        else
            return false;//State not found!
	}
	################################################################################
	incorect!
	*/
	return true;
}
