#include "GlobalInteraction.h"
#include "mitkInteractionConst.h"
#include "Event.h"
#include "mitkStatusBar.h"
//#include "Focus.h"



//##ModelId=3EAD420E0088
mitk::GlobalInteraction::GlobalInteraction(std::string type)
: StateMachine(type)
{
	//Quickimplementation ... Ivo: No Need of focus, cause glWidget sends the Event now.
	//m_Focus = new mitk::Focus("focus");
}

inline mitk::StateEvent* GenerateEmptyStateEvent(int eventId)
{
    mitk::Event *noEvent = new mitk::Event(NULL,
        mitk::Type_User,
        mitk::BS_NoButton,
		mitk::BS_NoButton,
        mitk::Key_none);
    mitk::StateEvent *stateEvent = new mitk::StateEvent(eventId, noEvent);
    //mitk::StateEvent *stateEvent = new mitk::StateEvent();
	//stateEvent->Set( eventId, noEvent );
	return stateEvent;
}

//##ModelId=3E7F497F01AE
bool mitk::GlobalInteraction::ExecuteSideEffect(int sideEffectId, mitk::StateEvent const* stateEvent, int objectEventId, int groupEventId)
{
  bool ok = false;
  //if the Event is a PositionEvent, then get the worldCoordinates through Focus
  /*Quickimplementation... no need of focus, since the qglwidget send the event
  if ( (stateEvent->GetEvent() )->GetType()== MouseButtonPress)
  {
    ok = m_Focus->HandleEvent(stateEvent,...);//give it to Focus which calculates accordingly to it's Statemachine the worldcoordinates
	//for debugging
	if (!ok)
	  (StatusBar::GetInstance())->DisplayText("Error! Sender: PositionEvent;   Message: HandleEvent returned false", 10000);
  }
  */
     
  ok = false;
  switch (sideEffectId)
  {
  case SeDONOTHING:
    ok = true;
	break;
  default:
  //unknown event!
	ok = true;
  }

/*
@todo: List of all StateMachines/Interactables that are beneath 
the focused BaseRenderer (Focus-StateMachine).
register at DataTree, that if anything changes, the list can be refreshed.
If a new object is generated here, the new object flows into the m_SelectedElements
and the Focus changes.
*/

  //Quickimplementation; is to be changed with List from DataTree
  //send the event to all in List
  for (StateMachineListIter it = m_LocalStateMachines.begin(); it != m_LocalStateMachines.end(); it++)
  {
    if((*it)!=NULL)
      ok = (*it)->HandleEvent(stateEvent, objectEventId, groupEventId);
  }
  return true;//return ok;
}



//##ModelId=3EF099E90065
void mitk::GlobalInteraction::AddStateMachine(mitk::StateMachine* stateMachine)
{
  if ( std::find(m_LocalStateMachines.begin(), m_LocalStateMachines.end(),stateMachine) == m_LocalStateMachines.end() )
  {
    m_LocalStateMachines.push_back(stateMachine);
  }
}

//##ModelId=3EF099E900D2
bool mitk::GlobalInteraction::RemoveStateMachine(mitk::StateMachine* stateMachine)
{
  // Try find  
  StateMachineListIter position = std::find(m_LocalStateMachines.begin(), m_LocalStateMachines.end(),stateMachine);
  if (position == m_LocalStateMachines.end())
    return false;
  position = m_LocalStateMachines.erase(position);
  return true;
}