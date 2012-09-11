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

  mitk::ContourModel::Pointer m_ContourLeft = mitk::ContourModel::New();
  mitk::ContourModel::Pointer m_ContourRight = mitk::ContourModel::New();
  m_NextActiveVertexDown.Fill(0);
  m_NextActiveVertexUp.Fill(0);
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


    mitk::ContourModel *contour = dynamic_cast<mitk::ContourModel *>( m_DataNode->GetData() );

    //
    m_ContourLeft = mitk::ContourModel::New();

    //get coordinates of next active vertex downwards from selected vertex
    int downIndex = SplitContourFromSelectedVertex( contour, m_ContourLeft, false, timestep);

    mitk::ContourModel::VertexIterator itDown = contour->IteratorBegin() + downIndex;
    m_NextActiveVertexDown = (*itDown)->Coordinates;


    //
    m_ContourRight = mitk::ContourModel::New();

    //get coordinates of next active vertex upwards from selected vertex
    int upIndex = SplitContourFromSelectedVertex( contour, m_ContourRight, true, timestep);

    mitk::ContourModel::VertexIterator itUp = contour->IteratorBegin() + upIndex;
    m_NextActiveVertexUp = (*itUp)->Coordinates;

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

  mitk::ContourModel::Pointer newContour = mitk::ContourModel::New();
  newContour->Expand(contour->GetTimeSteps());
  newContour->Concatenate( m_ContourLeft, timestep );


  //recompute contour between neighbored two active control points
  this->m_LiveWireFilter->SetStartPoint( m_NextActiveVertexDown );
  this->m_LiveWireFilter->SetEndPoint( m_NextActiveVertexUp );
  this->m_LiveWireFilter->Update();

  mitk::ContourModel::Pointer liveWireContour = mitk::ContourModel::New();
  liveWireContour = this->m_LiveWireFilter->GetOutput();


  //insert new liveWire computed points
  newContour->Concatenate( liveWireContour, timestep );

  newContour->Concatenate( m_ContourRight, timestep );

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

  int timestep = stateEvent->GetEvent()->GetSender()->GetTimeStep();

  mitk::ContourModel *contour = dynamic_cast<mitk::ContourModel *>( m_DataNode->GetData() );

  mitk::ContourModel::Pointer newContour = mitk::ContourModel::New();
  newContour->Expand(contour->GetTimeSteps());

  /*++++++++++++++ concatenate LEFT ++++++++++++++++++++++++++++++*/
  newContour->Concatenate( m_ContourLeft, timestep );


  mitk::Point3D currentPosition = positionEvent->GetWorldPosition();


  /*+++++++++++++++ start computation ++++++++++++++++++++*/
  //recompute contour between previous active vertex and selected vertex
  this->m_LiveWireFilter->SetStartPoint( m_NextActiveVertexDown );
  this->m_LiveWireFilter->SetEndPoint( currentPosition );
  this->m_LiveWireFilter->Update();

  mitk::ContourModel::Pointer liveWireContour = mitk::ContourModel::New();
  liveWireContour = this->m_LiveWireFilter->GetOutput();

  //remove point at current position because it is included in next livewire segment too.
  liveWireContour->RemoveVertexAt( liveWireContour->GetNumberOfVertices(timestep) - 1, timestep);

  /*++++++++++++++ concatenate LIVEWIRE ++++++++++++++++++++++++++++++*/
  //insert new liveWire computed points
  newContour->Concatenate( liveWireContour, timestep );


  //recompute contour between selected vertex and next active vertex
  this->m_LiveWireFilter->SetStartPoint( currentPosition );
  this->m_LiveWireFilter->SetEndPoint( m_NextActiveVertexUp );
  this->m_LiveWireFilter->Update();

  liveWireContour = this->m_LiveWireFilter->GetOutput();

  //make point at mouse position active again, so it is drawn
  const_cast<mitk::ContourModel::VertexType*>( liveWireContour->GetVertexAt(0, timestep) )->IsActive = true;

  /*++++++++++++++ concatenate RIGHT ++++++++++++++++++++++++++++++*/
  //insert new liveWire computed points
  newContour->Concatenate( liveWireContour, timestep );
  /*------------------------------------------------*/


  newContour->Concatenate( m_ContourRight, timestep );

  newContour->SetIsClosed(contour->IsClosed(timestep), timestep);

  m_DataNode->SetData(newContour);

  this->m_lastMousePosition = positionEvent->GetWorldPosition();

  assert( positionEvent->GetSender()->GetRenderWindow() );
  mitk::RenderingManager::GetInstance()->RequestUpdate( positionEvent->GetSender()->GetRenderWindow() );

  return true;
}



int mitk::ContourModelLiveWireInteractor::SplitContourFromSelectedVertex(mitk::ContourModel* sourceContour, mitk::ContourModel* destinationContour, bool fromSelectedUpwards, int timestep)
{

  mitk::ContourModel::VertexIterator end =sourceContour->IteratorEnd();
  mitk::ContourModel::VertexIterator begin =sourceContour->IteratorBegin();

  //search next active control point to left and rigth and set as start and end point for filter
  mitk::ContourModel::VertexIterator itSelected = sourceContour->IteratorBegin();


  //move iterator to position
  while((*itSelected) != sourceContour->GetSelectedVertex())
  {
    itSelected++;
  }


  if(fromSelectedUpwards)
  {
    mitk::ContourModel::VertexIterator itUp = itSelected;

    if(itUp != end)
    {
      itUp++;//step once up otherwise the the loop breaks immediately
    }

    while( itUp != end && !((*itUp)->IsActive)){ itUp++; }

    mitk::ContourModel::VertexIterator it = itUp;

    //copy the rest of the original contour
    while(it != end)
    {
      destinationContour->AddVertex( (*it)->Coordinates, (*it)->IsActive, timestep);
      it++;
    }

    //return the offset of iterator at one before next-vertex-upwards
    if(itUp != begin)
    {
      return itUp._Myoff - 1;
    }
    else
    {
      return itUp._Myoff;
    }

  }
  else //downwards
  {
    mitk::ContourModel::VertexIterator itDown = itSelected;

    if(itDown != begin)
    {
      itDown--;//step once down otherwise the the loop breaks immediately
    }

    while( itDown != begin && !((*itDown)->IsActive)){ itDown--; }

    mitk::ContourModel::VertexIterator it = sourceContour->IteratorBegin();

    if(it != end)//if not empty
    {
      //always add the first vertex
      destinationContour->AddVertex( (*it)->Coordinates, (*it)->IsActive, timestep);
      it++;
    }

    //copy from begin to itDown
    while(it <= itDown)
    {
      destinationContour->AddVertex( (*it)->Coordinates, (*it)->IsActive, timestep);
      it++;
    }

    //add vertex at itDown - it's not considered during while loop
    if( it != begin && it != end)
    {
      //destinationContour->AddVertex( (*it)->Coordinates, (*it)->IsActive, timestep);
    }

    //return the offset of iterator at one after next-vertex-downwards
    if( itDown != end)
    {
      return itDown._Myoff + 1;//index of next vertex
    }
    else
    {
      return itDown._Myoff - 1;
    }
  }
}
