#include "State.h"

//##ModelId=3E5B2A9203BD
mitk::State::State(string stateName, int stateId)
{
}

//##ModelId=3E5B2B2E0304
bool mitk::State::AddTransition(string transitionName, int transitionId, int nextStateId, int eventId, int sideEffectId) const
{
}

//##ModelId=3E5B2B9000AC
Transition* mitk::State::GetTransition(int eventId) const
{
}

//##ModelId=3E5B2C0503D5
string mitk::State::GetName() const
{
}

//##ModelId=3E5B2C14016A
int mitk::State::GetId() const
{
}

//##ModelId=3E64B4360017
bool mitk::State::locateTransition(int tEventId) const
{
}

