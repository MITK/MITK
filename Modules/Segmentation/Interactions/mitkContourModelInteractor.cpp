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
:Interactor("ContourModelInteractor", dataNode)
{
  CONNECT_ACTION( AcCHECKPOINT, OnCheckPointClick );
  CONNECT_ACTION( AcCHECKOBJECT, OnCheckContourClick );
  CONNECT_ACTION( AcDELETEPOINT, OnDeletePoint );
  CONNECT_ACTION( AcMOVEPOINT, OnMovePoint );
  CONNECT_ACTION( AcMOVE, OnMoveContour );
  CONNECT_ACTION( AcFINISH, OnFinish );
}


mitk::ContourModelInteractor::~ContourModelInteractor()
{
}


float mitk::ContourModelInteractor::CanHandleEvent(StateEvent const* stateEvent) const
{
  float returnValue = 0.0;
  //if it is a key event that can be handled in the current state, then return 0.5
  mitk::PositionEvent const  *positionEvent =
    dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());

  //Key event handling:
  if (positionEvent == NULL)
  {
    //check for delete and escape event
    if(stateEvent->GetId() == 12 || stateEvent->GetId() == 14)
    {
      return 1.0;
    }
    //check, if the current state has a transition waiting for that key event.
    else if (this->GetCurrentState()->GetTransition(stateEvent->GetId())!=NULL)
    {
      return 0.5;
    }
    else
    {
      return 0;
    }
  }

  int timestep = stateEvent->GetEvent()->GetSender()->GetTimeStep();

  mitk::ContourModel *contour = dynamic_cast<mitk::ContourModel *>(
    m_DataNode->GetData() );

  if ( contour != NULL )
  {

    mitk::Point3D worldPoint3D = positionEvent->GetWorldPosition();

    mitk::Geometry3D *contourGeometry =
      dynamic_cast< Geometry3D * >( contour->GetGeometry( timestep ) );

    if ( contourGeometry )
    {
      //if click is inside bounds the interactor can handle the event best
      if( contourGeometry->IsInside(worldPoint3D) )
      {
        return 1.0;
      }
      return 0.9;
    }
  }

  return returnValue;
}




void mitk::ContourModelInteractor::DataChanged()
{
  //go to initial state
  const mitk::Event* nullEvent = new mitk::Event(NULL, Type_User, BS_NoButton, BS_NoButton, Key_none);
  mitk::StateEvent* newStateEvent = new mitk::StateEvent(AcFINISH, nullEvent);
  this->HandleEvent( newStateEvent );
  delete newStateEvent;
  delete nullEvent;
  return;
}



bool mitk::ContourModelInteractor::OnCheckPointClick( Action* action, const StateEvent* stateEvent)
{
  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;


  mitk::StateEvent* newStateEvent = NULL;

  int timestep = stateEvent->GetEvent()->GetSender()->GetTimeStep();

  mitk::ContourModel *contour = dynamic_cast<mitk::ContourModel *>(
    m_DataNode->GetData() );


  contour->Deselect();

  /*
  * Check distance to any vertex.
  * Transition YES if click close to a vertex
  */
  mitk::Point3D click = positionEvent->GetWorldPosition();


  if (contour->SelectVertexAt(click, 1.5, timestep) )
  {
    contour->SetSelectedVertexAsControlPoint();
    assert( stateEvent->GetEvent()->GetSender()->GetRenderWindow() );
    mitk::RenderingManager::GetInstance()->RequestUpdate( stateEvent->GetEvent()->GetSender()->GetRenderWindow() );
    newStateEvent = new mitk::StateEvent(EIDYES, stateEvent->GetEvent());
    m_lastMousePosition = click;
  }
  else
  {
    newStateEvent = new mitk::StateEvent(EIDNO, stateEvent->GetEvent());
  }

  this->HandleEvent( newStateEvent );

  return true;
}



bool mitk::ContourModelInteractor::OnCheckContourClick( Action* action, const StateEvent* stateEvent)
{
  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;

  int timestep = stateEvent->GetEvent()->GetSender()->GetTimeStep();
  mitk::Point3D click = positionEvent->GetWorldPosition();
  mitk::StateEvent* newStateEvent = NULL;

  mitk::ContourModel *contour = dynamic_cast<mitk::ContourModel *>(
    m_DataNode->GetData() );
  mitk::Geometry3D *contourGeometry = dynamic_cast< Geometry3D * >( contour->GetGeometry( timestep ) );


  if ( contourGeometry->IsInside(click) )
  {
    m_lastMousePosition = click;
    newStateEvent = new mitk::StateEvent(EIDYES, stateEvent->GetEvent());
  }
  else
  {
    newStateEvent = new mitk::StateEvent(EIDNO, stateEvent->GetEvent());
  }

  this->HandleEvent( newStateEvent );

  return true;
}



bool mitk::ContourModelInteractor::OnDeletePoint( Action* action, const StateEvent* stateEvent)
{

  int timestep = stateEvent->GetEvent()->GetSender()->GetTimeStep();

  mitk::ContourModel *contour = dynamic_cast<mitk::ContourModel *>( m_DataNode->GetData() );

  contour->RemoveVertex(contour->GetSelectedVertex());


  return true;
}




bool mitk::ContourModelInteractor::OnMovePoint( Action* action, const StateEvent* stateEvent)
{
  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;


  mitk::ContourModel *contour = dynamic_cast<mitk::ContourModel *>( m_DataNode->GetData() );

  mitk::Vector3D translation;
  mitk::Point3D currentPosition = positionEvent->GetWorldPosition();
  translation[0] = currentPosition[0] - this->m_lastMousePosition[0];
  translation[1] = currentPosition[1] - this->m_lastMousePosition[1];
  translation[2] = currentPosition[2] - this->m_lastMousePosition[2];
  contour->ShiftSelectedVertex(translation);

  this->m_lastMousePosition = positionEvent->GetWorldPosition();

  assert( positionEvent->GetSender()->GetRenderWindow() );
  mitk::RenderingManager::GetInstance()->RequestUpdate( positionEvent->GetSender()->GetRenderWindow() );

  return true;
}



bool mitk::ContourModelInteractor::OnMoveContour( Action* action, const StateEvent* stateEvent)
{
  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;

  int timestep = positionEvent->GetSender()->GetTimeStep();

  mitk::ContourModel *contour = dynamic_cast<mitk::ContourModel *>( m_DataNode->GetData() );
  mitk::Vector3D translation;
  mitk::Point3D currentPosition = positionEvent->GetWorldPosition();
  translation[0] = currentPosition[0] - this->m_lastMousePosition[0];
  translation[1] = currentPosition[1] - this->m_lastMousePosition[1];
  translation[2] = currentPosition[2] - this->m_lastMousePosition[2];
  contour->ShiftContour(translation, timestep);

  this->m_lastMousePosition = positionEvent->GetWorldPosition();

  assert( positionEvent->GetSender()->GetRenderWindow() );
  mitk::RenderingManager::GetInstance()->RequestUpdate( positionEvent->GetSender()->GetRenderWindow() );

  return true;
}




bool mitk::ContourModelInteractor::OnFinish( Action* action, const StateEvent* stateEvent)
{

  mitk::ContourModel *contour = dynamic_cast<mitk::ContourModel *>( m_DataNode->GetData() );
  contour->Deselect();

  assert( stateEvent->GetEvent()->GetSender()->GetRenderWindow() );
  mitk::RenderingManager::GetInstance()->RequestUpdate( stateEvent->GetEvent()->GetSender()->GetRenderWindow() );

  return true;
}
