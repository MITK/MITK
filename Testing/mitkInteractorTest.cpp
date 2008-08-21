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


#include <mitkInteractor.h>
#include <mitkEvent.h>
#include <mitkStateEvent.h>
#include <mitkInteractionConst.h>
#include <iostream>
#include "mitkTestingMacros.h"

int mitkInteractorTest(int /*argc*/, char* /*argv*/[])
{
  MITK_TEST_BEGIN("Interactor")

  //create interactor; use pattern InteractorTestPattern
  mitk::Interactor::Pointer interactor = mitk::Interactor::New("InteractorTestPattern", NULL);

  //load interactor
  std::cout << "Testing to initialize the interactor";
  MITK_TEST_CONDITION_REQUIRED(interactor.IsNotNull(),"Testing to initialize the interactor")
  
  std::cout<<"The pattern of the interactor is called: "<<interactor->GetType()<<std::endl;
  
  //creating an event to go from state 0 to state 1:
  //<!-- middle MouseButton -->
  //<event NAME="middle MouseBN" ID="4" TYPE="Type_MouseButtonPress" BUTTON="BS_MidButton" BUTTONSTATE="0x0000" KEY="Key_none" />
  mitk::Event *event = new mitk::Event(NULL, mitk::Type_MouseButtonPress, mitk::BS_MidButton, mitk::BS_NoButton, mitk::Key_none);
  mitk::StateEvent *stateEvent = new mitk::StateEvent(4, event);
  MITK_TEST_CONDITION_REQUIRED(interactor->HandleEvent(stateEvent),"Testing to send an event to go from State 0 to State 1")
  
  MITK_TEST_CONDITION_REQUIRED(interactor->GetMode() == mitk::Interactor::SMSELECTED,"Testing right Mode and thus right action behavior")
  
  //statemachine should have called transition "initmove", executed action with he id 9 and currentState should be 1 = "move"
  delete event;

  //send next event to stay in State1
  //<!-- middle MouseButton and MouseMove -->
  //<event NAME="middleBN+MouseMove" ID="533" TYPE="Type_MouseMove" BUTTON="BS_NoButton" BUTTONSTATE="0x0004" KEY="Key_none" />
  event = new mitk::Event(NULL, mitk::Type_MouseMove, mitk::BS_NoButton, mitk::BS_MidButton, mitk::Key_none);
  stateEvent->Set(533, event);
  MITK_TEST_CONDITION_REQUIRED(interactor->HandleEvent(stateEvent),"Staying in State 1")

  MITK_TEST_CONDITION_REQUIRED(interactor->GetMode() == mitk::Interactor::SMSELECTED,"Testing for the same Mode")
  delete event;

  //create a new event
  //<!-- MiddleMouseButtonRelease -->
  //<event NAME="MiddleMouseRelease" ID="506" TYPE="Type_MouseButtonRelease" BUTTON="BS_MidButton" BUTTONSTATE="0x0004" KEY="Key_none" />
  event = new mitk::Event(NULL, mitk::Type_MouseButtonRelease, mitk::BS_MidButton, mitk::BS_MidButton, mitk::Key_none);
  stateEvent->Set(506, event);
  //because the statemachine is waiting for this event, 0.5 should be returned
  float returnvalue = interactor->CalculateJurisdiction(stateEvent);
  MITK_TEST_CONDITION_REQUIRED(returnvalue == 0.5,"Testing to call CalculateJurisdiction with next event")
  std::cout<<"CalculateJurisdiction returned: "<<returnvalue<<std::endl;

  //The transition "finish" should call no action an lead into state 0 = startState 
  MITK_TEST_CONDITION_REQUIRED(interactor->HandleEvent(stateEvent),"Testing to send next event leading back to startState0")

  MITK_TEST_CONDITION_REQUIRED(interactor->GetMode() == mitk::Interactor::SMDESELECTED,"Testing right Mode and thus right action behavior")
  delete event;

  delete stateEvent;

  MITK_TEST_END()
}
