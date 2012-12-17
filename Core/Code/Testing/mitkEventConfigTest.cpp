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

#include "mitkTestingMacros.h"
#include "mitkEventConfig.h"
#include "mitkPropertyList.h"
#include "mitkInteractionEvent.h"
#include "mitkInteractionEventConst.h"
#include "mitkMouseMoveEvent.h"
#include "mitkMouseWheelEvent.h"
#include "mitkMouseReleaseEvent.h"
#include "mitkInteractionKeyEvent.h"
#include "mitkMousePressEvent.h"
#include <string>

int mitkEventConfigTest(int /*argc*/, char* /*argv*/[])
{
  MITK_TEST_BEGIN("EventConfig")

  /*
   * Loads a test a Config file and test if Config is build up correctly,
   * and if mapping from mitkEvents to EventVariant names works properly.
   * Indirectly this also tests the EventFactory Class, since we also test if the events have been constructed properly.
   */
    mitk::EventConfig* smc = mitk::EventConfig::New();

  MITK_TEST_CONDITION_REQUIRED(
      smc->LoadConfig("/home.local/webechr.local/EclipseTest/test/StatemachineConfigTest.xml") == true
       , "01 Check if file can be loaded" );

  /*
   * Test the global properties:
   * Test if stored values match the ones in the test config file.
   */
  mitk::PropertyList::Pointer properties = smc->GetPropertyList();
  std::string prop1, prop2;

  MITK_TEST_CONDITION_REQUIRED(
      properties->GetStringProperty("property1",prop1) &&
      prop1 == "yes" &&
      properties->GetStringProperty("scrollModus",prop2) &&
      prop2 == "leftright"
       , "02 Check Global Properties");


  /*
   * Check if Events get mapped to the proper Variants
   */

  mitk::Point2D pos;
  mitk::MousePressEvent::Pointer mpe1 = mitk::MousePressEvent::New(NULL,pos,mitk::MiddleMouseButton | mitk::LeftMouseButton ,mitk::ControlKey | mitk::AltKey,mitk::LeftMouseButton  );
  mitk::MousePressEvent::Pointer standard1 = mitk::MousePressEvent::New(NULL,pos,mitk::LeftMouseButton,mitk::NoKey ,mitk::LeftMouseButton );
  mitk::MouseMoveEvent::Pointer mme1 = mitk::MouseMoveEvent::New(NULL,pos,mitk::RightMouseButton | mitk::LeftMouseButton,mitk::ShiftKey );
  mitk::MouseMoveEvent::Pointer mme2 = mitk::MouseMoveEvent::New(NULL,pos,mitk::RightMouseButton,mitk::ShiftKey );
  mitk::MouseWheelEvent::Pointer mwe1 = mitk::MouseWheelEvent::New(NULL,pos,mitk::RightMouseButton,mitk::ShiftKey,-2 );
  mitk::InteractionKeyEvent::Pointer ke = mitk::InteractionKeyEvent::New(NULL,'l',mitk::NoKey );


  MITK_TEST_CONDITION_REQUIRED(
       smc->GetMappedEvent(mpe1.GetPointer()) == "Variant1" &&
       smc->GetMappedEvent(standard1.GetPointer()) == "Standard1" &&
       smc->GetMappedEvent(mme1.GetPointer()) == "Move2" &&
       smc->GetMappedEvent(ke.GetPointer()) == "Key1" &&
       smc->GetMappedEvent(mme2.GetPointer()) == "" // does not exist in file
        , "03 Check Mouse- and Key-Events "  );




  // always end with this!
  MITK_TEST_END()

}
