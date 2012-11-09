/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#include "mitkGlobalInteraction.h"
#include "mitkInteractionConst.h"
#include "mitkStateEvent.h"
#include "mitkPositionEvent.h"

#include <itksys/SystemTools.hxx>

#include <vtkWorldPointPicker.h>



mitk::GlobalInteraction::GlobalInteraction()
: StateMachine(NULL)
, m_StateMachineFactory(NULL)
, m_EventMapper(NULL)
, m_CurrentlyInInformListenersLoop(false)
, m_CurrentlyInInformInteractorsLoop(false)
, m_IsInitialized(false)
, m_EventNotificationPolicy(INFORM_MULTIPLE)
{
}

mitk::GlobalInteraction::~GlobalInteraction()
{
  //s_GlobalInteraction doesn't have to be set = NULL;



  // StateMachineFactory and EventMapper have to be deleted explicitly, as they inherit from Vtk
  if (this->IsInitialized())
  {
    m_StateMachineFactory->Delete();
    m_StateMachineFactory = NULL;
    m_EventMapper->Delete();
    m_EventMapper = NULL;
  }
  m_ListenerList.clear();
  m_InteractorList.clear();
  m_SelectedList.clear();
  m_InteractorRelevanceMap.clear();
  m_ListenerRelevanceMap.clear();
  m_FocusManager = NULL;
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
  if(listener == NULL) return;
  if(dynamic_cast<Interactor*>(listener)!=NULL)
  {
    MITK_WARN << "Trying to add an Interactor ("
      << listener->GetNameOfClass() << ") as a listener. "
      << "This will probably cause problems";
  }

  if ( std::find(m_ListenerList.begin(), m_ListenerList.end(),listener) == m_ListenerList.end() )
  {
    m_ListenerList.push_back(listener);
  }
}


bool mitk::GlobalInteraction::RemoveListener(mitk::StateMachine* listener)
{
  // Defers removal to a time after the current event handling is finished. Otherwise the implementation of InformListeners would crash sometimes.
  m_ListenersFlaggedForRemoval.push_back(listener);

  StateMachineListIter position = std::find(m_ListenerList.begin(), m_ListenerList.end(),listener);
  bool removePossible = (position != m_ListenerList.end());

  RemoveFlaggedListeners();

  //check if in RelevanceMap
  for (StateMachineMapIter it = m_ListenerRelevanceMap.begin(); it != m_ListenerRelevanceMap.end(); it++)
  {
    if ((*it).second == listener)
    {
      m_ListenerRelevanceMap.erase(it);
      break;
    }
  }

  return removePossible;
}

void mitk::GlobalInteraction::RemoveFlaggedListeners()
{
  if (m_CurrentlyInInformListenersLoop) return;

  // iterate flagged listeners, remove them if possible
  if (m_ListenersFlaggedForRemoval.empty()) return;

  for (StateMachineCPointerListIter it = m_ListenersFlaggedForRemoval.begin(); it != m_ListenersFlaggedForRemoval.end(); ++it)
  {
    StateMachineListIter foundPosition = std::find( m_ListenerList.begin(), m_ListenerList.end(), *it );
    if (foundPosition != m_ListenerList.end())
    {
      m_ListenerList.erase( foundPosition );
    }
  }

  m_ListenersFlaggedForRemoval.clear();
}

void mitk::GlobalInteraction::AddInteractor(mitk::Interactor* interactor)
{
  if(interactor == NULL) return;
  if ( std::find(m_InteractorList.begin(), m_InteractorList.end(),interactor) == m_InteractorList.end() )
  {
    m_InteractorList.push_back(interactor);

    //if Interactor already selected, then add to selected list
    if (interactor->GetMode()==Interactor::SMSELECTED)
      this->AddToSelectedInteractors(interactor);
  }
}

bool mitk::GlobalInteraction::InteractorRegistered (mitk::Interactor* interactor)
{
  if ( std::find(m_InteractorList.begin(), m_InteractorList.end(), interactor) == m_InteractorList.end() )
    return false;
  else
    return true;
}

bool mitk::GlobalInteraction::ListenerRegistered (mitk::StateMachine* listener)
{
  if ( std::find(m_ListenerList.begin(), m_ListenerList.end(), listener) == m_ListenerList.end() )
    return false;
  else
    return true;
}

bool mitk::GlobalInteraction::RemoveInteractor(mitk::Interactor* interactor)
{
  InteractorListIter position = std::find(m_InteractorList.begin(), m_InteractorList.end(),interactor);
  if (position == m_InteractorList.end())
    return false;
  position = m_InteractorList.erase(position);

  //check if the interactor is also held in SelectedList
  this->RemoveFromSelectedInteractors(interactor);

  //check if in RelevanceMap
  for (InteractorMapIter it = m_InteractorRelevanceMap.begin(); it != m_InteractorRelevanceMap.end(); it++)
  {
    if ((*it).second == interactor)
    {
      if (m_CurrentInteractorIter == it)
        m_CurrentInteractorIter = m_InteractorRelevanceMap.end();
      m_InteractorRelevanceMap.erase(it);
      break;
    }
  }

  return true;
}

void mitk::GlobalInteraction::InformListeners(mitk::StateEvent const* stateEvent)
{
  m_CurrentlyInInformListenersLoop = true;

  for (StateMachineListIter it = m_ListenerList.begin(); it != m_ListenerList.end(); it++)
  {
    if((*it).IsNotNull())
      (*it)->HandleEvent(stateEvent);
  }

  m_CurrentlyInInformListenersLoop = false;
  RemoveFlaggedListeners();
}

bool mitk::GlobalInteraction::AskSelected(mitk::StateEvent const* stateEvent)
{
  if (m_SelectedList.empty())
    return false;

  bool ok = false, oneOk = false;

  //copy of m_SelectedList to be stable if an iterator gets removed during the following steps
  InteractorList copyOfSelectedList = m_SelectedList;
  InteractorListIter it = copyOfSelectedList.begin();

  for (; it != copyOfSelectedList.end(); it++)
  {
    oneOk = (*it)->HandleEvent(stateEvent);

    //if one HandleEvent did succeed, then set returnvalue on true;
    if (oneOk)
      ok = true;
  }
  return ok;
}


void mitk::GlobalInteraction::FillInteractorRelevanceMap(mitk::StateEvent const* stateEvent, float threshold)
{
  m_InteractorRelevanceMap.clear();

  for (InteractorListIter it = m_InteractorList.begin(); it != m_InteractorList.end(); it++)
  {
    if ((*it).IsNotNull())
    {
      //first ask for CanHandleEvent(..) and write it into the map if > 0
      float value = (*it)->CanHandleEvent(stateEvent);
      if (value > threshold)
      {
        m_InteractorRelevanceMap.insert(InteractorMap::value_type(value, (*it)));
      }
    }
  }
  //set the iterator to the first element to start stepping through interactors
  if (! m_InteractorRelevanceMap.empty())
    m_CurrentInteractorIter = m_InteractorRelevanceMap.begin();
  else
    m_CurrentInteractorIter = m_InteractorRelevanceMap.end();
}

void mitk::GlobalInteraction::FillListenerRelevanceMap(const mitk::StateEvent *stateEvent, float threshold)
{
  m_ListenerRelevanceMap.clear();

  for (StateMachineListIter it = m_ListenerList.begin(); it != m_ListenerList.end(); it++)
  {
    if((*it).IsNotNull())
    {
      float value = (*it)->CanHandleEvent(stateEvent);
      if (value > threshold)
      {
        m_ListenerRelevanceMap.insert(StateMachineMap::value_type(value, (*it)));
      }
    }
  }
}

/*
* Go through the list of interactors, that could possibly handle an event and ask if it has handled the event.
* If an interactor has handled an event, it should add itself to the list of selectedInteractors
* Ask as long as no interactor answers, that it could be handled
*/
bool mitk::GlobalInteraction::AskCurrentInteractor(mitk::StateEvent const* stateEvent)
{
  //no need to check if we don't have any interactors. nearly equal to m_CurrentInteractorIter == m_InteractorRelevanceMap.end
  if (m_InteractorRelevanceMap.empty())
    return false;

  bool handled = false;
  while ( m_CurrentInteractorIter != m_InteractorRelevanceMap.end()&& !handled)
  {
    handled = (*m_CurrentInteractorIter).second->HandleEvent(stateEvent);

    if (!handled)
      m_CurrentInteractorIter++;
  }

  //loop for later usage
  if (m_CurrentInteractorIter == m_InteractorRelevanceMap.end())
    m_CurrentInteractorIter = m_InteractorRelevanceMap.begin();
  return handled;
}

bool mitk::GlobalInteraction::AddFocusElement(mitk::FocusManager::FocusElement* element)
{
  return m_FocusManager->AddElement(element);
}

bool mitk::GlobalInteraction::RemoveFocusElement(mitk::FocusManager::FocusElement* element)
{
  return m_FocusManager->RemoveElement(element);
}

mitk::FocusManager::FocusElement* mitk::GlobalInteraction::GetFocus()
{
  return m_FocusManager->GetFocused();
}

bool mitk::GlobalInteraction::SetFocus(mitk::FocusManager::FocusElement* element)
{
  return m_FocusManager->SetFocused(element);
}

mitk::FocusManager* mitk::GlobalInteraction::GetFocusManager()
{
  return m_FocusManager.GetPointer();
}

mitk::EventMapper* mitk::GlobalInteraction::GetEventMapper()
{
  if (!this->IsInitialized())
  {
    MITK_FATAL <<"Global Interaction needs initialization!\n";
    return NULL;
  }
  return m_EventMapper;
}


bool mitk::GlobalInteraction::ExecuteAction(Action* action, mitk::StateEvent const* stateEvent)
{
  bool ok = false;

  ok = false;
  switch (action->GetActionId())
  {
  case AcDONOTHING:
    ok = true;
    break;
  case AcINFORMLISTENERS:
    if (m_EventNotificationPolicy == INFORM_MULTIPLE)
    {
      InformListeners(stateEvent);
      ok = true;
      break;
    }
    else
    {
      //0.5 since this is the default value which is returned by the superclass implementation of statemachine
      this->FillListenerRelevanceMap(stateEvent, 0.5);
      if (!m_ListenerRelevanceMap.empty())
      {
        StateMachineMapIter iter = m_ListenerRelevanceMap.begin();
        ok = (*iter).second->HandleEvent(stateEvent);
      }
      break;
    }
  case AcASKINTERACTORS:
    if (! AskSelected(stateEvent))//no interactor selected anymore
    {
      //fill the RelevanceMap to ask them bit by bit.
      //currentInteractor is set here to the beginning
      FillInteractorRelevanceMap(stateEvent, 0);

      //ask the Interactors to handle that event
      AskCurrentInteractor(stateEvent);
    }
    ok = true;
    break;
  default:
    ok = true;
  }
  return ok;
}


mitk::GlobalInteraction* mitk::GlobalInteraction::GetInstance()
{
  static mitk::GlobalInteraction::Pointer s_GlobalInteraction;

  if (s_GlobalInteraction.IsNull())
  {
    s_GlobalInteraction = mitk::GlobalInteraction::New();
  }
  return s_GlobalInteraction;
}


mitk::State* mitk::GlobalInteraction::GetStartState(const char* type)
{
  if ( this->IsInitialized() )
    return m_StateMachineFactory->GetStartState(type);

  MITK_FATAL << "Fatal Error in mitkGlobalInteraction.cpp: GlobalInteraction not initialized!\n";
  return NULL;
}

bool mitk::GlobalInteraction::AddToSelectedInteractors(mitk::Interactor* interactor)
{
  InteractorListIter position = std::find(m_SelectedList.begin(), m_SelectedList.end(),interactor);
  if (position != m_SelectedList.end())
  {
    //already added so don't add once more!
    return true;
  }
  else
    m_SelectedList.push_back(interactor);

  return true;
}

bool mitk::GlobalInteraction::RemoveFromSelectedInteractors(mitk::Interactor* interactor)
{
  if (interactor == NULL)
    return false;

  InteractorListIter position = std::find(m_SelectedList.begin(), m_SelectedList.end(),interactor);
  if (position != m_SelectedList.end())
  {
    position = m_SelectedList.erase(position);
    return true;
  }
  else
    return false;
}

mitk::StateMachineFactory* mitk::GlobalInteraction::GetStateMachineFactory()
{
  return m_StateMachineFactory;
}

bool mitk::GlobalInteraction::Initialize(const char* globalInteractionName, const std::string XMLBehaviorInput)
{
  if (this->IsInitialized())
  {
    MITK_WARN <<"Global Interaction has already been initialized.\n";
    return false;
  }

  m_FocusManager = FocusManager::New();

  // instantiates m_StateMachineFactory and load interaction patterns from XML string
  //if factory has been initialized before, delete it and initialize once more to not add patterns
  if (m_StateMachineFactory)
    m_StateMachineFactory->Delete();
  m_StateMachineFactory = StateMachineFactory::New();

  //if EventMapper was initialized before, delete it and initialize once more
  // to create new event descriptions and not to add them
  if (m_EventMapper)
    m_EventMapper->Delete();
  m_EventMapper = EventMapper::New();


  bool success = true;

  if (XMLBehaviorInput == "")
  {
    //load default behavior
    success &= m_StateMachineFactory->LoadStandardBehavior();
    success &= m_EventMapper->LoadStandardBehavior();
  }
  else if (itksys::SystemTools::FileExists(XMLBehaviorInput.c_str()) )
  {
    // load standard behavior from file
    success &= m_StateMachineFactory->LoadBehavior(XMLBehaviorInput);
    success &= m_EventMapper->LoadBehavior(XMLBehaviorInput);
  }
  else
  {
    //load standard behavior from XML string
    success &= m_StateMachineFactory->LoadBehaviorString(XMLBehaviorInput);
    success &= m_EventMapper->LoadBehaviorString(XMLBehaviorInput);
  }

  if(!success)
  {
    MITK_FATAL << "Error initializing global interaction!\n";
    return false;
  }

  //now instantiate what could not be done in InitializeStateStates because StateMachineFactory was not up yet:

  // (Re-) Initialize Superclass (StateMachine), because type was not given at time of construction
  m_Type = globalInteractionName;

  //get the start state of the pattern
  State::Pointer startState = m_StateMachineFactory->GetStartState(globalInteractionName);

  if (startState.IsNull())
  {
    MITK_FATAL << "Fatal Error in mitkGlobalInteraction.cpp: No StartState recieved from StateMachineFactory!\n";
    return false;
  }

  //clear the vector
  m_CurrentStateVector.clear();
  //add the start state pointer for the first time step to the list
  m_CurrentStateVector.push_back(startState);
  m_IsInitialized = true;
  return true;
}

void mitk::GlobalInteraction::SetEventNotificationPolicy(EVENT_NOTIFICATION_POLICY policy)
{
  this->m_EventNotificationPolicy = policy;
}

mitk::GlobalInteraction::EVENT_NOTIFICATION_POLICY mitk::GlobalInteraction::GetEventNotificationPolicy() const
{
  return this->m_EventNotificationPolicy;
}

