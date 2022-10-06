/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkContourModelLiveWireInteractor.h"

#include "mitkInteractionPositionEvent.h"
#include "mitkToolManager.h"

#include "mitkBaseRenderer.h"
#include "mitkRenderingManager.h"

#include <mitkInteractionConst.h>

#include "mitkIOUtil.h"

mitk::ContourModelLiveWireInteractor::ContourModelLiveWireInteractor() : ContourModelInteractor()
{
  m_LiveWireFilter = mitk::ImageLiveWireContourModelFilter::New();
  m_LiveWireFilter->SetUseCostFunction(true);
  m_NextActiveVertexDown.Fill(0);
  m_NextActiveVertexUp.Fill(0);
}

mitk::ContourModelLiveWireInteractor::~ContourModelLiveWireInteractor()
{
}

void mitk::ContourModelLiveWireInteractor::ConnectActionsAndFunctions()
{
  CONNECT_CONDITION("checkisOverPoint", OnCheckPointClick);
  CONNECT_CONDITION("mouseMove", IsHovering);

  CONNECT_FUNCTION("movePoint", OnMovePoint);
  CONNECT_FUNCTION("deletePoint", OnDeletePoint);
  CONNECT_FUNCTION("addPoint", OnAddPoint)
  CONNECT_FUNCTION("finish", OnFinishEditing);
}

bool mitk::ContourModelLiveWireInteractor::OnCheckPointClick(const InteractionEvent *interactionEvent)
{
  auto isVertexSelected = Superclass::OnCheckPointClick(interactionEvent);

  if (isVertexSelected)
  {
    auto* contour = dynamic_cast<mitk::ContourModel*>(this->GetDataNode()->GetData());
    const auto* positionEvent =
      dynamic_cast<const mitk::InteractionPositionEvent*>(interactionEvent);
    mitk::Point3D click = positionEvent->GetPositionInWorld();
    const auto timeStep = interactionEvent->GetSender()->GetTimeStep(GetDataNode()->GetData());

    auto controlVertices = contour->GetControlVertices(timeStep);
    const mitk::ContourModel::VertexType* nextPoint = contour->GetNextControlVertexAt(click, mitk::ContourModelLiveWireInteractor::eps, timeStep);
    const mitk::ContourModel::VertexType* previousPoint = contour->GetPreviousControlVertexAt(click, mitk::ContourModelLiveWireInteractor::eps, timeStep);
    this->SplitContourFromSelectedVertex(contour, nextPoint, previousPoint, timeStep);
    m_NextActiveVertexUp = nextPoint->Coordinates;
    m_NextActiveVertexDown = previousPoint->Coordinates;

    // clear previous void positions
    this->m_LiveWireFilter->ClearRepulsivePoints();

    // all points in lower and upper part should be marked as repulsive points to not be changed
    this->SetRepulsivePoints(previousPoint, m_ContourLeft, timeStep);
    this->SetRepulsivePoints(nextPoint, m_ContourRight, timeStep);

    // clear container with void points between neighboring control points
    m_ContourBeingModified.clear();
  }
  return isVertexSelected;
}

void mitk::ContourModelLiveWireInteractor::SetWorkingImage(mitk::Image *_arg)
{
  if (this->m_WorkingSlice != _arg)
  {
    this->m_WorkingSlice = _arg;
    this->m_LiveWireFilter->SetInput(this->m_WorkingSlice);
  }
}

void mitk::ContourModelLiveWireInteractor::OnAddPoint(StateMachineAction* sm, InteractionEvent* interactionEvent)
{
  Superclass::OnAddPoint(sm, interactionEvent);
}

void mitk::ContourModelLiveWireInteractor::OnDeletePoint(StateMachineAction *, InteractionEvent *interactionEvent)
{
  const auto timeStep = interactionEvent->GetSender()->GetTimeStep(GetDataNode()->GetData());

  auto *contour = dynamic_cast<mitk::ContourModel *>(this->GetDataNode()->GetData());
  if (contour == nullptr)
  {
    MITK_ERROR << "Invalid Contour!";
    return;
  }

  if (contour->GetSelectedVertex())
  {
    mitk::ContourModel::Pointer newContour = mitk::ContourModel::New();
    newContour->Expand(contour->GetTimeSteps());
    newContour->SetTimeGeometry(contour->GetTimeGeometry()->Clone());

    newContour->Concatenate(m_ContourLeft, timeStep);

    // recompute contour between neighbored two active control points
    this->m_LiveWireFilter->SetStartPoint(this->m_NextActiveVertexDown);
    this->m_LiveWireFilter->SetEndPoint(this->m_NextActiveVertexUp);
    this->m_LiveWireFilter->Update();

    mitk::ContourModel *liveWireContour = this->m_LiveWireFilter->GetOutput();
    assert(liveWireContour);

    if (liveWireContour->IsEmpty(timeStep))
      return;

    liveWireContour->RemoveVertexAt(0, timeStep);
    liveWireContour->RemoveVertexAt(liveWireContour->GetNumberOfVertices(timeStep) - 1, timeStep);

    // insert new live wire computed points
    newContour->Concatenate(liveWireContour, timeStep);

    // insert right side of original contour
    newContour->Concatenate(this->m_ContourRight, timeStep);

    newContour->SetClosed(contour->IsClosed(timeStep), timeStep);

    // instead of leaving a single point, delete all points
    if (newContour->GetNumberOfVertices(timeStep) <= 2)
    {
      newContour->Clear(timeStep);
    }

    this->GetDataNode()->SetData(newContour);

    mitk::RenderingManager::GetInstance()->RequestUpdate(interactionEvent->GetSender()->GetRenderWindow());
  }
}

void mitk::ContourModelLiveWireInteractor::OnMovePoint(StateMachineAction *, InteractionEvent *interactionEvent)
{
  const auto *positionEvent = dynamic_cast<const InteractionPositionEvent *>(interactionEvent);
  if (!positionEvent)
    return;

  const auto timeStep = interactionEvent->GetSender()->GetTimeStep(GetDataNode()->GetData());
  mitk::Point3D currentPosition = positionEvent->GetPositionInWorld();

  auto *contour = dynamic_cast<mitk::ContourModel *>(this->GetDataNode()->GetData());
  if (contour == nullptr)
  {
    MITK_ERROR << "invalid contour";
    return;
  }

  mitk::ContourModel::Pointer editingContour = mitk::ContourModel::New();
  editingContour->Expand(contour->GetTimeSteps());
  editingContour->SetTimeGeometry(contour->GetTimeGeometry()->Clone());

  // recompute left live wire, i.e. the contour between previous active vertex and selected vertex
  this->m_LiveWireFilter->SetStartPoint(this->m_NextActiveVertexDown);
  this->m_LiveWireFilter->SetEndPoint(currentPosition);

  // remove void positions between previous active vertex and next active vertex.
  if (!m_ContourBeingModified.empty())
  {
    std::vector<itk::Index<2>>::const_iterator iter = m_ContourBeingModified.begin();
    for (; iter != m_ContourBeingModified.end(); iter++)
    {
      this->m_LiveWireFilter->RemoveRepulsivePoint((*iter));
    }
  }

  // update to get the left livewire. Remember that the points in the rest of the contour are already
  // set as void positions in the filter
  this->m_LiveWireFilter->Update();

  mitk::ContourModel::Pointer leftLiveWire = this->m_LiveWireFilter->GetOutput();
  assert(leftLiveWire);

  if (!leftLiveWire->IsEmpty(timeStep))
    leftLiveWire->RemoveVertexAt(0, timeStep);

  editingContour->Concatenate(leftLiveWire, timeStep);

  // the new index of the selected vertex
  unsigned int selectedVertexIndex =
    this->m_ContourLeft->GetNumberOfVertices(timeStep) + leftLiveWire->GetNumberOfVertices(timeStep) - 1;

  // at this point the container has to be empty
  m_ContourBeingModified.clear();

  // add points from left live wire contour
  auto iter = leftLiveWire->IteratorBegin(timeStep);
  for (; iter != leftLiveWire->IteratorEnd(timeStep); iter++)
  {
    itk::Index<2> idx;
    this->m_WorkingSlice->GetGeometry()->WorldToIndex((*iter)->Coordinates, idx);
    this->m_LiveWireFilter->AddRepulsivePoint(idx);

    // add indices
    m_ContourBeingModified.push_back(idx);
  }

  // recompute right live wire, i.e. the contour between selected vertex and next active vertex
  this->m_LiveWireFilter->SetStartPoint(currentPosition);
  this->m_LiveWireFilter->SetEndPoint(m_NextActiveVertexUp);

  // update filter with all contour points set as void but the right live wire portion to be calculated now
  this->m_LiveWireFilter->Update();

  mitk::ContourModel::Pointer rightLiveWire = this->m_LiveWireFilter->GetOutput();
  assert(rightLiveWire);

  if (!leftLiveWire->IsEmpty(timeStep))
    leftLiveWire->SetControlVertexAt(leftLiveWire->GetNumberOfVertices() - 1, timeStep);

  if (!rightLiveWire->IsEmpty(timeStep))
    rightLiveWire->RemoveVertexAt(0, timeStep);

  editingContour->Concatenate(rightLiveWire, timeStep);

  mitk::ContourModel::Pointer newContour = mitk::ContourModel::New();
  newContour->Expand(contour->GetTimeSteps());
  newContour->SetTimeGeometry(contour->GetTimeGeometry()->Clone());

  // concatenate left original contour
  newContour->Concatenate(this->m_ContourLeft, timeStep);

  newContour->Concatenate(editingContour, timeStep, true);

  // set last inserted vertex as selected
  newContour->SelectVertexAt(selectedVertexIndex, timeStep);

  // set as control point
  newContour->SetSelectedVertexAsControlPoint(true);

  // concatenate right original contour
  newContour->Concatenate(this->m_ContourRight, timeStep);

  newContour->SetClosed(contour->IsClosed(timeStep), timeStep);
  this->GetDataNode()->SetData(newContour);

  mitk::RenderingManager::GetInstance()->RequestUpdate(positionEvent->GetSender()->GetRenderWindow());
}


void mitk::ContourModelLiveWireInteractor::SplitContourFromSelectedVertex(mitk::ContourModel *srcContour,
                                                                          const mitk::ContourModel::VertexType *nextPoint,
                                                                          const mitk::ContourModel::VertexType *previousPoint,
                                                                          int timeStep)
{
  m_ContourLeft = mitk::ContourModel::New();
  m_ContourRight = mitk::ContourModel::New();

  auto it = srcContour->IteratorBegin();
  // part between nextPoint and end of Countour
  bool upperPart = false;
  // part between start of countour and previousPoint
  bool lowerPart = true;

  // edge cases when point right before first control vertex is selected or first control vertex is selected
  if (nextPoint == (*it) || srcContour->GetSelectedVertex() == (*it))
  {
    upperPart = true;
    lowerPart = false;
    m_ContourLeft->AddVertex(previousPoint->Coordinates, previousPoint->IsControlPoint, timeStep);
  }
  // if first control vertex is selected, move to next point before adding vertices to m_ContourRight
  // otherwise, second line appears when moving the vertex
  if (srcContour->GetSelectedVertex() == (*it))
  {
    while (*it != nextPoint)
    {
      it++;
    }
  }

  for (; it != srcContour->IteratorEnd(timeStep); it++)
  {
    // everything in lower part should be added to m_CountoutLeft
    if (lowerPart)
    {
      m_ContourLeft->AddVertex((*it)->Coordinates, (*it)->IsControlPoint, timeStep);
    }
    // start of "restricted area" where no vertex should be added to m_CountoutLeft or m_CountoutRight
    if (*it == previousPoint)
    {
      lowerPart = false;
      upperPart = false;
    }
    // start of upperPart
    if (*it == nextPoint)
    {
      upperPart = true;
    }
    // everything in upper part should be added to m_CountoutRight
    if (upperPart)
    {
      m_ContourRight->AddVertex((*it)->Coordinates, (*it)->IsControlPoint, timeStep);
    }
  }
}

void mitk::ContourModelLiveWireInteractor::SetRepulsivePoints(const mitk::ContourModel::VertexType *pointToExclude,
                                                              mitk::ContourModel *contour,
                                                              int timeStep)
{
  auto it = contour->IteratorBegin();
  for (; it != contour->IteratorEnd(timeStep); it++)
  {
    if (*it != pointToExclude)
    {
      itk::Index<2> idx;
      this->m_WorkingSlice->GetGeometry()->WorldToIndex((*it)->Coordinates, idx);
      this->m_LiveWireFilter->AddRepulsivePoint(idx);
    }
  }
}

void mitk::ContourModelLiveWireInteractor::OnFinishEditing(StateMachineAction *, InteractionEvent *)
{

}
