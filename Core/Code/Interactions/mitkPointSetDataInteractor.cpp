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
}

bool mitk::PointSetDataInteractor::AddPoint(StateMachineAction*,
    InteractionEvent* interactionEvent)
{
  // Find the position, the point is to be added to: first entry with
  // empty index. If the Set is empty, then start with 0. if not empty,
  // then take the first index which is not occupied
  int lastPosition = 0;
  PointSet::PointsContainer* pointsContainer =
      m_PointSet->GetPointSet(0)->GetPoints();

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

  InteractionPositionEvent* positionEvent =
      dynamic_cast<InteractionPositionEvent*>(interactionEvent);
  if (positionEvent != NULL)
  {
    // Get time step from BaseRenderer
    int timeStep = positionEvent->GetSender()->GetTimeStep();
    mitk::Point3D point = positionEvent->GetPositionInWorld();
    m_PointSet->InsertPoint(lastPosition, point, timeStep);
    GetDataNode()->SetData(m_PointSet);
    GetDataNode()->Modified();
    if (m_NumberOfPoints >= m_MaxNumberOfPoints)
    {
      InternalEvent::Pointer event = InternalEvent::New(NULL, this,
          "MaxNumberOfPoints");
      positionEvent->GetSender()->GetDispatcher()->QueueEvent(
          event.GetPointer());
    }
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    // TODO implement logic around this:
    // notify if last added point closed the contour
    IsContourClosed();
    return true;
  }
  else
  {
    return false;
  }
}

bool mitk::PointSetDataInteractor::SelectPoint(StateMachineAction*,
    InteractionEvent* interactionEvent)
{
  InteractionPositionEvent* positionEvent =
      dynamic_cast<InteractionPositionEvent*>(interactionEvent);
  if (positionEvent != NULL)
  {
    int timeStep = positionEvent->GetSender()->GetTimeStep();
    Point3D point = positionEvent->GetPositionInWorld();
    // iterate over point set and check if it contains a point close enough to the pointer to be selected
    if (GetPointIndexByPosition(point) != -1)
    {
      //TODO FIXME is this safe ? can pointset and renderer have different sizes ?
      m_SelectedPointIndex = GetPointIndexByPosition(point, timeStep);

      GetDataNode()->SetProperty("contourcolor",
          ColorProperty::New(0.0, 0.0, 1.0));
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

/*
 * Check whether the DataNode contains a pointset, if not create one and add it.
 */
mitk::PointSetDataInteractor::PointSetDataInteractor() :
    m_NumberOfPoints(0), m_SelectedPointIndex(-1)
{
}

mitk::PointSetDataInteractor::~PointSetDataInteractor()
{
}

bool mitk::PointSetDataInteractor::RemovePoint(StateMachineAction*,
    InteractionEvent*)
{
  return true;
}

bool mitk::PointSetDataInteractor::CloseContourPoint(StateMachineAction*,
    InteractionEvent*)
{
  return true;
}

bool mitk::PointSetDataInteractor::MovePoint(StateMachineAction*,
    InteractionEvent*)
{
  return true;
}

bool mitk::PointSetDataInteractor::MoveSet(StateMachineAction*,
    InteractionEvent*)
{
  return true;
}

bool mitk::PointSetDataInteractor::UnSelectPoint(StateMachineAction*,
    InteractionEvent*)
{
  return true;
}

bool mitk::PointSetDataInteractor::Abort(StateMachineAction*,
    InteractionEvent* interactionEvent)
{
  MITK_INFO<< "request abort";
  InternalEvent::Pointer event = InternalEvent::New(NULL,this, INTERNALDeactivateMe );
  interactionEvent->GetSender()->GetDispatcher()->QueueEvent(event.GetPointer());
  return true;
}

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
    //TODO initialization stuff
    // Check if loaded set matches descriptions such as maximal point number
  }
}

bool mitk::PointSetDataInteractor::IsContourClosed()
{
  return false; // well, it usually isn't .. ;)
}
