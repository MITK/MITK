/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// Testing
#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"
// MITK includes
#include <mitkCoreServices.h>
#include "mitkInteractionEventConst.h"
#include "mitkMouseMoveEvent.h"
#include "mitkMousePressEvent.h"
#include "mitkMouseReleaseEvent.h"
// VTK includes
#include "mitkVtkPropRenderer.h"

class mitkInteractionEventTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkInteractionEventTestSuite);
  MITK_TEST(MousePressEvent_Success);
  MITK_TEST(MouseReleaseEvent_Success);
  MITK_TEST(MouseMoveEvent_Success);
  CPPUNIT_TEST_SUITE_END();

private:
  mitk::VtkPropRenderer::Pointer m_Renderer;

  mitk::InteractionEvent::MouseButtons m_ButtonStates;
  mitk::InteractionEvent::MouseButtons m_EventButton;
  mitk::InteractionEvent::ModifierKeys m_Modifiers;
  
  mitk::Point2D m_Point;

  mitk::Point3D m_WorldPos;

public:
  void setUp()
  {
    m_Renderer = nullptr;
    m_ButtonStates = mitk::InteractionEvent::LeftMouseButton | mitk::InteractionEvent::RightMouseButton;
    m_EventButton = mitk::InteractionEvent::LeftMouseButton;
    m_Modifiers = mitk::InteractionEvent::ControlKey | mitk::InteractionEvent::AltKey;

    m_Point[0] = 17;
    m_Point[1] = 170;

    m_WorldPos[0] = 0.5;
    m_WorldPos[1] = 10.609;
    m_WorldPos[2] = 5.0;
  }
  void tearDown()
  {
  }

  void MousePressEvent_Success()
  {    
    // MousePress Events
    mitk::MousePressEvent::Pointer me1 =
      mitk::MousePressEvent::New(m_Renderer, m_Point, m_ButtonStates, m_Modifiers, m_EventButton);
    mitk::MousePressEvent::Pointer me2 =
      mitk::MousePressEvent::New(m_Renderer, m_Point, m_ButtonStates, m_Modifiers, m_EventButton);
    m_Point[0] = 178;
    m_Point[1] = 170;
    mitk::MousePressEvent::Pointer me3 =
      mitk::MousePressEvent::New(m_Renderer, m_Point, m_ButtonStates, m_Modifiers, m_EventButton);
    m_Modifiers = mitk::InteractionEvent::ControlKey;
    mitk::MousePressEvent::Pointer me4 =
      mitk::MousePressEvent::New(m_Renderer, m_Point, m_ButtonStates, m_Modifiers, m_EventButton);
    
    CPPUNIT_ASSERT_MESSAGE("Checking isEqual and Constructors of mitk::InteractionEvent, mitk::MousePressEvent",
                            *me1 == *me2 && *me1 == *me3 && *me2 == *me3 && *me3 != *me4);
  }

  void MouseReleaseEvent_Success()
  {
    // MouseReleaseEvents
    mitk::MouseReleaseEvent::Pointer mr1 =
      mitk::MouseReleaseEvent::New(m_Renderer, m_Point, m_ButtonStates, m_Modifiers, m_EventButton);
    mitk::MouseReleaseEvent::Pointer mr2 =
      mitk::MouseReleaseEvent::New(m_Renderer, m_Point, m_ButtonStates, m_Modifiers, m_EventButton);
    m_Point[0] = 178;
    m_Point[1] = 170;
    mitk::MouseReleaseEvent::Pointer mr3 =
      mitk::MouseReleaseEvent::New(m_Renderer, m_Point, m_ButtonStates, m_Modifiers, m_EventButton);
    m_EventButton = mitk::InteractionEvent::RightMouseButton;
    mitk::MouseReleaseEvent::Pointer mr4 =
      mitk::MouseReleaseEvent::New(m_Renderer, m_Point, m_ButtonStates, m_Modifiers, m_EventButton);
    
    CPPUNIT_ASSERT_MESSAGE("Checking isEqual and Constructors of mitk::InteractionEvent, mitk::MouseReleaseEvent",
                            *mr1 == *mr2 && *mr1 == *mr3 && *mr2 == *mr3 && *mr3 != *mr4);
  }

  void MouseMoveEvent_Success()
  {
    // MouseMoveEvents
    mitk::MouseMoveEvent::Pointer mm1 = mitk::MouseMoveEvent::New(m_Renderer, m_Point, m_ButtonStates, m_Modifiers);
    m_Point[0] = 178;
    m_Point[1] = 170;
    mitk::MouseMoveEvent::Pointer mm3 = mitk::MouseMoveEvent::New(m_Renderer, m_Point, m_ButtonStates, m_Modifiers);
    m_Modifiers = mitk::InteractionEvent::AltKey;
    mitk::MouseMoveEvent::Pointer mm4 = mitk::MouseMoveEvent::New(m_Renderer, m_Point, m_ButtonStates, m_Modifiers);
    
    CPPUNIT_ASSERT_MESSAGE("Checking isEqual and Constructors of mitk::InteractionEvent, mitk::MouseMoveEvent",
                            *mm1 == *mm3 && *mm3 != *mm4);
  }

};
MITK_TEST_SUITE_REGISTRATION(mitkInteractionEvent)
