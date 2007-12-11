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


#include <fstream>
int mitkInteractorTest(int /*argc*/, char* /*argv*/[])
{
  //create interactor; use pattern InteractorTestPattern
  mitk::Interactor::Pointer interactor = mitk::Interactor::New("InteractorTestPattern", NULL);

  //load interactor
  std::cout << "Testing to initialize the interactor: ";
  if (interactor.IsNull())
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;
  
  std::cout<<"The pattern of the interactor is called: "<<interactor->GetType()<<std::endl;

  std::cout << "Testing to send an event to go from State 0 to State 1: ";
  //creating an event to go from state 0 to state 1:
  //<!-- middle MouseButton -->
  //<event NAME="middle MouseBN" ID="4" TYPE="Type_MouseButtonPress" BUTTON="BS_MidButton" BUTTONSTATE="0x0000" KEY="Key_none" />
  mitk::Event *event = new mitk::Event(NULL, mitk::Type_MouseButtonPress, mitk::BS_MidButton, mitk::BS_NoButton, mitk::Key_none);
  mitk::StateEvent *stateEvent = new mitk::StateEvent(4, event);
  if (!interactor->HandleEvent(stateEvent))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing right Mode and thus right action behavior: ";
  if (interactor->GetMode() != mitk::Interactor::SMSELECTED)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;
  
  //statemachine should have called transition "initmove", executed action with he id 9 and currentState should be 1 = "move"
  delete event;

  //send next event to stay in State1
  //<!-- middle MouseButton and MouseMove -->
  //<event NAME="middleBN+MouseMove" ID="533" TYPE="Type_MouseMove" BUTTON="BS_NoButton" BUTTONSTATE="0x0004" KEY="Key_none" />
  event = new mitk::Event(NULL, mitk::Type_MouseMove, mitk::BS_NoButton, mitk::BS_MidButton, mitk::Key_none);
  stateEvent->Set(533, event);
  std::cout << "Staying in State 1: ";
  if (!interactor->HandleEvent(stateEvent))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing for the same Mode: ";
  if (interactor->GetMode() != mitk::Interactor::SMSELECTED)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;
  delete event;

  //create a new event
  //<!-- MiddleMouseButtonRelease -->
  //<event NAME="MiddleMouseRelease" ID="506" TYPE="Type_MouseButtonRelease" BUTTON="BS_MidButton" BUTTONSTATE="0x0004" KEY="Key_none" />
  event = new mitk::Event(NULL, mitk::Type_MouseButtonRelease, mitk::BS_MidButton, mitk::BS_MidButton, mitk::Key_none);
  stateEvent->Set(506, event);
  std::cout << "Testing to call CalculateJurisdiction with next event: ";
  //because the statemachine is waiting for this event, 0.5 should be returned
  float returnvalue = interactor->CalculateJurisdiction(stateEvent);
  if ( returnvalue != 0.5)
  {
    std::cout<<"[FAILED]"<<std::endl;
    std::cout<<"CalculateJurisdiction returned: "<<returnvalue<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;
  std::cout<<"CalculateJurisdiction returned: "<<returnvalue<<std::endl;

  std::cout << "Testing to send next event leading back to startState0: ";
  //The transition "finish" should call no action an lead into state 0 = startState 
  if (!interactor->HandleEvent(stateEvent))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing right Mode and thus right action behavior: ";
  if (interactor->GetMode() != mitk::Interactor::SMDESELECTED)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;
  delete event;

  delete stateEvent;

  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
