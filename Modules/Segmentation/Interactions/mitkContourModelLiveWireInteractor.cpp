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

#include "mitkContourModelLiveWireInteractor.h"

#include "mitkToolManager.h"

#include "mitkBaseRenderer.h"
#include "mitkRenderingManager.h"

#include <mitkPositionEvent.h>
#include <mitkStateEvent.h>

#include <mitkInteractionConst.h>


mitk::ContourModelLiveWireInteractor::ContourModelLiveWireInteractor(DataNode* dataNode)
:ContourModelInteractor(dataNode)
{
  m_LiveWireFilter = mitk::ImageLiveWireContourModelFilter::New();
}


mitk::ContourModelLiveWireInteractor::~ContourModelLiveWireInteractor()
{
}



bool mitk::ContourModelLiveWireInteractor::OnCheckPointClick( Action* action, const StateEvent* stateEvent)
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
    contour->SetSelectedVertexAcitve();

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



bool mitk::ContourModelLiveWireInteractor::OnDeletePoint( Action* action, const StateEvent* stateEvent)
{

  int timestep = stateEvent->GetEvent()->GetSender()->GetTimeStep();

  mitk::ContourModel *contour = dynamic_cast<mitk::ContourModel *>( m_DataNode->GetData() );

  mitk::ContourModel::VertexIterator end =contour->IteratorEnd();
  mitk::ContourModel::VertexIterator begin =contour->IteratorBegin();

  //search next active control point to left and rigth and set as start and end point for filter
  mitk::ContourModel::VertexIterator itUp = contour->IteratorBegin();

  //move iterator to position
  while((*itUp) != contour->GetSelectedVertex())
  {
    itUp++;
  }

  mitk::ContourModel::VertexIterator itDown = itUp;

  //step once down otherwise the the loop breaks immediately
  itUp++;
  itDown--;

  while(!((*itUp)->IsActive)){ itUp++; }

  while(!((*itDown)->IsActive)){ itDown--; }

    //recompute contour between neighbored two active control points
  this->m_LiveWireFilter->SetStartPoint( (*itDown)->Coordinates );
  this->m_LiveWireFilter->SetEndPoint( (*itUp)->Coordinates );
  this->m_LiveWireFilter->Update();

  mitk::ContourModel::Pointer liveWireContour = mitk::ContourModel::New();
  liveWireContour = this->m_LiveWireFilter->GetOutput();

  //construct a new contour
  mitk::ContourModel::Pointer newContour = mitk::ContourModel::New();
  mitk::ContourModel::VertexIterator it = contour->IteratorBegin();

  //copy from begin to itUP
  while(it != itDown)
  {
    newContour->AddVertex( (*it)->Coordinates, (*it)->IsActive, timestep);
    it++;
  }

  //add vertex at itDown - it's not considered during while loop
  if( it != end)
  {
    newContour->AddVertex( (*it)->Coordinates, (*it)->IsActive, timestep);
  }

  //insert new liveWire computed points
  newContour->Concatenate( liveWireContour, timestep);

  //copy the rest of the original contour
  it = itUp;
  while(it != end)
  {
    newContour->AddVertex( (*it)->Coordinates, (*it)->IsActive, timestep);
    it++;
  }

  newContour->SetIsClosed(contour->IsClosed(timestep), timestep);

  m_DataNode->SetData(newContour);


  assert( stateEvent->GetEvent()->GetSender()->GetRenderWindow() );
  mitk::RenderingManager::GetInstance()->RequestUpdate( stateEvent->GetEvent()->GetSender()->GetRenderWindow() );

  return true;
}




bool mitk::ContourModelLiveWireInteractor::OnMovePoint( Action* action, const StateEvent* stateEvent)
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


  //search for next active control points

  //Add indices of next active control points on each side of current point
  //SetEndIndex
  //AddEndIndex

  //start computation

  //retrieve both new paths
  //std::vector< std::vector< IndexType > > GetMultipleVectorPaths();

  this->m_lastMousePosition = positionEvent->GetWorldPosition();

  assert( positionEvent->GetSender()->GetRenderWindow() );
  mitk::RenderingManager::GetInstance()->RequestUpdate( positionEvent->GetSender()->GetRenderWindow() );

  return true;
}
