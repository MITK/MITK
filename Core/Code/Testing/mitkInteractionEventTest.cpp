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
  MITK_TEST_BEGIN("InteractionEvent")


  vtkRenderWindow  * renWin = vtkRenderWindow::New();
  mitk::VtkPropRenderer::Pointer renderer = mitk::VtkPropRenderer::New( "ContourRenderer",renWin, mitk::RenderingManager::GetInstance() );

  unsigned int buttonStates = mitk::BS_LeftButton || mitk::BS_RightButton;
  unsigned int eventButton = mitk::BS_LeftButton;
  unsigned int modifiers = mitk::BS_ControlButton || mitk::BS_AltButton;

  mitk::Point2D point;
  point[0] = 17;
  point[1] = 170;

  mitk::MousePressEvent me1 = mitk::MousePressEvent::New(renderer,point, buttonStates, modifiers, eventButton);
  mitk::MousePressEvent me2 = mitk::MousePressEvent::New(renderer,point, buttonStates, modifiers, eventButton);
  point[0] = 178;
  point[1] = 170;
  mitk::MousePressEvent me3 = mitk::MousePressEvent::New(renderer,point, buttonStates, modifiers, eventButton);
  modifiers = mitk::BS_ControlButton;
  mitk::MousePressEvent me4 = mitk::MousePressEvent::New(renderer,point, buttonStates, modifiers, eventButton);


  //check Get...
  MITK_TEST_CONDITION_REQUIRED(
      event->GetSender() == renderer ||
      event->GetType() == 0 ||
      event->GetButton() == 1 ||
      event->GetButtonState() == 2 ||
      event->GetKey() == 3
      , "Checking Get methods of mitk::Event");

  renWin->Delete();
  delete event;

  // always end with this!
  MITK_TEST_END()

}
