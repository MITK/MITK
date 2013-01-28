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

#include "mitkDisplayVectorInteractor.h"
#include "mitkBaseRenderer.h"
#include "mitkInteractionPositionEvent.h"
#include "mitkPropertyList.h"
#include <string.h>

void mitk::DisplayVectorInteractor::Notify(InteractionEvent::Pointer interactionEvent, bool isHandled)
{
  // to use the state machine pattern,
  // the event is passed to the state machine interface to be handled
  if (!isHandled)
  {
    this->HandleEvent(interactionEvent, NULL);
  }
}

void mitk::DisplayVectorInteractor::ConnectActionsAndFunctions()
{
  CONNECT_FUNCTION("init", Init);
  CONNECT_FUNCTION("move", Move);
  CONNECT_FUNCTION("zoom", Zoom);
}

mitk::DisplayVectorInteractor::DisplayVectorInteractor()
{
  m_StartDisplayCoordinate.Fill(0);
  m_LastDisplayCoordinate.Fill(0);
  m_CurrentDisplayCoordinate.Fill(0);
}

mitk::DisplayVectorInteractor::~DisplayVectorInteractor()
{
}

bool mitk::DisplayVectorInteractor::Init(StateMachineAction*, InteractionEvent* interactionEvent)
{
  BaseRenderer::Pointer sender = interactionEvent->GetSender();
  InteractionPositionEvent* positionEvent = dynamic_cast<InteractionPositionEvent*>(interactionEvent);
  if (positionEvent == NULL)
  {
    MITK_WARN<< "DisplayVectorInteractor cannot process the event: " << interactionEvent->GetEventClass();
    return false;
  }

  Vector2D origin = sender->GetDisplayGeometry()->GetOriginInMM();
  double scaleFactorMMPerDisplayUnit = sender->GetDisplayGeometry()->GetScaleFactorMMPerDisplayUnit();
  m_StartDisplayCoordinate = positionEvent->GetPointerPositionOnScreen();
  m_LastDisplayCoordinate = positionEvent->GetPointerPositionOnScreen();
  m_CurrentDisplayCoordinate = positionEvent->GetPointerPositionOnScreen();
  m_StartCoordinateInMM = mitk::Point2D(
      (origin + m_StartDisplayCoordinate.GetVectorFromOrigin() * scaleFactorMMPerDisplayUnit).GetDataPointer());
  return true;
}

bool mitk::DisplayVectorInteractor::Move(StateMachineAction*, InteractionEvent* interactionEvent)
{
  BaseRenderer::Pointer sender = interactionEvent->GetSender();
  InteractionPositionEvent* positionEvent = dynamic_cast<InteractionPositionEvent*>(interactionEvent);
  if (positionEvent == NULL)
  {
    MITK_WARN<< "DisplayVectorInteractor: cannot process the event in Move action: " << interactionEvent->GetEventClass();
    return false;
  }
  // perform translation
  sender->GetDisplayGeometry()->MoveBy((positionEvent->GetPointerPositionOnScreen() - m_LastDisplayCoordinate) * (-1.0));
  sender->GetRenderingManager()->RequestUpdate(sender->GetRenderWindow());
  m_LastDisplayCoordinate = positionEvent->GetPointerPositionOnScreen();
  return true;
}

bool mitk::DisplayVectorInteractor::Zoom(StateMachineAction*, InteractionEvent* interactionEvent)
{

  BaseRenderer::Pointer sender = interactionEvent->GetSender();
  InteractionPositionEvent* positionEvent = dynamic_cast<InteractionPositionEvent*>(interactionEvent);
  if (positionEvent == NULL)
  {
    MITK_WARN<< "DisplayVectorInteractor cannot process the event: " << interactionEvent->GetEventClass();
    return false;
  }

  mitk::PropertyList::Pointer properties = GetPropertyList();
  std::string strScrollDirection = "", strZoomFactor = "";

  properties->GetStringProperty("zoomFactor", strZoomFactor);
  float factor = 1.0, zoomFactor = .05;
  if (atoi(strZoomFactor.c_str()) > 0)
  {
    zoomFactor = 1.0 + (atoi(strZoomFactor.c_str()) / 100.0);
  }

  float distance = 0;
  properties->GetStringProperty("scrollDirection", strScrollDirection);
  if (strScrollDirection == "leftright")
  {

    distance = m_CurrentDisplayCoordinate[1] - m_LastDisplayCoordinate[1];
  }
  else
  {
    distance = m_CurrentDisplayCoordinate[0] - m_LastDisplayCoordinate[0];
  }

  if (distance < 0.0)
  {
    factor = 1.0 / zoomFactor;
  }
  else if (distance > 0.0)
  {
    factor = 1.0 * zoomFactor; // 5%
  }
  sender->GetDisplayGeometry()->ZoomWithFixedWorldCoordinates(factor, m_StartDisplayCoordinate, m_StartCoordinateInMM);
  sender->GetRenderingManager()->RequestUpdate(sender->GetRenderWindow());
  m_LastDisplayCoordinate = m_CurrentDisplayCoordinate;
  m_CurrentDisplayCoordinate = positionEvent->GetPointerPositionOnScreen();
  return true;
}
