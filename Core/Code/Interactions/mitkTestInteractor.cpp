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

void mitk::TestInteractor::ConnectActionsAndFunctions()
{
  CONNECT_FUNCTION("addpoint", AddPoint);
  CONNECT_FUNCTION("selectpoint", SelectPoint);
  CONNECT_FUNCTION("unselect", DeSelectPoint);
  CONNECT_FUNCTION("deleteselection", DeleteSelectedPoint);
}

bool mitk::TestInteractor::AddPoint(StateMachineAction*, InteractionEvent* interactionEvent)
{

  InteractionPositionEvent* positionEvent = dynamic_cast<InteractionPositionEvent*> (interactionEvent);
  if (positionEvent != NULL) {


  mitk::Point3D point = positionEvent->GetPositionInWorld();
  point.Fill(3.0);
  m_PointSet->SetPoint(m_NumberOfPoints,point,0);
  m_NumberOfPoints++;
  GetDataNode()->SetData(m_PointSet);
  MITK_INFO<< "AddPoint";
  return true;
  } else
  {
    return false;
  }
}

bool mitk::TestInteractor::SelectPoint(StateMachineAction*, InteractionEvent*)
{
  //MITK_INFO<< "SelectPoint";
  return true;
}

bool mitk::TestInteractor::DeSelectPoint(StateMachineAction*, InteractionEvent*)
{
  //MITK_INFO<< "DeSelectPoint";
  return true;
}

mitk::TestInteractor::TestInteractor() :
m_NumberOfPoints(0)
{
  m_PointSet = PointSet::New();
}

mitk::TestInteractor::~TestInteractor()
{
}

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
  //MITK_INFO<< "DeleteSelectedPoint";
  return true;
}
