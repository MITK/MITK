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

#include "mitkMousePressEvent.h"
#include "mitkVtkPropRenderer.h"
#include "mitkInteractionConst.h"
#include "mitkTestingMacros.h"

int mitkInteractionEventTest(int /*argc*/, char* /*argv*/[])
{
  /*
   * Create different Events, fill them with data.
   * And check if isEqual method is implemented properly.
   */


  MITK_TEST_BEGIN("InteractionEvent")

  mitk::VtkPropRenderer::Pointer renderer = NULL;

  unsigned int buttonStates = mitk::BS_LeftButton || mitk::BS_RightButton;
  unsigned int eventButton = mitk::BS_LeftButton;
  unsigned int modifiers = mitk::BS_ControlButton || mitk::BS_AltButton;

  mitk::Point2D point;
  point[0] = 17;
  point[1] = 170;

  mitk::MousePressEvent::Pointer me1 = mitk::MousePressEvent::New(renderer,point, buttonStates, modifiers, eventButton);
  mitk::MousePressEvent::Pointer me2 = mitk::MousePressEvent::New(renderer,point, buttonStates, modifiers, eventButton);
  point[0] = 178;
  point[1] = 170;
  mitk::MousePressEvent::Pointer me3 = mitk::MousePressEvent::New(renderer,point, buttonStates, modifiers, eventButton);
  modifiers = mitk::BS_ControlButton;
  mitk::MousePressEvent::Pointer me4 = mitk::MousePressEvent::New(renderer,point, buttonStates, modifiers, eventButton);

  MITK_TEST_CONDITION_REQUIRED(
      me1->isEqual(me2.GetPointer()) ||
      me1->isEqual(me3.GetPointer())||
      !(me1->isEqual(me3.GetPointer())) ||
      !(me3->isEqual(me4.GetPointer()))
      , "Checking isEqual and Constructors of mitk::InteractionEvent, mitk::MousePressEvent");

  // always end with this!
  MITK_TEST_END()

}
