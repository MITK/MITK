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
  InteractorListIter position = std::find(m_InteractorList.begin(), m_InteractorList.end(),interactor);
  if (position == m_InteractorList.end())
    return false;
  position = m_InteractorList.erase(position);

  //ether in JurisdictionMap or in SelectedList
  position = std::find(m_SelectedList.begin(), m_SelectedList.end(),interactor);
  if (position != m_SelectedList.end())
    position = m_SelectedList.erase(position);
  else
  {
    for (InteractorMapIter it = m_JurisdictionMap.begin(); it != m_JurisdictionMap.end(); it++)
    {
      if ((*it).second == interactor)
      {
        m_JurisdictionMap.erase(it);
        if (m_CurrentInteractor == it)
          m_CurrentInteractor ==NULL;
        break;
      }
    }
  }

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
  bool ok, oneOk;
  ok = false;
  
  for (InteractorListIter it = m_SelectedList.begin(); it != m_SelectedList.end(); it++)
  {
    if((*it)!=NULL)
    {
      //Interactor are in Mode SELECTED or SUBSELECTED
	    oneOk = (*it)->HandleEvent(stateEvent, objectEventId, groupEventId);
      
      //if one HandleEvent did succeed, then set returnvalue on true;
      if (oneOk)
        ok = true;

      //if mode changed, then erase from selectedList
      if ((*it)->GetMode()==Interactor::SMDESELECTED)
        m_SelectedList.erase(it);
    }
  }
  return ok;
}

void mitk::GlobalInteraction::FillJurisdictionMap(mitk::StateEvent const* stateEvent, int , int, float swell)
{
  for (InteractorListIter it = m_InteractorList.begin(); it != m_InteractorList.end(); it++)
  {
    if((*it)!=NULL)
    {
      //first ask for CalculateJurisdiction(..) and write it into the map if > 0
      float value = (*it)->CalculateJurisdiction(stateEvent);
      if (value > swell)
      {
        m_JurisdictionMap.insert(InteractorMap::value_type(value, (*it)));
      }
    }
  }
  //set the iterator to the first element to start stepping through interactors
  if (! m_JurisdictionMap.empty())
    m_CurrentInteractor = m_JurisdictionMap.begin();
  else
    m_CurrentInteractor = NULL;
}

void mitk::GlobalInteraction::AskCurrentInteractor(mitk::StateEvent const* stateEvent, int objectEventId, int groupEventId)
{
  if ( m_CurrentInteractor != NULL)
  {
    (*m_CurrentInteractor).second->HandleEvent(stateEvent, objectEventId, groupEventId);

    //if after handling an event Interactor is in mode SELECTED or SUBSELECTED
    if ( ((*m_CurrentInteractor).second->GetMode() == mitk::Interactor::SMSELECTED ) || 
          ((*m_CurrentInteractor).second->GetMode() == mitk::Interactor::SMSUBSELECTED) )
    {
      m_SelectedList.clear();
      m_SelectedList.push_back((*m_CurrentInteractor).second);
      //clear the map to ask the selected interactor next time
      m_JurisdictionMap.clear();
      m_CurrentInteractor = NULL;
    }
    else //if the event could not be handled, then go to next interactor
    {
      m_CurrentInteractor++;
      //if at end, then loop to the begining
      if (m_CurrentInteractor == m_JurisdictionMap.end())
        m_CurrentInteractor= m_JurisdictionMap.begin();
    }
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
  case AcINFORMLISTENERS:
    InformListeners(stateEvent, objectEventId, groupEventId);
    ok = true;
    break;
  case AcASKINTERACTORS:
    if (! AskSelected(stateEvent, objectEventId, groupEventId))//no selected
    {
      //if m_JurisdictionMap is empty, then fill it.
      if (m_JurisdictionMap.empty())
        FillJurisdictionMap(stateEvent, objectEventId, groupEventId, 0);
      
      
      if (m_JurisdictionMap.empty())//no jurisdiction value above 0 could be found, so take all
        FillJurisdictionMap(stateEvent, objectEventId, groupEventId, -1);

      //ask the next Interactor to handle that event
      AskCurrentInteractor(stateEvent, objectEventId, groupEventId);
    }
    ok = true;
    break;
  default:
	  ok = true;
  }
  return ok;
}