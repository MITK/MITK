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

#include "mitkInteractionEventConst.h"
#include "mitkMouseMoveEvent.h"
#include "mitkMousePressEvent.h"
#include "mitkMouseReleaseEvent.h"
#include "mitkTestingMacros.h"
#include "mitkVtkPropRenderer.h"

int mitkInteractionEventTest(int /*argc*/, char * /*argv*/ [])
{
  /*
   * Create different Events, fill them with data.
   * And check if isEqual method is implemented properly.
   */
  MITK_TEST_BEGIN("InteractionEvent")

  mitk::VtkPropRenderer::Pointer renderer = nullptr;

  mitk::InteractionEvent::MouseButtons buttonStates =
    mitk::InteractionEvent::LeftMouseButton | mitk::InteractionEvent::RightMouseButton;
  mitk::InteractionEvent::MouseButtons eventButton = mitk::InteractionEvent::LeftMouseButton;
  mitk::InteractionEvent::ModifierKeys modifiers = mitk::InteractionEvent::ControlKey | mitk::InteractionEvent::AltKey;

  mitk::Point2D point;
  point[0] = 17;
  point[1] = 170;

  mitk::Point3D worldPos;
  worldPos[0] = 0.5;
  worldPos[1] = 10.609;
  worldPos[2] = 5.0;

  // MousePress Events
  mitk::MousePressEvent::Pointer me1 =
    mitk::MousePressEvent::New(renderer, point, buttonStates, modifiers, eventButton);
  mitk::MousePressEvent::Pointer me2 =
    mitk::MousePressEvent::New(renderer, point, buttonStates, modifiers, eventButton);
  point[0] = 178;
  point[1] = 170;
  mitk::MousePressEvent::Pointer me3 =
    mitk::MousePressEvent::New(renderer, point, buttonStates, modifiers, eventButton);
  modifiers = mitk::InteractionEvent::ControlKey;
  mitk::MousePressEvent::Pointer me4 =
    mitk::MousePressEvent::New(renderer, point, buttonStates, modifiers, eventButton);

  MITK_TEST_CONDITION_REQUIRED(*me1 == *me2 && *me1 == *me3 && *me2 == *me3 && *me3 != *me4,
                               "Checking isEqual and Constructors of mitk::InteractionEvent, mitk::MousePressEvent");

  // MouseReleaseEvents
  mitk::MouseReleaseEvent::Pointer mr1 =
    mitk::MouseReleaseEvent::New(renderer, point, buttonStates, modifiers, eventButton);
  mitk::MouseReleaseEvent::Pointer mr2 =
    mitk::MouseReleaseEvent::New(renderer, point, buttonStates, modifiers, eventButton);
  point[0] = 178;
  point[1] = 170;
  mitk::MouseReleaseEvent::Pointer mr3 =
    mitk::MouseReleaseEvent::New(renderer, point, buttonStates, modifiers, eventButton);
  eventButton = mitk::InteractionEvent::RightMouseButton;
  mitk::MouseReleaseEvent::Pointer mr4 =
    mitk::MouseReleaseEvent::New(renderer, point, buttonStates, modifiers, eventButton);

  MITK_TEST_CONDITION_REQUIRED(*mr1 == *mr2 && *mr1 == *mr3 && *mr2 == *mr3 && *mr3 != *mr4,
                               "Checking isEqual and Constructors of mitk::InteractionEvent, mitk::MouseReleaseEvent");

  // MouseMoveEvents
  mitk::MouseMoveEvent::Pointer mm1 = mitk::MouseMoveEvent::New(renderer, point, buttonStates, modifiers);
  point[0] = 178;
  point[1] = 170;
  mitk::MouseMoveEvent::Pointer mm3 = mitk::MouseMoveEvent::New(renderer, point, buttonStates, modifiers);
  modifiers = mitk::InteractionEvent::AltKey;
  mitk::MouseMoveEvent::Pointer mm4 = mitk::MouseMoveEvent::New(renderer, point, buttonStates, modifiers);

  MITK_TEST_CONDITION_REQUIRED(*mm1 == *mm3 && *mm3 != *mm4,
                               "Checking isEqual and Constructors of mitk::InteractionEvent, mitk::MouseMoveEvent");

  // always end with this!
  MITK_TEST_END()
}
