/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkEvent.h"
#include "mitkVtkPropRenderer.h"
#include "mitkTestingMacros.h"
#include "mitkGlobalInteraction.h"


int mitkEventTest(int /*argc*/, char* /*argv*/[])
{
  MITK_TEST_BEGIN("Event")

  // Global interaction must(!) be initialized if used
  mitk::GlobalInteraction::GetInstance()->Initialize("global", NULL);

  vtkRenderWindow* renWin = vtkRenderWindow::New();
  mitk::VtkPropRenderer::Pointer renderer = mitk::VtkPropRenderer::New("ContourRenderer",renWin);

  //Create Event
  mitk::Event * event = new mitk::Event(renderer, 0, 1, 2, 3);

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
