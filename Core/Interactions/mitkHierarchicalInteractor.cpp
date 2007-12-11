/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkHierarchicalInteractor.h"
#include "mitkAction.h"
#include "mitkStateEvent.h"
#include "mitkState.h"
#include "mitkInteractionConst.h"
#include <mitkProperties.h>
#include <mitkEventMapper.h>

mitk::HierarchicalInteractor::HierarchicalInteractor(const char * type, DataTreeNode* dataTreeNode)
:Interactor(type,dataTreeNode)
{
  m_SelectedInteractors.clear();
  m_AllInteractors.clear();
}

bool mitk::HierarchicalInteractor::IsSubSelected() const 
{
  return (m_Mode==SMSUBSELECTED);
}

//##Documentation
//## first we look if the event can be handled by this interactor.
//## if it could not be handled, then check if we are in subselected mode.
//## Then send the event to lower statemachines and afterwards check if they are still sub/selected
bool mitk::HierarchicalInteractor::HandleEvent(StateEvent const* stateEvent)
{
  ////DEBUG only: remember states of sub-interactors
  //typedef std::map<Interactor::Pointer, State*> SisType;
  //SisType selectedInteractorStates;
  //typedef std::pair <Interactor::Pointer, State*> SisPair;
  //InteractorListIter i = m_SelectedInteractors.begin();
  //const InteractorListIter end = m_SelectedInteractors.end();
  //while ( i != end )
  //{
  //  selectedInteractorStates.insert(SisPair(*i, (*i)->GetCurrentState());
  //  ++i;
  //}

  //first do a standard HandleEvent
  bool ok = Interactor::HandleEvent( stateEvent );

  if (!ok && IsSubSelected() )
  {
    ok = TransmitEvent( EventMapper::RefreshStateEvent(const_cast<StateEvent*>(stateEvent)) );

    //check if we still have a selected subinteractor. if not, then send deselectEvent
    bool subSelected = false;
    InteractorListIter i = m_SelectedInteractors.begin();
    const InteractorListIter end = m_SelectedInteractors.end();

    while ( i != end )
    {
      subSelected = ( (*i)->IsSelected() || subSelected );
      ++i;
    }

    if ( !subSelected ) 
    {
      StateEvent sd( EIDSUBDESELECT, stateEvent->GetEvent() );
      ok = Interactor::HandleEvent( &sd );
      if (!ok)
        itkWarningMacro("No SSD Transition defined! Redesign Interaction!!! State: " << GetCurrentState()->GetName() << " EventId: " << stateEvent->GetId());
      if(IsSubSelected())
        itkWarningMacro("Mode SubSelected, but no selected sub-interactor!!! Redesign Interaction!!! State: " << GetCurrentState()->GetName() << " EventId: " << stateEvent->GetId());
    }
  }
  else
  {
    //DEBUG only: check selection mode!
    bool subSelected = false;
    InteractorListIter i = m_SelectedInteractors.begin();
    const InteractorListIter end = m_SelectedInteractors.end();

    while ( i != end )
    {
      subSelected = ( (*i)->IsSelected() || subSelected );
      ++i;
    }
    if(IsSubSelected()!=subSelected)
      itkWarningMacro("IsSubSelected()="<<IsSubSelected()<<"!=subSelected!!! Redesign Interaction!!! State: " << GetCurrentState()->GetName() << " EventId: " << stateEvent->GetId());
  }
  return ok;
}

bool mitk::HierarchicalInteractor::TransmitEvent( StateEvent const* stateEvent ) 
{
  bool ok = false;
  InteractorListIter i = m_SelectedInteractors.begin();
  const InteractorListIter end = m_SelectedInteractors.end();

  while ( i != end )
  {
    //safety!
    /*if ((*i) == NULL)
    break;*/

    ok = (*i)->HandleEvent( stateEvent ) || ok;

    if (!(*i)->IsSelected())
    {
      i = m_SelectedInteractors.erase(i);
      if (m_SelectedInteractors.empty())
        break;
    }
    else
      i++;
  }
  return ok;
}

float mitk::HierarchicalInteractor::CalculateJurisdiction(StateEvent const* stateEvent) const
//go through all lower statemachines and this statemachine and return the highest value
{
  float thisJurisdiction = Superclass::CalculateJurisdiction( stateEvent );
  float lowerJurisdiction = this->CalculateLowerJurisdiction(stateEvent);
  if (lowerJurisdiction > thisJurisdiction)
    thisJurisdiction = lowerJurisdiction;

  return thisJurisdiction;
}

float mitk::HierarchicalInteractor::CalculateLowerJurisdiction(StateEvent const* stateEvent) const
{
  float lowerJurisdiction = 0.0f;
  InteractorListConstIter i = m_AllInteractors.begin();
  InteractorListConstIter end = m_AllInteractors.end();

  while ( i != end )
  {
    float currentJurisdiction = (*i)->CalculateJurisdiction( stateEvent );
    if (lowerJurisdiction < currentJurisdiction)
      lowerJurisdiction = currentJurisdiction;
    i++;
  }

  return lowerJurisdiction;
}


bool mitk::HierarchicalInteractor::ExecuteAction( Action* action, mitk::StateEvent const* stateEvent ) 
{
  switch (action->GetActionId())
  {
  case AcMODEDESELECT:
    {
      //declare operations for modechange:
      this->CreateModeOperation( SMDESELECTED );
      m_SelectedInteractors.clear();
      return true;
    }
  case AcMODESELECT:
    { 
      //declare operations for modechange:
      this->CreateModeOperation( SMSELECTED );
      m_SelectedInteractors.clear();
      return true;
    }
  case AcMODESUBSELECT:
    {
      //declare operations for modechange:
      this->CreateModeOperation( SMSUBSELECTED );
      return true;
    }
  case AcCHECKONESUBINTERACTOR:
    {
      m_SelectedInteractors.clear();
      InteractorListIter i = m_AllInteractors.begin();
      InteractorListIter bestInteractor = m_AllInteractors.end();
      InteractorListConstIter end = m_AllInteractors.end();
      float jurisdiction = 0.0f;

      while ( i != end )
      {
        float currentJurisdiction = (*i)->CalculateJurisdiction( stateEvent );

        if ( jurisdiction < currentJurisdiction )          
        {
          jurisdiction = currentJurisdiction;
          bestInteractor = i;
        }

        i++;
      }

      FloatProperty* thresholdProperty = dynamic_cast<FloatProperty*>(action->GetProperty("threshold"));
      float threshold;

      if ( thresholdProperty )
        threshold = thresholdProperty->GetValue();
      else 
        threshold = 0.5;         

      if ( jurisdiction >= threshold && bestInteractor != m_AllInteractors.end())  
      {          
        m_SelectedInteractors.push_back( *bestInteractor );
        mitk::StateEvent* newStateEvent = new mitk::StateEvent(EIDYES, stateEvent->GetEvent());
        this->HandleEvent( newStateEvent );
      }
      else 
      {
        mitk::StateEvent* newStateEvent = new mitk::StateEvent(EIDNO, stateEvent->GetEvent());
        this->HandleEvent( newStateEvent );        
      }      

      return true;
    }
  case AcCHECKSUBINTERACTORS:
    {
      FloatProperty* thresholdProperty = dynamic_cast<FloatProperty*>(action->GetProperty("threshold"));
      float threshold;

      if ( thresholdProperty )
        threshold = thresholdProperty->GetValue();
      else 
        threshold = 0.5;         

      m_SelectedInteractors.clear();
      InteractorListIter i = m_AllInteractors.begin();
      InteractorListIter bestInteractor = m_AllInteractors.end();
      const InteractorListIter end = m_AllInteractors.end();

      while ( i != end )
      {
        float jurisdiction = CalculateJurisdiction( stateEvent );

        if ( jurisdiction > threshold )          
          m_SelectedInteractors.push_back( *i );

        i++;
      }

      if ( !m_SelectedInteractors.empty() )
      {          
        mitk::StateEvent* newStateEvent = new mitk::StateEvent(EIDYES, stateEvent->GetEvent());
        this->HandleEvent( newStateEvent );
      }
      else 
      {
        mitk::StateEvent* newStateEvent = new mitk::StateEvent(EIDNO, stateEvent->GetEvent());
        this->HandleEvent( newStateEvent );
      }      

      return true;
    }
  case AcFORCESUBINTERACTORS:
    {
      m_SelectedInteractors = m_AllInteractors;
      return true;
    }
  case AcTRANSMITEVENT:
    {
      if ( IsSubSelected() ) 
      {
        TransmitEvent( mitk::EventMapper::RefreshStateEvent(const_cast<StateEvent*>(stateEvent)) );
      }
      return true;
    }
  default:
    return Superclass::ExecuteAction( action, stateEvent );
  }   
}

bool mitk::HierarchicalInteractor::AddInteractor(mitk::Interactor::Pointer interactor)
{
  if (interactor.IsNotNull())
  {
    m_AllInteractors.push_back(interactor);
    interactor->SetDataTreeNode(m_DataTreeNode);
    return true;
  }
  else
    return false;
}

void mitk::HierarchicalInteractor::SetDataTreeNode( mitk::DataTreeNode* dataTreeNode )
{
  Superclass::SetDataTreeNode(dataTreeNode);

  InteractorListConstIter i = m_AllInteractors.begin();
  InteractorListConstIter end = m_AllInteractors.end();

  while ( i != end )
  {
    (*i)->SetDataTreeNode( dataTreeNode );
    i++;
  }
}
