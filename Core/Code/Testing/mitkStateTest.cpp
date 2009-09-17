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


#include <mitkState.h>
#include <mitkTransition.h>
#include "mitkTestingMacros.h"
#include <fstream>


/**
*http://msdn.microsoft.com/en-us/library/e5ewb1h3.aspx
* VS9 memory leakage detection
**/
//#ifdef WIN32
//#ifdef _DEBUG
//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>
//#endif
//#endif


int mitkStateTest(int /*argc*/, char* /*argv*/[])
{
  int stateId = 10;
  
  //Create State
  mitk::State::Pointer state = mitk::State::New("state", stateId);
  
  //check reference count
  MITK_TEST_CONDITION_REQUIRED(state->GetReferenceCount() == 1,"Testing ReferenceCount of State");

  //check Id
  MITK_TEST_CONDITION_REQUIRED(state->GetId()==stateId,"Testing GetID ");

  int count = 0;
  //Create Transition
  std::string firstTName = "firstTransition";
  std::string secondTName = "secondTransition";
  mitk::Transition* firstTransition = new mitk::Transition(firstTName, count, count+1);
  MITK_TEST_CONDITION_REQUIRED(state->AddTransition( firstTransition ),"Adding first transition");
  MITK_TEST_CONDITION_REQUIRED(state->IsValidEvent(count+1),"Check if the first EventId is valid");
  MITK_TEST_CONDITION_REQUIRED(state->GetTransition(count+1) == firstTransition ,"Getting first transition");
  MITK_TEST_CONDITION_REQUIRED(state->GetReferenceCount() == 1,"Testing ReferenceCount still one");
  ++count;
  
  mitk::Transition* secondTransition = new mitk::Transition(secondTName, count, count+1);
  MITK_TEST_CONDITION_REQUIRED(state->AddTransition( secondTransition ),"Adding second transition");
  MITK_TEST_CONDITION_REQUIRED(state->IsValidEvent(count+1),"Check if the second EventId is valid");
  MITK_TEST_CONDITION_REQUIRED(state->GetTransition(count+1) == secondTransition ,"Getting second transition");
  MITK_TEST_CONDITION_REQUIRED(state->GetReferenceCount() == 1,"Testing ReferenceCount still one");

  ++count;
  MITK_TEST_CONDITION_REQUIRED( ! state->IsValidEvent(count+1),"Check if a non existent EventId is valid");
  
  //deleting state and checking if transitions are deleted
  state = NULL;
  MITK_TEST_CONDITION_REQUIRED(state.IsNull(),"Testing setting state to null and deleting it with it"); 

    
  std::cout << "Check state with cyclic definition: StateA->TransitionA->StateA: \n";
  stateId = 20;
  const char* name = "StateA";
  state = mitk::State::New(name, stateId);
  MITK_TEST_CONDITION_REQUIRED(state->GetId()==stateId,"Testing GetID ");
  MITK_TEST_CONDITION_REQUIRED(state->GetName()==name,"Testing GetID "); 
  MITK_TEST_CONDITION_REQUIRED(state->GetReferenceCount() == 1,"Testing ReferenceCount of State"); 
  std::cout << "Add next state to transition: ";
  count = 0;
  //creating first transition
  firstTransition = new mitk::Transition(firstTName, stateId, count+1);
  firstTransition->SetNextState(state);
  state->AddTransition(firstTransition);
  MITK_TEST_CONDITION_REQUIRED(state->GetReferenceCount() == 1,"Testing ReferenceCount still one");
  //creating second transition
  secondTransition = new mitk::Transition(secondTName, stateId, count+2);
  secondTransition ->SetNextState(state);
  state->AddTransition(secondTransition); 
  MITK_TEST_CONDITION_REQUIRED(state->GetReferenceCount() == 1,"Testing ReferenceCount still one");
  
  //destroying it again.
  state = NULL;
  MITK_TEST_CONDITION_REQUIRED(state.IsNull(),"Testing setting state to null and deleting it with it"); 

  //doesn't have to be done because the memory is freed in ~State destructor
  //delete firstTransition;
  //delete secondTransition;


  //#ifdef WIN32
  //#ifdef _DEBUG
  ////memory leakage detection
  //_CrtDumpMemoryLeaks();
  //#endif
  //#endif

  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
