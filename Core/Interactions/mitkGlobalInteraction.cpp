/*=========================================================================

  Program:   Medical Imaging & Interaction Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/ for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkGlobalInteraction.h"
#include "mitkInteractionConst.h"
#include "mitkEventMapper.h"
#include "mitkEvent.h"
#include <mitkStatusBar.h>
#include "mitkAction.h"
#include <mitkPositionEvent.h>
#include <vtkWorldPointPicker.h>
#include <mitkOpenGLRenderer.h>



//##ModelId=3EAD420E0088
mitk::GlobalInteraction::GlobalInteraction(const char * type)
: StateMachine(type)
{
  //build up the FocusManager
  m_FocusManager = new mitk::FocusManager();
  m_InteractionDebugger = InteractionDebugger::New();
  InteractionDebugger::Deactivate();
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
        if (m_CurrentInteractorIter == it)
          m_CurrentInteractorIter == m_JurisdictionMap.end();
        break;
      }
    }
  }

  return true;
}

void mitk::GlobalInteraction::InformListeners(mitk::StateEvent const* stateEvent)
{
  for (StateMachineListIter it = m_ListenerList.begin(); it != m_ListenerList.end(); it++)
  {
    if((*it)!=NULL)
	    (*it)->HandleEvent(stateEvent);
  }

}

bool mitk::GlobalInteraction::AskSelected(mitk::StateEvent const* stateEvent)
{
  bool ok, oneOk;
  ok = false;
  InteractorListIter it = m_SelectedList.begin();
  
  while ( it != m_SelectedList.end())
  {
    if((*it)!=NULL && !m_SelectedList.empty())
    {
      //Interactor are in Mode SELECTED or SUBSELECTED
	    oneOk = (*it)->HandleEvent(stateEvent);
      
      //if one HandleEvent did succeed, then set returnvalue on true;
      if (oneOk)
        ok = true;

      //if mode changed, then erase from selectedList
      if ((*it)->GetMode()==Interactor::SMDESELECTED)
      {
        it = m_SelectedList.erase(it);
      }
      else
      {
        ++it;
      }
    }
    else
      ++it;
  }
  return ok;
}

void mitk::GlobalInteraction::FillJurisdictionMap(mitk::StateEvent const* stateEvent, float threshold)
{
  for (InteractorListIter it = m_InteractorList.begin(); it != m_InteractorList.end(); it++)
  {
    if((*it)!=NULL)
    {
      //first ask for CalculateJurisdiction(..) and write it into the map if > 0
      float value = (*it)->CalculateJurisdiction(stateEvent);
      if (value > threshold)
      {
        ///TODO: hier werden die gleichen IDs überschrieben! von map auf vector umändern!
        m_JurisdictionMap.insert(InteractorMap::value_type(value, (*it)));
      }
    }
  }
  //set the iterator to the first element to start stepping through interactors
  if (! m_JurisdictionMap.empty())
    m_CurrentInteractorIter = m_JurisdictionMap.begin();
  else
    m_CurrentInteractorIter = m_JurisdictionMap.end();
}

void mitk::GlobalInteraction::AskCurrentInteractor(mitk::StateEvent const* stateEvent)
{
  if (m_JurisdictionMap.empty())
    return;

  if ( m_CurrentInteractorIter != m_JurisdictionMap.end())//if set
  {
    (*m_CurrentInteractorIter).second->HandleEvent(stateEvent);

    //if after handling an event Interactor is in mode SELECTED or SUBSELECTED
    //then make sure, that this sub-/ selected interactor gets the next event
    if ( ((*m_CurrentInteractorIter).second->GetMode() == mitk::Interactor::SMSELECTED ) || 
          ((*m_CurrentInteractorIter).second->GetMode() == mitk::Interactor::SMSUBSELECTED) )
    {
      m_SelectedList.clear();
      m_SelectedList.push_back((*m_CurrentInteractorIter).second);
      
      //clear the map cause we have found an selected interactor so we can directly take that interactor the next time
      m_JurisdictionMap.clear();
      m_CurrentInteractorIter = m_JurisdictionMap.end();
    }
    else //if the event could not be handled, then go to next interactor
    {
      m_CurrentInteractorIter++;
      //if at end, then loop to the begining
      if (m_CurrentInteractorIter == m_JurisdictionMap.end())
        m_CurrentInteractorIter= m_JurisdictionMap.begin();
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

bool mitk::GlobalInteraction::GetWorldCoordinate(const mitk::DisplayPositionEvent *displayPositionEvent, mitk::PositionEvent* positionEvent)
{
  /* pick a Prop3D and assume its position as event 3D coordinates */
  //also possible to take the Sender of the event!
  const mitk::Point2D displayPoint = displayPositionEvent->GetDisplayPosition();
  const FocusManager::FocusElement* fe = this->GetFocus();
  FocusManager::FocusElement* fe2 =  const_cast <FocusManager::FocusElement*>(fe);
  mitk::OpenGLRenderer* glRenderer = dynamic_cast<mitk::OpenGLRenderer*>(fe2);
  if (glRenderer == NULL)
    return false;

  vtkWorldPointPicker *worldPicker = vtkWorldPointPicker::New();
  //picker->SetTolerance (0.0001);
  worldPicker->Pick(displayPoint[0], displayPoint[1], 0, glRenderer->GetVtkRenderer());
  mitk::Point3D worldPoint;
  vtk2itk(worldPicker->GetPickPosition(), worldPoint);
  positionEvent->SetWorldPosition(worldPoint);
  return true;
}

//##ModelId=3E7F497F01AE
bool mitk::GlobalInteraction::ExecuteAction(Action* action, mitk::StateEvent const* stateEvent)
{
  bool ok = false;
  
 
  //check if we already have a PositionEvent with 3D worldcoordinates.
  //if we have a DiplayPositionEvent, we check if the Event is thrown by a mouseclick.
  //If so, compute the worldcoodinates through this->GetWorldPosition()
  mitk::PositionEvent const  *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
  if (posEvent == NULL)
    if (stateEvent->GetEvent()->GetType() == Type_MouseButtonPress)
    {
      mitk::DisplayPositionEvent const  *displayEvent = dynamic_cast <const mitk::DisplayPositionEvent *> (stateEvent->GetEvent());
      if (displayEvent!=NULL)
      {
        mitk::Point3D worldPoint;
        worldPoint.Fill(0);

        PositionEvent* positionEvent = new PositionEvent(displayEvent->GetSender(), displayEvent->GetType(), displayEvent->GetButton(),
          displayEvent->GetButtonState(), displayEvent->GetKey(), displayEvent->GetDisplayPosition(), worldPoint);
        ok = GetWorldCoordinate(displayEvent, positionEvent);
        if (ok)
        {
          mitk::EventMapper::SetStateEvent(positionEvent);
        }
      }
    }


  ok = false;
  switch (action->GetActionId())
  {
  case AcDONOTHING:
    ok = true;
	break;
  case AcINFORMLISTENERS:
    InformListeners(stateEvent);
    ok = true;
    break;
  case AcASKINTERACTORS:
    if (! AskSelected(stateEvent))//no selected
    {
      //if m_JurisdictionMap is empty, then fill it.
      if (m_JurisdictionMap.empty())
        FillJurisdictionMap(stateEvent, 0);
      
      //no jurisdiction value above 0 could be found, so take all to convert to old scheme
      if (m_JurisdictionMap.empty())
        FillJurisdictionMap(stateEvent, -1);

      //ask the next Interactor to handle that event
      AskCurrentInteractor(stateEvent);
      
      //after asking for jurisdiction and sending the events to the interactors,
      //interactors should change the mode. We can now clear the jurisdictionmap.
      m_JurisdictionMap.clear();
    }
    else
    {
      //checking if the selected one is still in Mode Subselected or selected
      //...//todo
    }
    ok = true;
    break;
  default:
	  ok = true;
  }
  return ok;
}
