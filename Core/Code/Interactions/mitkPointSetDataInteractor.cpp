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

}

bool mitk::PointSetDataInteractor::AddPoint(StateMachineAction*, InteractionEvent* interactionEvent)
{
  GetDataNode()->SetBoolProperty("show contour", true);

  InteractionPositionEvent* positionEvent = dynamic_cast<InteractionPositionEvent*>(interactionEvent);
  if (positionEvent != NULL)
  {
    mitk::Point3D point = positionEvent->GetPositionInWorld();
    m_PointSet->InsertPoint(m_NumberOfPoints, point, 0);
    m_NumberOfPoints++;
    GetDataNode()->SetData(m_PointSet);
    GetDataNode()->Modified();
    if (m_NumberOfPoints >= 3)
    {
      InternalEvent::Pointer event = InternalEvent::New(NULL,this, "has3points");
      positionEvent->GetSender()->GetDispatcher()->QueueEvent(event.GetPointer());
    }
    if (m_NumberOfPoints == 2)
      {
        InternalEvent::Pointer event = InternalEvent::New(NULL,this, "has2points");
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
    Point3D point = positionEvent->GetPositionInWorld();
    // iterate over point set and check if it contains a point close enough to the pointer to be selected
    if (GetPointIndexByPosition(point) != -1)
    {
      m_SelectedPointIndex = GetPointIndexByPosition(point);
      GetDataNode()->SetBoolProperty("show contour", true);
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

/*
 * Check whether the DataNode contains a pointset, if not create one and add it.
 */
mitk::PointSetDataInteractor::PointSetDataInteractor() :
    m_NumberOfPoints(0), m_SelectedPointIndex(-1)
{
  PointSet* points = dynamic_cast<PointSet*>(GetDataNode()->GetData());
  if (points == NULL)
  {
    m_PointSet = PointSet::New();

  } else
  {
    m_PointSet = points;
    GetDataNode()->SetData(m_PointSet);
  }
  //TODO initialization stuff
  // Check if loaded set matches descriptions such as maximal point number
}

mitk::PointSetDataInteractor::~PointSetDataInteractor()
{
}

bool mitk::PointSetDataInteractor::RemovePoint(StateMachineAction*, InteractionEvent*)
{
}

bool mitk::PointSetDataInteractor::CloseContourPoint(StateMachineAction*, InteractionEvent*)
{
}

bool mitk::PointSetDataInteractor::MovePoint(StateMachineAction*, InteractionEvent*)
{
}

bool mitk::PointSetDataInteractor::MoveSet(StateMachineAction*, InteractionEvent*)
{
}

bool mitk::PointSetDataInteractor::UnSelectPoint(StateMachineAction*, InteractionEvent*)
{
}

bool mitk::PointSetDataInteractor::Abort(StateMachineAction*, InteractionEvent* interactionEvent)
{
  MITK_INFO << "request abort";
  InternalEvent::Pointer event = InternalEvent::New(NULL,this, INTERNALDeactivateMe );
  interactionEvent->GetSender()->GetDispatcher()->QueueEvent(event.GetPointer());
  return true;
}
