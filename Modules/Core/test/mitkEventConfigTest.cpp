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
#include "usModule.h"
#include "usGetModuleContext.h"
#include <string>
#include <iostream>
#include <fstream>

int mitkEventConfigTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("EventConfig")

  if (argc != 2) {
    MITK_ERROR << "Test needs configuration test file as parameter.";
    return -1;
  }


  /*
   * Loads a test a Config file and test if Config is build up correctly,
   * and if mapping from mitkEvents to EventVariant names works properly.
   * Indirectly this also tests the EventFactory Class, since we also test if the events have been constructed properly.
   *
   * The configuration object is constructed in three different ways,
   * each one is tested here.
   */

  // Construction using compiled-in resrouces:
   us::Module *module = us::GetModuleContext()->GetModule();
   mitk::EventConfig newConfig("StatemachineConfigTest.xml",module);

  MITK_TEST_CONDITION_REQUIRED(
         newConfig.IsValid() == true
          , "01 Check if file can be loaded and is valid" );
  /*
   * Test the global properties:
   * Test if stored values match the ones in the test config file.
   */
  mitk::PropertyList::Pointer properties = newConfig.GetAttributes();
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
  mitk::MousePressEvent::Pointer mpe1 = mitk::MousePressEvent::New(NULL,pos,mitk::InteractionEvent::MiddleMouseButton | mitk::InteractionEvent::LeftMouseButton ,mitk::InteractionEvent::ControlKey | mitk::InteractionEvent::AltKey,mitk::InteractionEvent::LeftMouseButton  );
  mitk::MousePressEvent::Pointer standard1 = mitk::MousePressEvent::New(NULL,pos,mitk::InteractionEvent::LeftMouseButton,mitk::InteractionEvent::NoKey ,mitk::InteractionEvent::LeftMouseButton );
  mitk::MouseMoveEvent::Pointer mme1 = mitk::MouseMoveEvent::New(NULL,pos,mitk::InteractionEvent::RightMouseButton | mitk::InteractionEvent::LeftMouseButton,mitk::InteractionEvent::ShiftKey );
  mitk::MouseMoveEvent::Pointer mme2 = mitk::MouseMoveEvent::New(NULL,pos,mitk::InteractionEvent::RightMouseButton,mitk::InteractionEvent::ShiftKey );
  mitk::MouseWheelEvent::Pointer mwe1 = mitk::MouseWheelEvent::New(NULL,pos,mitk::InteractionEvent::RightMouseButton,mitk::InteractionEvent::ShiftKey,-2 );
  mitk::InteractionKeyEvent::Pointer ke = mitk::InteractionKeyEvent::New(NULL,"l",mitk::InteractionEvent::NoKey );

  MITK_TEST_CONDITION_REQUIRED(
        newConfig.GetMappedEvent(mpe1.GetPointer()) == "Variant1" &&
        newConfig.GetMappedEvent(standard1.GetPointer()) == "Standard1" &&
        newConfig.GetMappedEvent(mme1.GetPointer()) == "Move2" &&
        newConfig.GetMappedEvent(ke.GetPointer()) == "Key1" &&
        newConfig.GetMappedEvent(mme2.GetPointer()) == "" // does not exist in file
        , "03 Check Mouse- and Key-Events "  );

  // Construction providing a input stream
  std::ifstream configStream(argv[1]);
  mitk::EventConfig newConfig2(configStream);

  MITK_TEST_CONDITION_REQUIRED(
         newConfig2.IsValid() == true
          , "01 Check if file can be loaded and is valid" );
  /*
   * Test the global properties:
   * Test if stored values match the ones in the test config file.
   */
  properties = newConfig2.GetAttributes();
    MITK_TEST_CONDITION_REQUIRED(
        properties->GetStringProperty("property1",prop1) &&
        prop1 == "yes" &&
        properties->GetStringProperty("scrollModus",prop2) &&
        prop2 == "leftright"
        , "02 Check Global Properties");

  /*
   * Check if Events get mapped to the proper Variants
   */
  MITK_TEST_CONDITION_REQUIRED(
        newConfig2.GetMappedEvent(mpe1.GetPointer()) == "Variant1" &&
        newConfig2.GetMappedEvent(standard1.GetPointer()) == "Standard1" &&
        newConfig2.GetMappedEvent(mme1.GetPointer()) == "Move2" &&
        newConfig2.GetMappedEvent(ke.GetPointer()) == "Key1" &&
        newConfig2.GetMappedEvent(mme2.GetPointer()) == "" // does not exist in file
        , "03 Check Mouse- and Key-Events "  );

  // always end with this!

  // Construction providing a property list
  std::vector<mitk::PropertyList::Pointer> configDescription;

  mitk::PropertyList::Pointer propertyList1 = mitk::PropertyList::New();
  propertyList1->SetStringProperty(mitk::InteractionEventConst::xmlParameterEventClass().c_str(), "MousePressEvent");
  propertyList1->SetStringProperty(mitk::InteractionEventConst::xmlParameterEventVariant().c_str(), "MousePressEventVariant");
  propertyList1->SetStringProperty("Modifiers","CTRL,ALT");
  configDescription.push_back(propertyList1);

  mitk::PropertyList::Pointer propertyList2 = mitk::PropertyList::New();
  propertyList2->SetStringProperty(mitk::InteractionEventConst::xmlParameterEventClass().c_str(), "MOUSERELEASEEVENT");
  propertyList2->SetStringProperty(mitk::InteractionEventConst::xmlParameterEventVariant().c_str(), "MouseReleaseEventVariant");
  propertyList2->SetStringProperty("Modifiers","SHIFT");
  configDescription.push_back(propertyList2);

  mitk::PropertyList::Pointer propertyList3 = mitk::PropertyList::New();
  propertyList3->SetStringProperty(mitk::InteractionEventConst::xmlParameterEventClass().c_str(), "MOUSERELEASEEVENT");
  propertyList3->SetStringProperty(mitk::InteractionEventConst::xmlParameterEventVariant().c_str(), "MouseReleaseEventVariant");
  propertyList3->SetStringProperty("Modifiers","ALT");
  configDescription.push_back(propertyList3);

  mitk::EventConfig newConfig3( configDescription );

  mitk::MousePressEvent::Pointer mousePress1 = mitk::MousePressEvent::New(NULL,pos,mitk::InteractionEvent::NoButton,mitk::InteractionEvent::AltKey | mitk::InteractionEvent::ControlKey ,mitk::InteractionEvent::NoButton );
  mitk::MouseReleaseEvent::Pointer mouseRelease1 = mitk::MouseReleaseEvent::New(NULL,pos,mitk::InteractionEvent::NoButton,mitk::InteractionEvent::ShiftKey ,mitk::InteractionEvent::NoButton );

  // create a second event with the same name but different modifiers...
  mitk::MouseReleaseEvent::Pointer mouseRelease2 = mitk::MouseReleaseEvent::New(NULL,pos,mitk::InteractionEvent::NoButton,mitk::InteractionEvent::AltKey ,mitk::InteractionEvent::NoButton );

  MITK_TEST_CONDITION_REQUIRED(
    newConfig3.GetMappedEvent(mousePress1.GetPointer()) == "MousePressEventVariant" &&
    newConfig3.GetMappedEvent(mouseRelease1.GetPointer()) == "MouseReleaseEventVariant" &&
    newConfig3.GetMappedEvent(mouseRelease2.GetPointer()) == "MouseReleaseEventVariant",
    "04 Check Mouseevents from PropertyLists"  );


  MITK_TEST_END()

}
