#include "mitkGlobalInteraction.h"
#include "mitkInteractionConst.h"
#include "mitkEvent.h"
#include "mitkStatusBar.h"


//##ModelId=3EAD420E0088
mitk::GlobalInteraction::GlobalInteraction(const char * type)
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


void mitk::GlobalInteraction::AddListener(mitk::StateMachine* listener)
{
  if ( std::find(m_ListenerList.begin(), m_ListenerList.end(),listener) == m_ListenerList.end() )
  {
    m_ListenerList.push_back(listener);
  }
}


bool mitk::GlobalInteraction::RemoveListener(mitk::StateMachine* listener)
{
  // Try find
  StateMachineListIter position = std::find(m_ListenerList.begin(), m_ListenerList.end(),listener);
  if (position == m_ListenerList.end())
    return false;
  position = m_ListenerList.erase(position);
  return true;
}

void mitk::GlobalInteraction::AddInteractor(mitk::Interactor* interactor)
{
  if ( std::find(m_InteractorList.begin(), m_InteractorList.end(),interactor) == m_InteractorList.end() )
  {
    m_InteractorList.push_back(interactor);
  }
}


bool mitk::GlobalInteraction::RemoveInteractor(mitk::Interactor* interactor)
{
  // Try find
  InteractorListIter position = std::find(m_InteractorList.begin(), m_InteractorList.end(),interactor);
  if (position == m_InteractorList.end())
    return false;
  position = m_InteractorList.erase(position);
  return true;
}


void mitk::GlobalInteraction::InformListeners(mitk::StateEvent const* stateEvent, int objectEventId, int groupEventId)
{
  for (StateMachineListIter it = m_ListenerList.begin(); it != m_ListenerList.end(); it++)
  {
     if((*it)!=NULL)
	   (*it)->HandleEvent(stateEvent, objectEventId, groupEventId);
  }

}

bool mitk::GlobalInteraction::AskSelected(mitk::StateEvent const* stateEvent, int objectEventId, int groupEventId)
{
  bool ok;
  for (InteractorListIter it = m_SelectedList.begin(); it != m_SelectedList.end(); it++)
  {
    if((*it)!=NULL)
	  ok = (*it)->HandleEvent(stateEvent, objectEventId, groupEventId);
  }

  return ok;
}

void mitk::GlobalInteraction::AskAllInteractors(mitk::StateEvent const* stateEvent, int objectEventId, int groupEventId)
{
  for (InteractorListIter it = m_InteractorList.begin(); it != m_InteractorList.end(); it++)
    {
      if((*it)!=NULL)
        (*it)->HandleEvent(stateEvent, objectEventId, groupEventId);
    }

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



//##ModelId=3E7F497F01AE
bool mitk::GlobalInteraction::ExecuteAction(int actionId, mitk::StateEvent const* stateEvent, int objectEventId, int groupEventId)
{
  bool ok = false;

  switch (actionId)
  {
  case AcDONOTHING:
    ok = true;
	break;
  default:
	  ok = true;
  }

  InformListeners(stateEvent, objectEventId, groupEventId);
  AskAllInteractors(stateEvent, objectEventId, groupEventId);

  return ok;
}