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

bool mitk::HierarchicalInteractor::HandleEvent(StateEvent const* stateEvent, int objectEventId, int groupEventId)
{
  bool ok = Interactor::HandleEvent( stateEvent, objectEventId, groupEventId );

  if ( IsSubSelected() ) 
  {
    ok = TransmitEvent( stateEvent, objectEventId, groupEventId ) || ok ;
  }

  return ok;
}

bool mitk::HierarchicalInteractor::TransmitEvent( StateEvent const* stateEvent, int objectEventId, int groupEventId ) 
{
  InteractorListIter i = m_SelectedInteractors.begin();
  const InteractorListIter end = m_SelectedInteractors.end();
  bool ok = false;
  bool subSelected = false;

  while ( i != end )
  {
  
    ok = (*i)->HandleEvent( stateEvent, objectEventId, groupEventId ) || ok;
    subSelected = (*i)->IsSelected() || subSelected;
    i++;
  }

  if ( !subSelected ) 
  {
    StateEvent sd( EIDSUBDESELCT, stateEvent->GetEvent() );
    HandleEvent( &sd, objectEventId, groupEventId );
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
          float currentJurisdiction = CalculateJurisdiction( stateEvent );
    
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
          mitk::StateEvent* newStateEvent = new mitk::StateEvent(StYES, stateEvent->GetEvent());
          this->HandleEvent( newStateEvent, objectEventId, groupEventId );
        }
        else 
        {
          mitk::StateEvent* newStateEvent = new mitk::StateEvent(StNO, stateEvent->GetEvent());
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
          mitk::StateEvent* newStateEvent = new mitk::StateEvent(StYES, stateEvent->GetEvent());
          this->HandleEvent( newStateEvent, objectEventId, groupEventId );
        }
        else 
        {
          mitk::StateEvent* newStateEvent = new mitk::StateEvent(StNO, stateEvent->GetEvent());
          this->HandleEvent( newStateEvent, objectEventId, groupEventId );        
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