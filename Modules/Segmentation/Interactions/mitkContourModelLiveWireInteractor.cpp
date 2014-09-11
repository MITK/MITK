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

#include "mitkIOUtil.h"

mitk::ContourModelLiveWireInteractor::ContourModelLiveWireInteractor(DataNode* dataNode)
:ContourModelInteractor(dataNode)
{

  m_LiveWireFilter = mitk::ImageLiveWireContourModelFilter::New();

  m_NextActiveVertexDown.Fill(0);
  m_NextActiveVertexUp.Fill(0);
}

mitk::ContourModelLiveWireInteractor::~ContourModelLiveWireInteractor()
{
}

bool mitk::ContourModelLiveWireInteractor::OnCheckPointClick( Action* action, const StateEvent* stateEvent)
{
  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());

  if (!positionEvent)
  {
    this->HandleEvent( new mitk::StateEvent(EIDNO, stateEvent->GetEvent()) );
    return false;
  }

  mitk::StateEvent* newStateEvent = NULL;

  int timestep = stateEvent->GetEvent()->GetSender()->GetTimeStep();

  mitk::ContourModel *contour = dynamic_cast<mitk::ContourModel *>( m_DataNode->GetData() );
  assert ( contour );

  contour->Deselect();

  // Check distance to any vertex.
  // Transition YES if click close to a vertex
  mitk::Point3D click = positionEvent->GetWorldPosition();

  if (contour->SelectVertexAt(click, 1.5, timestep) )
  {
    contour->SetSelectedVertexAsControlPoint(false);

    //m_lastMousePosition = click;

    m_ContourLeft = mitk::ContourModel::New();

    //get coordinates of next active vertex downwards from selected vertex
    int downIndex = this->SplitContourFromSelectedVertex( contour, m_ContourLeft, false, timestep);

    m_NextActiveVertexDownIter = contour->IteratorBegin() + downIndex;
    m_NextActiveVertexDown = (*m_NextActiveVertexDownIter)->Coordinates;

    m_ContourRight = mitk::ContourModel::New();

    //get coordinates of next active vertex upwards from selected vertex
    int upIndex = this->SplitContourFromSelectedVertex( contour, m_ContourRight, true, timestep);

    m_NextActiveVertexUpIter = contour->IteratorBegin() + upIndex;
    m_NextActiveVertexUp = (*m_NextActiveVertexUpIter)->Coordinates;

    // clear previous void positions
    this->m_LiveWireFilter->ClearRepulsivePoints();

    // set the current contour as void positions in the cost map
    // start with down side
    mitk::ContourModel::VertexIterator iter = contour->IteratorBegin(timestep);
    for (;iter != m_NextActiveVertexDownIter; iter++)
    {
      itk::Index<2> idx;
      this->m_WorkingSlice->GetGeometry()->WorldToIndex((*iter)->Coordinates, idx);
      this->m_LiveWireFilter->AddRepulsivePoint( idx );
    }

    // continue with upper side
    iter = m_NextActiveVertexUpIter + 1;
    for (;iter != contour->IteratorEnd(timestep); iter++)
    {
      itk::Index<2> idx;
      this->m_WorkingSlice->GetGeometry()->WorldToIndex((*iter)->Coordinates, idx);
      this->m_LiveWireFilter->AddRepulsivePoint( idx );
    }

    // clear container with void points between neighboring control points
    m_ContourBeingModified.clear();

    // let us have the selected point as a control point
    contour->SetSelectedVertexAsControlPoint(true);

    // finally, allow to leave current state
    newStateEvent = new mitk::StateEvent(EIDYES, stateEvent->GetEvent());
  }
  else
  {
    // do not allow to leave current state
    newStateEvent = new mitk::StateEvent(EIDNO, stateEvent->GetEvent());
  }

  this->HandleEvent( newStateEvent );

  assert( positionEvent->GetSender()->GetRenderWindow() );
  mitk::RenderingManager::GetInstance()->RequestUpdate( positionEvent->GetSender()->GetRenderWindow() );

  return true;
}

void mitk::ContourModelLiveWireInteractor::SetEditingContourModelNode (mitk::DataNode* _arg)
{
  if (this->m_EditingContourNode != _arg)
  {
    this->m_EditingContourNode = _arg;
    this->Modified();
  }
}

void mitk::ContourModelLiveWireInteractor::SetWorkingImage (mitk::Image* _arg)
{
  if (this->m_WorkingSlice != _arg)
  {
    this->m_WorkingSlice = _arg;
    this->m_LiveWireFilter->SetInput(this->m_WorkingSlice);
    this->Modified();
  }
}

bool mitk::ContourModelLiveWireInteractor::OnDeletePoint( Action* action, const StateEvent* stateEvent)
{

  int timestep = stateEvent->GetEvent()->GetSender()->GetTimeStep();

  mitk::ContourModel *contour = dynamic_cast<mitk::ContourModel *>( m_DataNode->GetData() );
  assert ( contour );

  if (contour->GetSelectedVertex())
  {

    mitk::ContourModel::Pointer newContour = mitk::ContourModel::New();
    newContour->Expand(contour->GetTimeSteps());

    newContour->Concatenate( m_ContourLeft, timestep );

    //recompute contour between neighbored two active control points
    this->m_LiveWireFilter->SetStartPoint( this->m_NextActiveVertexDown );
    this->m_LiveWireFilter->SetEndPoint( this->m_NextActiveVertexUp );
   // this->m_LiveWireFilter->ClearRepulsivePoints();
    this->m_LiveWireFilter->Update();

    mitk::ContourModel *liveWireContour = this->m_LiveWireFilter->GetOutput();
    assert ( liveWireContour );

    if ( liveWireContour->IsEmpty(timestep) )
        return false;

    liveWireContour->RemoveVertexAt( 0, timestep);
    liveWireContour->RemoveVertexAt( liveWireContour->GetNumberOfVertices(timestep) - 1, timestep);

    //insert new live wire computed points
    newContour->Concatenate( liveWireContour, timestep );

    // insert right side of original contour
    newContour->Concatenate( this->m_ContourRight, timestep );

    newContour->SetClosed(contour->IsClosed(timestep), timestep);

    // instead of leaving a single point, delete all points
    if (newContour->GetNumberOfVertices(timestep) <= 2){
      newContour->Clear(timestep);
    }

    m_DataNode->SetData(newContour);

    assert( stateEvent->GetEvent()->GetSender()->GetRenderWindow() );
    mitk::RenderingManager::GetInstance()->RequestUpdate( stateEvent->GetEvent()->GetSender()->GetRenderWindow() );

    return true;
  }
  return false;
}

bool mitk::ContourModelLiveWireInteractor::OnMovePoint( Action* action, const StateEvent* stateEvent)
{
  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;

  int timestep = stateEvent->GetEvent()->GetSender()->GetTimeStep();
  mitk::Point3D currentPosition = positionEvent->GetWorldPosition();

  mitk::ContourModel *contour = dynamic_cast<mitk::ContourModel *>( m_DataNode->GetData() );
  assert ( contour );

  mitk::ContourModel::Pointer editingContour = mitk::ContourModel::New();
  editingContour->Expand(contour->GetTimeSteps());

  // recompute left live wire, i.e. the contour between previous active vertex and selected vertex
  this->m_LiveWireFilter->SetStartPoint( this->m_NextActiveVertexDown );
  this->m_LiveWireFilter->SetEndPoint( currentPosition );

  // remove void positions between previous active vertex and next active vertex.
  if (!m_ContourBeingModified.empty())
  {
      std::vector< itk::Index< 2 > >::const_iterator iter = m_ContourBeingModified.begin();
      for (;iter != m_ContourBeingModified.end(); iter++)
      {
        this->m_LiveWireFilter->RemoveRepulsivePoint( (*iter) );
      }
  }

  // update to get the left livewire. Remember that the points in the rest of the contour are already
  // set as void positions in the filter
  this->m_LiveWireFilter->Update();

  mitk::ContourModel::Pointer leftLiveWire = this->m_LiveWireFilter->GetOutput();
  assert ( leftLiveWire );

  if ( !leftLiveWire->IsEmpty(timestep) )
    leftLiveWire->RemoveVertexAt(0, timestep);

  editingContour->Concatenate( leftLiveWire, timestep );

  //the new index of the selected vertex
  unsigned int selectedVertexIndex = this->m_ContourLeft->GetNumberOfVertices(timestep) + leftLiveWire->GetNumberOfVertices(timestep) -1;

  // at this point the container has to be empty
  m_ContourBeingModified.clear();

  // add points from left live wire contour
  mitk::ContourModel::VertexIterator iter = leftLiveWire->IteratorBegin(timestep);
  for (;iter != leftLiveWire->IteratorEnd(timestep); iter++)
  {
      itk::Index<2> idx;
      this->m_WorkingSlice->GetGeometry()->WorldToIndex((*iter)->Coordinates, idx);
      this->m_LiveWireFilter->AddRepulsivePoint( idx );

      // add indices
      m_ContourBeingModified.push_back(idx);
  }

  // recompute right live wire, i.e. the contour between selected vertex and next active vertex
  this->m_LiveWireFilter->SetStartPoint( currentPosition );
  this->m_LiveWireFilter->SetEndPoint( m_NextActiveVertexUp );

  // update filter with all contour points set as void but the right live wire portion to be calculated now
  this->m_LiveWireFilter->Update();

  mitk::ContourModel::Pointer rightLiveWire = this->m_LiveWireFilter->GetOutput();
  assert ( rightLiveWire );

  // reject strange paths
  if ( abs (rightLiveWire->GetNumberOfVertices(timestep) - leftLiveWire->GetNumberOfVertices(timestep)) > 50 )
  {
      return false;
  }

  if ( !leftLiveWire->IsEmpty(timestep) )
    leftLiveWire->SetControlVertexAt(leftLiveWire->GetNumberOfVertices()-1, timestep);

  if ( !rightLiveWire->IsEmpty(timestep) )
    rightLiveWire->RemoveVertexAt(0, timestep);

  editingContour->Concatenate( rightLiveWire, timestep );

// not really needed
/*
  // add points from right live wire contour
  iter = rightLiveWire->IteratorBegin(timestep);
  for (;iter != rightLiveWire->IteratorEnd(timestep); iter++)
  {
      itk::Index<2> idx;
      this->m_WorkingSlice->GetGeometry()->WorldToIndex((*iter)->Coordinates, idx);
      this->m_LiveWireFilter->AddRepulsivePoint( idx );
      // add indices
      m_ContourBeingModified.push_back(idx);
  }
*/


  m_EditingContourNode->SetData(editingContour);

  mitk::ContourModel::Pointer newContour = mitk::ContourModel::New();
  newContour->Expand(contour->GetTimeSteps());

  // concatenate left original contour
  newContour->Concatenate( this->m_ContourLeft, timestep );

  newContour->Concatenate( editingContour, timestep, true);

  // set last inserted vertex as selected
  newContour->SelectVertexAt(selectedVertexIndex, timestep);

  //set as control point
  newContour->SetSelectedVertexAsControlPoint(true);


  // concatenate right original contour
  newContour->Concatenate( this->m_ContourRight, timestep );

  newContour->SetClosed(contour->IsClosed(timestep), timestep);

  m_DataNode->SetData(newContour);

  //this->m_lastMousePosition = positionEvent->GetWorldPosition();

  assert( positionEvent->GetSender()->GetRenderWindow() );
  mitk::RenderingManager::GetInstance()->RequestUpdate( positionEvent->GetSender()->GetRenderWindow() );

  return true;
}

int mitk::ContourModelLiveWireInteractor::SplitContourFromSelectedVertex(mitk::ContourModel* srcContour,
                                                                         mitk::ContourModel* destContour,
                                                                         bool fromSelectedUpwards, int timestep)
{

  mitk::ContourModel::VertexIterator end = srcContour->IteratorEnd();
  mitk::ContourModel::VertexIterator begin = srcContour->IteratorBegin();

  //search next active control point to left and rigth and set as start and end point for filter
  mitk::ContourModel::VertexIterator itSelected = begin;

  // move iterator to position
  while ((*itSelected) != srcContour->GetSelectedVertex())
  {
    itSelected++;
  }

  // CASE search upwards for next control point
  if(fromSelectedUpwards)
  {
    mitk::ContourModel::VertexIterator itUp = itSelected;

    if(itUp != end)
    {
      itUp++;//step once up otherwise the loop breaks immediately
    }

    while( itUp != end && !((*itUp)->IsControlPoint))
    {
        itUp++;
    }

    mitk::ContourModel::VertexIterator it = itUp;

    if (itSelected != begin)
    {
      //copy the rest of the original contour
      while (it != end)
      {
        destContour->AddVertex( (*it)->Coordinates, (*it)->IsControlPoint, timestep);
        it++;
      }
    }
    //else do not copy the contour


    //return the offset of iterator at one before next-vertex-upwards
    if(itUp != begin)
    {
      return std::distance( begin, itUp) - 1;
    }
    else
    {
      return std::distance( begin, itUp);
    }

  }
  else //CASE search downwards for next control point
  {
    mitk::ContourModel::VertexIterator itDown = itSelected;
    mitk::ContourModel::VertexIterator it = srcContour->IteratorBegin();

    if( itSelected != begin )
    {
      if(itDown != begin)
      {
        itDown--;//step once down otherwise the the loop breaks immediately
      }

      while( itDown != begin && !((*itDown)->IsControlPoint)){ itDown--; }

      if(it != end)//if not empty
      {
        //always add the first vertex
        destContour->AddVertex( (*it)->Coordinates, (*it)->IsControlPoint, timestep);
        it++;
      }
      //copy from begin to itDown
      while(it <= itDown)
      {
        destContour->AddVertex( (*it)->Coordinates, (*it)->IsControlPoint, timestep);
        it++;
      }
    }
    else
    {
      //if selected vertex is the first element search from end of contour downwards
      itDown = end;
      itDown--;
      while(!((*itDown)->IsControlPoint) && itDown != begin){itDown--;}

      //move one forward as we don't want the first control point
      it++;
      //move iterator to second control point
      while( (it!=end) && !((*it)->IsControlPoint) ){it++;}
      //copy from begin to itDown
      while(it <= itDown)
      {
        //copy the contour from second control point to itDown
        destContour->AddVertex( (*it)->Coordinates, (*it)->IsControlPoint, timestep);
        it++;
      }
    }
/*
    //add vertex at itDown - it's not considered during while loop
    if( it != begin && it != end)
    {
      //destContour->AddVertex( (*it)->Coordinates, (*it)->IsControlPoint, timestep);
    }
*/
    //return the offset of iterator at one after next-vertex-downwards
    if( itDown != end)
    {
      return std::distance( begin, itDown);// + 1;//index of next vertex
    }
    else
    {
      return std::distance( begin, itDown) - 1;
    }
  }
}


bool mitk::ContourModelLiveWireInteractor::OnFinishEditing( Action* action, const StateEvent* stateEvent)
{
  int timestep = stateEvent->GetEvent()->GetSender()->GetTimeStep();

  mitk::ContourModel *editingContour = dynamic_cast<mitk::ContourModel *>( this->m_EditingContourNode->GetData() );
  assert ( editingContour );

  editingContour->Clear(timestep);
/*
  mitk::ContourModel *rightLiveWire = dynamic_cast<mitk::ContourModel *>( this->m_RightLiveWireContourNode->GetData() );
  assert ( rightLiveWire );

  rightLiveWire->Clear(timestep);
*/
  assert( stateEvent->GetEvent()->GetSender()->GetRenderWindow() );
  mitk::RenderingManager::GetInstance()->RequestUpdate( stateEvent->GetEvent()->GetSender()->GetRenderWindow() );

  return true;
}
