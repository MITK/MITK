#include "mitkHierarchicalInteractor.h"
#include "mitkAction.h"
#include "mitkInteractionConst.h"
#include <mitkProperties.h>

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
bool mitk::HierarchicalInteractor::HandleEvent(StateEvent const* stateEvent, int objectEventId, int groupEventId)
{
  bool ok = Interactor::HandleEvent( stateEvent, objectEventId, groupEventId );

  if (!ok)
  {
    if ( IsSubSelected() ) 
    {
      ok = TransmitEvent( stateEvent, objectEventId, groupEventId );

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
        ok = Interactor::HandleEvent( &sd, objectEventId, groupEventId );
        if (!ok)
          itkWarningMacro("No SSD Transition defined! Redesign Interaction!!!");
      }
    }
  }
  return ok;
}

bool mitk::HierarchicalInteractor::TransmitEvent( StateEvent const* stateEvent, int objectEventId, int groupEventId ) 
{
  bool ok = false;
  InteractorListIter i = m_SelectedInteractors.begin();
  const InteractorListIter end = m_SelectedInteractors.end();

  while ( i != end )
  {
    //safety!
    if ((*i) == NULL)
      break;

    ok = (*i)->HandleEvent( stateEvent, objectEventId, groupEventId ) || ok;

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

bool mitk::HierarchicalInteractor::ExecuteAction(Action* action, mitk::StateEvent const* stateEvent, int objectEventId, int groupEventId) 
{
  switch (action->GetActionId())
  {
    case AcMODEDESELECT:
      {
        //declare operations for modechange:
        this->CreateModeOperation(SMDESELECTED, objectEventId, groupEventId);
        m_SelectedInteractors.clear();
        return true;
      }
    case AcMODESELECT:
      { 
        //declare operations for modechange:
        this->CreateModeOperation(SMSELECTED, objectEventId, groupEventId);
        m_SelectedInteractors.clear();
        return true;
      }
    case AcMODESUBSELECT:
      {
        //declare operations for modechange:
        this->CreateModeOperation(SMSUBSELECTED, objectEventId, groupEventId);
        return true;
      }
    case AcCHECKONESUBINTERACTOR:
      {
        m_SelectedInteractors.clear();
        InteractorListIter i = m_AllInteractors.begin();
        InteractorListIter bestInteractor = m_AllInteractors.end();
        const InteractorListIter end = m_AllInteractors.end();
        float jurisdiction = 0.0f;

        while ( i != end )
        {
          float currentJurisdiction = (*i)->CalculateJurisdiction( stateEvent );
    
          if ( jurisdiction > currentJurisdiction )          
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

        if ( jurisdiction >= threshold )  
        {          
          m_SelectedInteractors.push_back( *i );
          mitk::StateEvent* newStateEvent = new mitk::StateEvent(EIDYES, stateEvent->GetEvent());
          this->HandleEvent( newStateEvent, objectEventId, groupEventId );
        }
        else 
        {
          mitk::StateEvent* newStateEvent = new mitk::StateEvent(EIDNO, stateEvent->GetEvent());
          this->HandleEvent( newStateEvent, objectEventId, groupEventId );        
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
        float jurisdiction = 0.0f;

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
          this->HandleEvent( newStateEvent, objectEventId, groupEventId );
        }
        else 
        {
          mitk::StateEvent* newStateEvent = new mitk::StateEvent(EIDNO, stateEvent->GetEvent());
          this->HandleEvent( newStateEvent, objectEventId, groupEventId );        
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
          TransmitEvent( stateEvent, objectEventId, groupEventId );
        }
        return true;
      }
    default:
      return Superclass::ExecuteAction( action, stateEvent, objectEventId, groupEventId);
  }   
}

bool mitk::HierarchicalInteractor::AddInteractor(mitk::Interactor::Pointer interactor)
{
  if (interactor.IsNotNull())
  {
    m_AllInteractors.push_back(interactor);
    return true;
  }
  else
    return false;
}