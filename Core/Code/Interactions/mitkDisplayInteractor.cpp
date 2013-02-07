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

#include "mitkDisplayInteractor.h"
#include "mitkBaseRenderer.h"
#include "mitkInteractionPositionEvent.h"
#include "mitkPropertyList.h"
#include <string.h>
// level window
#include "mitkStandaloneDataStorage.h"
#include "mitkNodePredicateDataType.h"
#include "mitkLevelWindowProperty.h"
#include "mitkLevelWindow.h"

void mitk::DisplayInteractor::Notify(InteractionEvent::Pointer interactionEvent, bool isHandled)
{
  // to use the state machine pattern,
  // the event is passed to the state machine interface to be handled
  if (!isHandled || m_AlwaysReact)
  {
    this->HandleEvent(interactionEvent, NULL);
  }
}

void mitk::DisplayInteractor::ConnectActionsAndFunctions()
{
  CONNECT_FUNCTION("init", Init);
  CONNECT_FUNCTION("move", Move);
  CONNECT_FUNCTION("zoom", Zoom);
  CONNECT_FUNCTION("scroll", Scroll);
  CONNECT_FUNCTION("ScrollOneDown", ScrollOneDown);
  CONNECT_FUNCTION("ScrollOneUp", ScrollOneUp);
  CONNECT_FUNCTION("levelWindow", AdjustLevelWindow);
}

mitk::DisplayInteractor::DisplayInteractor()
{
  m_StartDisplayCoordinate.Fill(0);
  m_LastDisplayCoordinate.Fill(0);
  m_CurrentDisplayCoordinate.Fill(0);
  m_IndexToSliceModifier = 4;
  m_AlwaysReact = false;
  m_AutoRepeat = false;
  m_ZoomFactor = 2;
}

mitk::DisplayInteractor::~DisplayInteractor()
{
}

bool mitk::DisplayInteractor::Init(StateMachineAction*, InteractionEvent* interactionEvent)
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

bool mitk::DisplayInteractor::Move(StateMachineAction*, InteractionEvent* interactionEvent)
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

bool mitk::DisplayInteractor::Zoom(StateMachineAction*, InteractionEvent* interactionEvent)
{
  BaseRenderer::Pointer sender = interactionEvent->GetSender();
  InteractionPositionEvent* positionEvent = dynamic_cast<InteractionPositionEvent*>(interactionEvent);
  if (positionEvent == NULL)
  {
    MITK_WARN<< "DisplayVectorInteractor cannot process the event: " << interactionEvent->GetEventClass();
    return false;
  }
  float factor = 1.0;
  float distance = 0;
  if (m_ZoomDirection == "leftright")
  {
    distance = m_CurrentDisplayCoordinate[1] - m_LastDisplayCoordinate[1];
  }
  else
  {
    distance = m_CurrentDisplayCoordinate[0] - m_LastDisplayCoordinate[0];
  }
  // set zooming speed
  if (distance < 0.0)
  {
    factor = 1.0 / m_ZoomFactor;
  }
  else if (distance > 0.0)
  {
    factor = 1.0 * m_ZoomFactor;
  }
  sender->GetDisplayGeometry()->ZoomWithFixedWorldCoordinates(factor, m_StartDisplayCoordinate, m_StartCoordinateInMM);
  sender->GetRenderingManager()->RequestUpdate(sender->GetRenderWindow());
  m_LastDisplayCoordinate = m_CurrentDisplayCoordinate;
  m_CurrentDisplayCoordinate = positionEvent->GetPointerPositionOnScreen();
  return true;
}

bool mitk::DisplayInteractor::Scroll(StateMachineAction*, InteractionEvent* interactionEvent)
{
  BaseRenderer::Pointer sender = interactionEvent->GetSender();
  InteractionPositionEvent* positionEvent = dynamic_cast<InteractionPositionEvent*>(interactionEvent);
  if (positionEvent == NULL)
  {
    MITK_WARN<< "DisplayVectorInteractor::Scroll cannot process the event: " << interactionEvent->GetEventClass();
    return false;
  }

  mitk::SliceNavigationController::Pointer sliceNaviController = interactionEvent->GetSender()->GetSliceNavigationController();
  if (sliceNaviController)
  {
    int delta = 0;
    // Scrolling direction
    if (m_ScrollDirection == "leftright")
    {
      delta = static_cast<int>(m_LastDisplayCoordinate[1] - positionEvent->GetPointerPositionOnScreen()[1]);
    }
    else
    {
      delta = static_cast<int>(m_LastDisplayCoordinate[0] - positionEvent->GetPointerPositionOnScreen()[0]);
    }
    // Set how many pixels the mouse has to be moved to scroll one slice
    // if we moved less than 'm_IndexToSliceModifier' pixels slice ONE slice only
    if (delta > 0 && delta < m_IndexToSliceModifier)
    {
      delta = m_IndexToSliceModifier;
    }
    else if (delta < 0 && delta > -m_IndexToSliceModifier)
    {
      delta = -m_IndexToSliceModifier;
    }
    delta /= m_IndexToSliceModifier;

    int newPos = sliceNaviController->GetSlice()->GetPos() + delta;

    // if auto repeat is on, start at first slice if you reach the last slice and vice versa
    int maxSlices = sliceNaviController->GetSlice()->GetSteps();
    if (m_AutoRepeat)
    {
      MITK_INFO<< m_AutoRepeat;
      while (newPos < 0)
      {
        newPos += maxSlices;
      }

      while (newPos >= maxSlices)
      {
        newPos -= maxSlices;
      }
    }
    else
    {
      // if the new slice is below 0 we still show slice 0
      // due to the stepper using unsigned int we have to do this ourselves
      if (newPos < 1)
      {
        newPos = 0;
      }
    }
    // set the new position
    sliceNaviController->GetSlice()->SetPos(newPos);
    m_LastDisplayCoordinate = m_CurrentDisplayCoordinate;
    m_CurrentDisplayCoordinate = positionEvent->GetPointerPositionOnScreen();
  }
  return true;
}

bool mitk::DisplayInteractor::ScrollOneDown(StateMachineAction*, InteractionEvent* interactionEvent)
{
  mitk::SliceNavigationController::Pointer sliceNaviController = interactionEvent->GetSender()->GetSliceNavigationController();
  if (!sliceNaviController->GetSliceLocked())
  {
    mitk::Stepper* stepper = sliceNaviController->GetSlice();
    if (stepper->GetSteps() <= 1)
    {
      stepper = sliceNaviController->GetTime();
    }
    stepper->Next();
  }
  return true;
}

bool mitk::DisplayInteractor::ScrollOneUp(StateMachineAction*, InteractionEvent* interactionEvent)
{
  mitk::SliceNavigationController::Pointer sliceNaviController = interactionEvent->GetSender()->GetSliceNavigationController();
  if (!sliceNaviController->GetSliceLocked())
  {
    mitk::Stepper* stepper = sliceNaviController->GetSlice();
    if (stepper->GetSteps() <= 1)
    {
      stepper = sliceNaviController->GetTime();
    }
    stepper->Previous();
    return true;
  }
  return false;
}

bool mitk::DisplayInteractor::AdjustLevelWindow(StateMachineAction*, InteractionEvent* interactionEvent)
{
  BaseRenderer::Pointer sender = interactionEvent->GetSender();
  InteractionPositionEvent* positionEvent = dynamic_cast<InteractionPositionEvent*>(interactionEvent);
  if (positionEvent == NULL)
  {
    MITK_WARN<< "DisplayVectorInteractor::Scroll cannot process the event: " << interactionEvent->GetEventClass();
    return false;
  }
  m_LastDisplayCoordinate = m_CurrentDisplayCoordinate;
  m_CurrentDisplayCoordinate = positionEvent->GetPointerPositionOnScreen();
  MITK_INFO<< "levelwindow";
  // search for active image
  mitk::DataStorage::Pointer storage = sender->GetDataStorage();
  mitk::DataNode::Pointer node = NULL;
  mitk::DataStorage::SetOfObjects::ConstPointer allImageNodes = storage->GetSubset(mitk::NodePredicateDataType::New("Image"));
  for (unsigned int i = 0; i < allImageNodes->size(); i++)
  {
    bool isActiveImage = false;
    bool propFound = allImageNodes->at(i)->GetBoolProperty("imageForLevelWindow", isActiveImage);

    if (propFound && isActiveImage)
    {
      node = allImageNodes->at(i);
      continue;
    }
  }
  if (node.IsNull())
  {
    node = storage->GetNode(mitk::NodePredicateDataType::New("Image"));
  }
  if (node.IsNull())
  {
    return false;
  }
  MITK_INFO<< "still here";

  mitk::LevelWindow lv = mitk::LevelWindow();
  node->GetLevelWindow(lv);
  ScalarType level = lv.GetLevel();
  ScalarType window = lv.GetWindow();
  // calculate adjustments from mouse movements
  level += (m_CurrentDisplayCoordinate[0] - m_LastDisplayCoordinate[0]) * static_cast<ScalarType>(2);
  window += (m_CurrentDisplayCoordinate[1] - m_LastDisplayCoordinate[1]) * static_cast<ScalarType>(2);

  lv.SetLevelWindow(level, window);
  dynamic_cast<mitk::LevelWindowProperty*>(node->GetProperty("levelwindow"))->SetLevelWindow(lv);

  sender->GetRenderingManager()->RequestUpdateAll();
  return true;
}

void mitk::DisplayInteractor::ConfigurationChanged()
{
  mitk::PropertyList::Pointer properties = GetPropertyList();
  // auto repeat
  std::string strAutoRepeat = "";
  if (properties->GetStringProperty("autoRepeat", strAutoRepeat))
  {
    MITK_INFO<< strAutoRepeat;
    if (strAutoRepeat == "true")
    {
      m_AutoRepeat = true;
    }
    else
    {
      m_AutoRepeat = false;
    }
  }
  // pixel movement for scrolling one slice
  std::string strPixelPerSlice = "";
  if (properties->GetStringProperty("pixelPerSlice", strPixelPerSlice))
  {
    MITK_INFO<< "pix"; //kommt nicht an
    m_IndexToSliceModifier = atoi(strPixelPerSlice.c_str());
  }
  else
  {
    m_IndexToSliceModifier = 4;
  }
  // scroll direction
  if (!properties->GetStringProperty("zoomDirection", m_ScrollDirection))
  {
    m_ScrollDirection = "updown";
  }
  // zoom direction
  if (!properties->GetStringProperty("zoomDirection", m_ZoomDirection))
  {
    m_ZoomDirection = "updown";
  }
  // zoom factor
  std::string strZoomFactor = "";
  properties->GetStringProperty("zoomFactor", strZoomFactor);
  m_ZoomFactor = .05;
  if (atoi(strZoomFactor.c_str()) > 0)
  {
    m_ZoomFactor = 1.0 + (atoi(strZoomFactor.c_str()) / 100.0);
  }
  // allwaysReact
  std::string strAlwaysReact = "";
  if (properties->GetStringProperty("alwaysReact", strAlwaysReact))
  {
    if (strAlwaysReact == "true")
    {
      m_AlwaysReact = true;
    }
    else
    {
      m_AlwaysReact = false;
    }
  }
  else
  {
    m_AlwaysReact = false;
  }
}
