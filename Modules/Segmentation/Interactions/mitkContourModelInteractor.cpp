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

#include "mitkContourModelInteractor.h"

#include "mitkToolManager.h"

#include "mitkBaseRenderer.h"
#include "mitkRenderingManager.h"

#include <mitkPositionEvent.h>
#include <mitkStateEvent.h>

#include <mitkInteractionConst.h>


mitk::ContourModelInteractor::ContourModelInteractor(DataNode* dataNode)
:Interactor("LiveWireInteractor", dataNode)
{

  CONNECT_ACTION( EActions::AcINITNEWOBJECT, OnInitLiveWire );
  CONNECT_ACTION( EActions::AcADDPOINT, OnAddPoint );
  CONNECT_ACTION( EActions::AcMOVE, OnMouseMoved );
  CONNECT_ACTION( EActions::AcCHECKPOINT, OnCheckPoint );
  CONNECT_ACTION( EActions::AcFINISH, OnFinish );
  CONNECT_ACTION( EActions::AcCLOSECONTOUR, OnCloseContour );
}


mitk::ContourModelInteractor::~ContourModelInteractor()
{
}


float mitk::ContourModelInteractor::CanHandleEvent(StateEvent const* stateEvent) const
{
  //float returnValue = 0.0;
  ////if it is a key event that can be handled in the current state, then return 0.5
  //mitk::DisplayPositionEvent const  *disPosEvent =
  //  dynamic_cast <const mitk::DisplayPositionEvent *> (stateEvent->GetEvent());

  ////Key event handling:
  //if (disPosEvent == NULL)
  //{
  //  //check, if the current state has a transition waiting for that key event.
  //  if (this->GetCurrentState()->GetTransition(stateEvent->GetId())!=NULL)
  //  {
  //    return 0.5;
  //  }
  //  else
  //  {
  //    return 0;
  //  }
  //}

  ////on MouseMove do nothing!
  //if (stateEvent->GetEvent()->GetType() == mitk::Type_MouseMove)
  //{
  //  return 0;
  //}

  ////get the time of the sender to look for the right transition.
  //mitk::BaseRenderer* sender = stateEvent->GetEvent()->GetSender();
  //if (sender != NULL)
  //{
  //  unsigned int timeStep = sender->GetTimeStep(m_DataNode->GetData());
  //  //if the event can be understood and if there is a transition waiting for that event
  //  mitk::State const* state = this->GetCurrentState(timeStep);
  //  if (state!= NULL)
  //    if (state->GetTransition(stateEvent->GetId())!=NULL)
  //      returnValue = 0.5;//it can be understood


  //  mitk::PointSet *pointSet = dynamic_cast<mitk::PointSet*>(m_DataNode->GetData());
  //  if ( pointSet != NULL )
  //  {
  //    //if we have one point or more, then check if the have been picked
  //    if ( (pointSet->GetSize( timeStep ) > 0)
  //      && (pointSet->SearchPoint(
  //      disPosEvent->GetWorldPosition(), m_Precision, timeStep) > -1) )
  //    {
  //      returnValue = 1.0;
  //    }
  //  }
  //}
  //return returnValue;
  return 0.5;
}



void mitk::ContourModelInteractor::DataChanged()
{
  const mitk::Event* nullEvent = new mitk::Event(NULL, Type_User, BS_NoButton, BS_NoButton, Key_none);
  mitk::StateEvent* newStateEvent = new mitk::StateEvent(AcFINISH, nullEvent);
  this->HandleEvent( newStateEvent );
  delete newStateEvent;
  delete nullEvent;
  return;
}


bool mitk::ContourModelInteractor::OnInitLiveWire (Action* action, const StateEvent* stateEvent)
{
  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;

  //m_LastEventSender = positionEvent->GetSender();
  //m_LastEventSlice = m_LastEventSender->GetSlice();

  if ( Superclass::CanHandleEvent(stateEvent) < 1.0 ) return false;

 // m_ToolManager->GetDataStorage()->Add( m_ContourModelNode );
  
  //m_Contour->AddVertex( const_cast<mitk::Point3D &>(positionEvent->GetWorldPosition()), true );

  assert( positionEvent->GetSender()->GetRenderWindow() );
  mitk::RenderingManager::GetInstance()->RequestUpdate( positionEvent->GetSender()->GetRenderWindow() );

  return true;
}


bool mitk::ContourModelInteractor::OnAddPoint (Action* action, const StateEvent* stateEvent)
{
  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;

  if ( Superclass::CanHandleEvent(stateEvent) < 1.0 ) return false;

  
  //m_Contour->AddVertex( const_cast<mitk::Point3D &>(positionEvent->GetWorldPosition()), true );

  assert( positionEvent->GetSender()->GetRenderWindow() );
  mitk::RenderingManager::GetInstance()->RequestUpdate( positionEvent->GetSender()->GetRenderWindow() );

  return true;
}


bool mitk::ContourModelInteractor::OnMouseMoved( Action* action, const StateEvent* stateEvent)
{
  if ( Superclass::CanHandleEvent(stateEvent) < 1.0 ) return false;

  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;


  //m_Contour->AddVertex( const_cast<mitk::Point3D &>(positionEvent->GetWorldPosition()) );

  assert( positionEvent->GetSender()->GetRenderWindow() );
  mitk::RenderingManager::GetInstance()->RequestUpdate( positionEvent->GetSender()->GetRenderWindow() );

  return true;
}


bool mitk::ContourModelInteractor::OnCheckPoint( Action* action, const StateEvent* stateEvent)
{
  if ( Superclass::CanHandleEvent(stateEvent) < 1.0 ) return false;

  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;


  mitk::StateEvent* newStateEvent = NULL;

  /* 
  * Check distance to first point.
  * Transition YES if click close to first control point 
  */
  //mitk::Point3D click = positionEvent->GetWorldPosition();

  //mitk::Point3D first = this->m_Contour->GetVertexAt(0)->Coordinates;


  //if (first.EuclideanDistanceTo(click) < 1.1)
  //{
  //  newStateEvent = new mitk::StateEvent(EIDYES, stateEvent->GetEvent());
  //}else
  //{
  //  newStateEvent = new mitk::StateEvent(EIDNO, stateEvent->GetEvent());
  //}
  ///* write new state (selected/not selected) to the property */
  //this->HandleEvent( newStateEvent );

  return true;
}


bool mitk::ContourModelInteractor::OnFinish( Action* action, const StateEvent* stateEvent)
{
  
  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;
  
  if ( CanHandleEvent(stateEvent) < 1.0 ) return false;


  //m_ContourModelNode->AddProperty( "color", ColorProperty::New(0.9, 1.0, 0.1), NULL, true );

  //set the livewire interactor to edit control points
  //m_ContourModelNode->SetInteractor(fooo);

  //m_Contour = mitk::ContourModel::New();
  //m_ContourModelNode = mitk::DataNode::New();
  //m_ContourModelNode->SetData( m_Contour );
  //m_ContourModelNode->SetProperty("name", StringProperty::New("anoter contour node"));
  //m_ContourModelNode->SetProperty("visible", BoolProperty::New(true));
  //m_ContourModelNode->AddProperty( "color", ColorProperty::New(0.1, 1.0, 0.1), NULL, true );

  this->Deactivated();

  return true;
}


bool mitk::ContourModelInteractor::OnCloseContour( Action* action, const StateEvent* stateEvent)
{
  
  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;
  
  if ( CanHandleEvent(stateEvent) < 1.0 ) return false;

  //m_Contour->Close();

  assert( positionEvent->GetSender()->GetRenderWindow() );
  mitk::RenderingManager::GetInstance()->RequestUpdate( positionEvent->GetSender()->GetRenderWindow() );

  return true;
}
    
