/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-08-09 09:26:25 +0200 (Sa, 09 Aug 2008) $
Version:   $Revision: 14937 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include <mitkEvent.h>
#include <mitkInteractionConst.h>
#include <mitkStateEvent.h>

#include <mitkStateMachine.h>
#include <mitkTestingMacros.h>

/**
 *  Simple example for a test for the (non-existent) class "StateMachine".
 *  
 *  argc and argv are the command line parameters which were passed to 
 *  the ADD_TEST command in the CMakeLists.txt file. For the automatic
 *  tests, argv is either empty for the simple tests or contains the filename
 *  of a test image for the image tests (see CMakeLists.txt).
 */
int mitkStateMachineTest(int /* argc */, char* /*argv*/[])
{
  // always start with this!
  MITK_TEST_BEGIN("StateMachine")

  std::string name("global"); //respectively "global"
  // let's create an object of our class  
  mitk::StateMachine::Pointer myStateMachine = mitk::StateMachine::New((const char *)(&name[0]));
  
  // first test: did this work?
  // using MITK_TEST_CONDITION_REQUIRED makes the test stop after failure, since
  // it makes no sense to continue without an object.
  MITK_TEST_CONDITION_REQUIRED(myStateMachine.IsNotNull(),"Testing instantiation")
  
  MITK_TEST_CONDITION_REQUIRED(myStateMachine->GetType() == name ,"Testing GetType") 

  //severe HandleEvent testing
  //creating an event to go from state 1 to state 2; don't execute an action
  mitk::Event *event = new mitk::Event(NULL, mitk::Type_None, mitk::Type_None, mitk::BS_NoButton, mitk::Key_none);
  mitk::StateEvent *stateEvent = new mitk::StateEvent(1013, event);
  MITK_TEST_CONDITION_REQUIRED(myStateMachine->HandleEvent(stateEvent) ,"Testing HandleEvent by statechange") 

  //go from state 2 to state 1; don't execute an action
  mitk::StateEvent *nextStateEvent = new mitk::StateEvent(1002, event);
  MITK_TEST_CONDITION_REQUIRED(myStateMachine->HandleEvent(nextStateEvent) ,"Testing HandleEvent returning to startstate") 

  myStateMachine = NULL;
  MITK_TEST_CONDITION_REQUIRED(myStateMachine.IsNull(),"Testing setting pointer to NULL")

  // Clean up...
  delete event;
  delete stateEvent;
  delete nextStateEvent;

  // always end with this!
  MITK_TEST_END()
}
