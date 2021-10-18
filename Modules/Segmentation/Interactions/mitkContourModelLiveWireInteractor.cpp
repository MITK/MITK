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
  CONNECT_FUNCTION("finish", OnFinishEditing);
}

bool mitk::ContourModelLiveWireInteractor::OnCheckPointClick(const InteractionEvent *interactionEvent)
{
  const auto *positionEvent = dynamic_cast<const InteractionPositionEvent *>(interactionEvent);

  if (!positionEvent)
    return false;

  const auto timeStep = interactionEvent->GetSender()->GetTimeStep(GetDataNode()->GetData());

  auto *contour = dynamic_cast<mitk::ContourModel *>(this->GetDataNode()->GetData());
  if (contour == nullptr)
  {
    MITK_ERROR << "Invalid Contour";
    return false;
  }

  contour->Deselect();

  // Check distance to any vertex.
  // Transition YES if click close to a vertex
  mitk::Point3D click = positionEvent->GetPositionInWorld();

  bool isVertexSelected = false;
  // Check, if clicked position is close to control vertex and if so, select closest control vertex.
  isVertexSelected = contour->SelectControlVertexAt(click, mitk::ContourModelLiveWireInteractor::eps, timeStep);

  // If the position is not close to control vertex. but hovering the contour line, we check, if it is close to non-control vertex.
  // The closest vertex will be set as a control vertex.
  if (isVertexSelected == false)
    isVertexSelected = contour->SelectVertexAt(click, mitk::ContourModelLiveWireInteractor::eps, timeStep);

   //  If the position is not close to control or non-control vertex. but hovering the contour line, we create a vertex at the position.
  if (isVertexSelected == false)
  {
    bool isHover = false;
    if (this->GetDataNode()->GetBoolProperty("contour.hovering", isHover, positionEvent->GetSender()) == false)
    {
      MITK_WARN << "Unknown property contour.hovering";
    }
    if (isHover)
    {
      contour->AddVertex(click, timeStep);
      isVertexSelected = contour->SelectVertexAt(click, mitk::ContourModelLiveWireInteractor::eps, timeStep);
    }
  }

  if (isVertexSelected)
  {
    contour->SetSelectedVertexAsControlPoint(true);
    auto controlVertices = contour->GetControlVertices(timeStep);
    const mitk::ContourModel::VertexType *nextPoint = contour->GetNextControlVertexAt(click, mitk::ContourModelLiveWireInteractor::eps, timeStep);
    const mitk::ContourModel::VertexType *previousPoint = contour->GetPreviousControlVertexAt(click, mitk::ContourModelLiveWireInteractor::eps, timeStep);
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

    // finally, return true to pass this condition
    return true;
  }
  else
  {
    // do not pass condition
    return false;
  }

  mitk::RenderingManager::GetInstance()->RequestUpdate(positionEvent->GetSender()->GetRenderWindow());
  return true;
}

void mitk::ContourModelLiveWireInteractor::SetEditingContourModelNode(mitk::DataNode *_arg)
{
  if (this->m_EditingContourNode != _arg)
  {
    this->m_EditingContourNode = _arg;
  }
}

void mitk::ContourModelLiveWireInteractor::SetWorkingImage(mitk::Image *_arg)
{
  if (this->m_WorkingSlice != _arg)
  {
    this->m_WorkingSlice = _arg;
    this->m_LiveWireFilter->SetInput(this->m_WorkingSlice);
  }
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

  // reject strange paths
  if (abs(rightLiveWire->GetNumberOfVertices(timeStep) - leftLiveWire->GetNumberOfVertices(timeStep)) > 50)
  {
    return;
  }

  if (!leftLiveWire->IsEmpty(timeStep))
    leftLiveWire->SetControlVertexAt(leftLiveWire->GetNumberOfVertices() - 1, timeStep);

  if (!rightLiveWire->IsEmpty(timeStep))
    rightLiveWire->RemoveVertexAt(0, timeStep);

  editingContour->Concatenate(rightLiveWire, timeStep);

  m_EditingContourNode->SetData(editingContour);

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

bool mitk::ContourModelLiveWireInteractor::IsHovering(const InteractionEvent *interactionEvent)
{
  const auto *positionEvent = dynamic_cast<const InteractionPositionEvent *>(interactionEvent);
  if (!positionEvent)
    return false;

  const auto timeStep = interactionEvent->GetSender()->GetTimeStep(GetDataNode()->GetData());

  auto *contour = dynamic_cast<mitk::ContourModel *>(this->GetDataNode()->GetData());

  mitk::Point3D currentPosition = positionEvent->GetPositionInWorld();

  bool isHover = false;
  this->GetDataNode()->GetBoolProperty("contour.hovering", isHover, positionEvent->GetSender());
  if (contour->IsNearContour(currentPosition, mitk::ContourModelLiveWireInteractor::eps, timeStep))
  {
    if (isHover == false)
    {
      this->GetDataNode()->SetBoolProperty("contour.hovering", true);
      mitk::RenderingManager::GetInstance()->RequestUpdate(positionEvent->GetSender()->GetRenderWindow());
    }
    return true;
  }
  else
  {
    if (isHover == true)
    {
      this->GetDataNode()->SetBoolProperty("contour.hovering", false);
      mitk::RenderingManager::GetInstance()->RequestUpdate(positionEvent->GetSender()->GetRenderWindow());
    }
  }
  return false;
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

void mitk::ContourModelLiveWireInteractor::OnFinishEditing(StateMachineAction *, InteractionEvent *interactionEvent)
{
  const auto timeStep = interactionEvent->GetSender()->GetTimeStep(GetDataNode()->GetData());

  auto *editingContour = dynamic_cast<mitk::ContourModel *>(this->m_EditingContourNode->GetData());

  editingContour->Clear(timeStep);

  mitk::RenderingManager::GetInstance()->RequestUpdate(interactionEvent->GetSender()->GetRenderWindow());
}
