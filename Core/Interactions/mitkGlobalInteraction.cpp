#include "mitkGlobalInteraction.h"
#include "mitkInteractionConst.h"
#include "mitkEvent.h"
#include "mitkStatusBar.h"


//##ModelId=3EAD420E0088
mitk::GlobalInteraction::GlobalInteraction(std::string type)
: StateMachine(type)
{
//build up the FocusManager
  m_FocusManager = new mitk::FocusManager();
}

inline mitk::StateEvent* GenerateEmptyStateEvent(int eventId)
{
  mitk::Event *noEvent = new mitk::Event(NULL,
        mitk::Type_User,
        mitk::BS_NoButton,
		    mitk::BS_NoButton,
        mitk::Key_none);
  mitk::StateEvent *stateEvent = new mitk::StateEvent(eventId, noEvent);
	return stateEvent;
}

//##ModelId=3E7F497F01AE
bool mitk::GlobalInteraction::ExecuteSideEffect(int sideEffectId, mitk::StateEvent const* stateEvent, int objectEventId, int groupEventId)
{
  bool ok = false;

  switch (sideEffectId)
  {
  case SeDONOTHING:
    ok = true;
	break;
  default:
	  ok = true;
  }

/*
@todo: List of all StateMachines/Interactables that are beneath 
the focused BaseRenderer.
register at DataTree, that if anything changes, the list can be refreshed.
If a new object is generated here, the new object flows into the m_SelectedElements
and the Focus changes.

multiple hierarchie in StateMachines:
this StateMachine sends all events to all SM under the focused Node.
Each SM can have further SM included (in member_var e.g.) and has to descide, 
whether to send the event further down or to stop it.
From outside each SM seems be be one class, but inside it is divided up in many 
SM to reduce the complexibility.
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

bool mitk::GlobalInteraction::AddFocusElement(mitk::FocusManager::FocusElement* element)
{
  return m_FocusManager->AddElement(element);
}

bool mitk::GlobalInteraction::RemoveFocusElement(mitk::FocusManager::FocusElement* element)
{
  return m_FocusManager->RemoveElement(element);
}

const mitk::FocusManager::FocusElement* mitk::GlobalInteraction::GetFocus() 
{
  return m_FocusManager->GetFocused();
}

bool mitk::GlobalInteraction::SetFocus(mitk::FocusManager::FocusElement* element)
{
  return m_FocusManager->SetFocused(element);
}

mitk::FocusManager* mitk::GlobalInteraction::GetFocusManager()
{
  return m_FocusManager;
}

