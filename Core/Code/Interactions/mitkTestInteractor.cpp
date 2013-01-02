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

#include "mitkTestInteractor.h"
#include "mitkMouseMoveEvent.h"
#include <mitkPointOperation.h>
#include "mitkInteractionConst.h" // TODO: refactor file
#include "mitkRenderingManager.h"
void mitk::TestInteractor::ConnectActionsAndFunctions()
{
  CONNECT_FUNCTION("addpoint", AddPoint);
  CONNECT_FUNCTION("selectpoint", SelectPoint);
  CONNECT_FUNCTION("unselect", DeSelectPoint);
  CONNECT_FUNCTION("deleteselection", DeleteSelectedPoint);
}

bool mitk::TestInteractor::AddPoint(StateMachineAction*, InteractionEvent* interactionEvent)
{
  InteractionPositionEvent* positionEvent = dynamic_cast<InteractionPositionEvent*>(interactionEvent);
  if (positionEvent != NULL)
  {
    mitk::Point3D point = positionEvent->GetPositionInWorld();
    m_PointSet->InsertPoint(m_NumberOfPoints, point, 0);
    m_NumberOfPoints++;
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

bool mitk::TestInteractor::SelectPoint(StateMachineAction*, InteractionEvent* interactionEvent)
{
  InteractionPositionEvent* positionEvent = dynamic_cast<InteractionPositionEvent*>(interactionEvent);
  if (positionEvent != NULL)
  {
    Point3D point = positionEvent->GetPositionInWorld();
    // iterate over point set and check if it contains a point close enough to the pointer to be selected
    if (GetPointIndexByPosition(point) != -1)
    {
      m_SelectedPointIndex = GetPointIndexByPosition(point);
      return true;
    }
    return false;
  }
  else
  {
    return false;
  }
}

bool mitk::TestInteractor::DeSelectPoint(StateMachineAction*, InteractionEvent* interactionEvent)
{
  InteractionPositionEvent* positionEvent = dynamic_cast<InteractionPositionEvent*>(interactionEvent);
  if (positionEvent != NULL)
  {
    Point3D point = positionEvent->GetPositionInWorld();
    // delections action is executed, if pointer points to nothing, OR
    // is close to a different point. this is done to ensure there is a deselect transition between selecting a different point.
    if (GetPointIndexByPosition(point) == -1 || GetPointIndexByPosition(point) != m_SelectedPointIndex)
    {
      m_SelectedPointIndex = -1;
      return true;
    }
  }
  return false;
}

mitk::TestInteractor::TestInteractor() :
    m_NumberOfPoints(0)
{
  m_PointSet = PointSet::New();
  m_NumberOfPoints = 0;
  m_SelectedPointIndex = -1;
}

mitk::TestInteractor::~TestInteractor()
{
}

// TODO: probably obsolete! also in superclass!!
bool mitk::TestInteractor::IsPointerOverData(InteractionEvent* interactionEvent)
{
  // here we only want to handle mouse move events, since for all other events it is irrelevant if
  // we hover over an object
  MouseMoveEvent* mouseMoveEvent = dynamic_cast<MouseMoveEvent*>(interactionEvent);
  if (mouseMoveEvent != NULL)
  {
    return true;
  }
  else
  {
    return true; // we do not handle other events
  }

}

bool mitk::TestInteractor::DeleteSelectedPoint(StateMachineAction*, InteractionEvent*)
{
  if (m_SelectedPointIndex != -1)
  {
    Point3D point;
    PointOperation* doOp = new PointOperation(mitk::OpREMOVE, point, m_SelectedPointIndex);
    GetDataNode()->GetData()->ExecuteOperation(doOp);
    m_SelectedPointIndex = -1;
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    return true;
  }
  else
  {
    MITK_WARN << "Impossible/unexpected State";
    return false;
  }
}
