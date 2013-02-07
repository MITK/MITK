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

#include "mitkPointSetDataInteractor.h"
#include "mitkMouseMoveEvent.h"
#include <mitkPointOperation.h>
#include "mitkInteractionConst.h" // TODO: refactor file
#include "mitkRenderingManager.h"
#include "mitkInternalEvent.h"
//
#include "mitkDispatcher.h"
#include "mitkBaseRenderer.h"

void mitk::PointSetDataInteractor::ConnectActionsAndFunctions()
{
  CONNECT_FUNCTION("addpoint", AddPoint);
  CONNECT_FUNCTION("selectpoint", SelectPoint);
  CONNECT_FUNCTION("unselect", UnSelectPoint);
  CONNECT_FUNCTION("initMove", InitMove);
  CONNECT_FUNCTION("initMovePointSet", InitMoveAll);
  CONNECT_FUNCTION("movePoint", MovePoint);
  CONNECT_FUNCTION("movePointSet", MoveSet);
  CONNECT_FUNCTION("finishMovement", FinishMove);
  CONNECT_FUNCTION("removePoint", RemovePoint);
}

bool mitk::PointSetDataInteractor::AddPoint(StateMachineAction* stateMachineAction, InteractionEvent* interactionEvent)
{
  // Find the position, the point is to be added to: first entry with
  // empty index. If the Set is empty, then start with 0. if not empty,
  // then take the first index which is not occupied
  int lastPosition = 0;
  PointSet::PointsContainer* pointsContainer = m_PointSet->GetPointSet(0)->GetPoints();

  if (!pointsContainer->empty())
  {
    mitk::PointSet::PointsIterator it, end;
    it = pointsContainer->Begin();
    end = pointsContainer->End();
    while (it != end)
    {
      if (!pointsContainer->IndexExists(lastPosition))
        break;
      ++it;
      ++lastPosition;
    }
  }

  InteractionPositionEvent* positionEvent = dynamic_cast<InteractionPositionEvent*>(interactionEvent);
  if (positionEvent != NULL)
  {
    IsClosedContour(stateMachineAction, interactionEvent);
    // Get time step from BaseRenderer
    int timeStep = positionEvent->GetSender()->GetTimeStep();
    mitk::Point3D point = positionEvent->GetPositionInWorld();
    m_PointSet->InsertPoint(lastPosition, point, timeStep);
    m_NumberOfPoints++;
    GetDataNode()->SetData(m_PointSet);
    GetDataNode()->Modified();
    if (m_MaxNumberOfPoints != 0 && m_NumberOfPoints >= m_MaxNumberOfPoints)
    {
      InternalEvent::Pointer event = InternalEvent::New(NULL, this, "MaxNumberOfPoints");
      positionEvent->GetSender()->GetDispatcher()->QueueEvent(event.GetPointer());
    }
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    return true;
  }
  else
  {
    return false;
  }
}

bool mitk::PointSetDataInteractor::SelectPoint(StateMachineAction*, InteractionEvent* interactionEvent)
{
  InteractionPositionEvent* positionEvent = dynamic_cast<InteractionPositionEvent*>(interactionEvent);
  if (positionEvent != NULL)
  {
    int timeStep = positionEvent->GetSender()->GetTimeStep();
    Point3D point = positionEvent->GetPositionInWorld();
    // iterate over point set and check if it contains a point close enough to the pointer to be selected
    if (GetPointIndexByPosition(point, timeStep) != -1)
    {
      //TODO FIXME is this safe ? can pointset and renderer have different sizes ?
      m_SelectedPointIndex = GetPointIndexByPosition(point, timeStep);
      GetDataNode()->SetProperty("contourcolor", ColorProperty::New(0.0, 0.0, 1.0));
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      return true;
    }
    return false;
  }
  else
  {
    return false;
  }
}

mitk::PointSetDataInteractor::PointSetDataInteractor() :
    m_NumberOfPoints(0), m_MaxNumberOfPoints(0),m_SelectedPointIndex(-1)
{
}

mitk::PointSetDataInteractor::~PointSetDataInteractor()
{
}

bool mitk::PointSetDataInteractor::RemovePoint(StateMachineAction*, InteractionEvent*)
{

  if (m_SelectedPointIndex != -1)
  {
    Point3D point;
    PointOperation* doOp = new PointOperation(mitk::OpREMOVE, point, m_SelectedPointIndex);
    GetDataNode()->GetData()->ExecuteOperation(doOp);
    m_SelectedPointIndex = -1;
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    m_NumberOfPoints--;
    return true;
  }
  else
  {
    return false;
  }
}

bool mitk::PointSetDataInteractor::IsClosedContour(StateMachineAction*, InteractionEvent* interactionEvent)
{
  InteractionPositionEvent* positionEvent = dynamic_cast<InteractionPositionEvent*>(interactionEvent);
  if (positionEvent != NULL)
  {
    int timeStep = positionEvent->GetSender()->GetTimeStep();
    Point3D point = positionEvent->GetPositionInWorld();
    // iterate over point set and check if it contains a point close enough to the pointer to be selected
    if (GetPointIndexByPosition(point, timeStep) != -1 && m_PointSet->GetSize(timeStep) >= 3)
    {
      InternalEvent::Pointer event = InternalEvent::New(NULL, this, "ClosedContour");
      positionEvent->GetSender()->GetDispatcher()->QueueEvent(event.GetPointer());
      return true;
    }

  }
  return false;
}

bool mitk::PointSetDataInteractor::MovePoint(StateMachineAction* stateMachineAction, InteractionEvent* interactionEvent)
{
  InteractionPositionEvent* positionEvent = dynamic_cast<InteractionPositionEvent*>(interactionEvent);
  if (positionEvent != NULL)
  {
    IsClosedContour(stateMachineAction, interactionEvent);
    // Get time step from BaseRenderer
    int timeStep = positionEvent->GetSender()->GetTimeStep();
    mitk::Point3D point = positionEvent->GetPositionInWorld();
    m_PointSet->SetPoint(m_SelectedPointIndex, point, timeStep);
    GetDataNode()->SetData(m_PointSet);
    GetDataNode()->Modified();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    IsClosedContour(stateMachineAction,interactionEvent);
    return true;
  }
  else
  {
    return false;
  }
}

bool mitk::PointSetDataInteractor::MoveSet(StateMachineAction*, InteractionEvent* interactionEvent)
{
  InteractionPositionEvent* positionEvent = dynamic_cast<InteractionPositionEvent*>(interactionEvent);
  if (positionEvent != NULL)
  {
    int timeStep = positionEvent->GetSender()->GetTimeStep();
    // Vector that represents movement relative to last position
    Point3D movementVector;
    movementVector[0] = positionEvent->GetPositionInWorld()[0] - m_PointSet->GetPoint(m_SelectedPointIndex, timeStep)[0];
    movementVector[1] = positionEvent->GetPositionInWorld()[1] - m_PointSet->GetPoint(m_SelectedPointIndex, timeStep)[1];
    movementVector[2] = positionEvent->GetPositionInWorld()[2] - m_PointSet->GetPoint(m_SelectedPointIndex, timeStep)[2];

    PointSet* points = dynamic_cast<PointSet*>(GetDataNode()->GetData());
    PointSet::PointsContainer* pointsContainer = points->GetPointSet(timeStep)->GetPoints();

    // Iterate over point set and update each point
    Point3D newPoint;
    for (PointSet::PointsIterator it = pointsContainer->Begin(); it != pointsContainer->End(); it++)
    {
      newPoint[0] = m_PointSet->GetPoint(it->Index(), timeStep)[0] + movementVector[0];
      newPoint[1] = m_PointSet->GetPoint(it->Index(), timeStep)[1] + movementVector[1];
      newPoint[2] = m_PointSet->GetPoint(it->Index(), timeStep)[2] + movementVector[2];
      m_PointSet->SetPoint(it->Index(), newPoint, timeStep);
    }

    GetDataNode()->SetData(m_PointSet);
    GetDataNode()->Modified();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    return true;
  }
  else
  {
    return false;
  }
}

bool mitk::PointSetDataInteractor::UnSelectPoint(StateMachineAction*, InteractionEvent* interactionEvent)
{
  InteractionPositionEvent* positionEvent = dynamic_cast<InteractionPositionEvent*>(interactionEvent);
  if (positionEvent != NULL)
  {
    int timeStep = positionEvent->GetSender()->GetTimeStep();
    Point3D point = positionEvent->GetPositionInWorld();
    // iterate over point set and check if it contains a point close enough to the pointer to be selected
    int index = GetPointIndexByPosition(point, timeStep);
    // here it is ensured that we don't switch from one point being selected to another one being selected,
    // without accepting the unselect of the current point
    if (index == -1 || index != m_SelectedPointIndex)
    {
      m_SelectedPointIndex = -1;
      GetDataNode()->SetProperty("contourcolor", ColorProperty::New(1.0, 0.0, 1.0));
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      return true;
    }
    return false;
  }
  else
  {
    return false;
  }
}

bool mitk::PointSetDataInteractor::Abort(StateMachineAction*, InteractionEvent* interactionEvent)
{
  InternalEvent::Pointer event = InternalEvent::New(NULL, this, IntDeactivateMe);
  interactionEvent->GetSender()->GetDispatcher()->QueueEvent(event.GetPointer());
  return true;
}

/*
 * Check whether the DataNode contains a pointset, if not create one and add it.
 */
void mitk::PointSetDataInteractor::DataNodeChanged()
{
  if (GetDataNode().IsNotNull())
  {
    // find proper place for this command!
    // maybe when DN is created ?
    GetDataNode()->SetBoolProperty("show contour", true);
    PointSet* points = dynamic_cast<PointSet*>(GetDataNode()->GetData());
    if (points == NULL)
    {
      m_PointSet = PointSet::New();
      GetDataNode()->SetData(m_PointSet);
    }
    else
    {
      m_PointSet = points;
    }
    // load config file parameter: maximal number of points
    mitk::PropertyList::Pointer properties = GetPropertyList();
    std::string strNumber;
    if (properties->GetStringProperty("MaxPoints", strNumber))
    {
      m_MaxNumberOfPoints = atoi(strNumber.c_str());
    }
  }
}

bool mitk::PointSetDataInteractor::InitMove(StateMachineAction*, InteractionEvent*)
{
  GetDataNode()->SetProperty("contourcolor", ColorProperty::New(1.0, 1.0, 1.0));
  return true;
}

bool mitk::PointSetDataInteractor::FinishMove(StateMachineAction* stateMachineAction, InteractionEvent* interactionEvent)
{
  IsClosedContour(stateMachineAction, interactionEvent);
  GetDataNode()->SetProperty("contourcolor", ColorProperty::New(1.0, 0.0, 0.0));
  return true;
}

bool mitk::PointSetDataInteractor::InitMoveAll(StateMachineAction*, InteractionEvent* interactionEvent)
{
  InteractionPositionEvent* positionEvent = dynamic_cast<InteractionPositionEvent*>(interactionEvent);
  if (positionEvent != NULL)
  {
    m_LastMovePosition = positionEvent->GetPositionInWorld();
    return true;
  }
  else
  {
    return false;
  }
}
