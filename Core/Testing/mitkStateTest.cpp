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
#include "mitkReferenceCountWatcher.h"
#include <fstream>

int mitkStateTest(int /*argc*/, char* /*argv*/[])
{
  int stateId = 10;
  
  //Create State
  mitk::State::Pointer state = mitk::State::New("state", stateId);

  //check Id
  std::cout << "check StateId: ";
  if (state->GetId()!=stateId)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  int count = 0;
  //Create Transition
  std::string firstTName = "firstTransition";
  std::string secondTName = "secondTransition";
  mitk::Transition::Pointer firstTransition = mitk::Transition::New(firstTName, count, count+1);
  ++count;
  mitk::Transition::Pointer secondTransition = mitk::Transition::New(secondTName, count, count+1);

  mitk::ReferenceCountWatcher::Pointer stateWatcher = new mitk::ReferenceCountWatcher(state, "state");
  mitk::ReferenceCountWatcher::Pointer firstTransitionWatcher = new mitk::ReferenceCountWatcher(firstTransition, "firstTransition");
  mitk::ReferenceCountWatcher::Pointer secondTransitionWatcher = new mitk::ReferenceCountWatcher(secondTransition, "secondTransition");


  //check reference counting
  std::cout<<"Check Reference Count of Transitions: ";
  if (firstTransition->GetReferenceCount() != 1 
    || secondTransition->GetReferenceCount() != 1)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;


  //add transitions
  std::cout << "Add two transitions: ";
  if (	!state->AddTransition( firstTransition.GetPointer() ) || !state->AddTransition( secondTransition.GetPointer() ))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  //check reference count again
  std::cout<<"Check Reference Count of Transitions: ";
  if (firstTransition->GetReferenceCount() != 2 
    || secondTransition->GetReferenceCount() != 2)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  //Decreasing the reference count of the two transitions
  std::cout << "Deleting transitions and counting Reference Count: ";
  firstTransition = NULL;
  secondTransition = NULL;
  ;
  if (firstTransitionWatcher->GetReferenceCount() != 1 ||
    secondTransitionWatcher->GetReferenceCount() != 1)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;
   

  count = 1;
  //check getting the transitions
  std::cout << "Try reading the first transition: ";
  const mitk::Transition* tempTransition = state->GetTransition(count);
  if (tempTransition->GetName() != firstTName)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  ++count;

  std::cout << "Try reading the second transition: ";
  tempTransition = state->GetTransition(count);
  if (tempTransition->GetName() != secondTName)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  //check valid EventIds
  std::cout << "Check if the first EventId is valid: ";
  count = 1;
  if (!state->IsValidEvent(count))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;
  ++count;
  std::cout << "Check if the second EventId is valid: ";
  if (!state->IsValidEvent(count))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;
  ++count;
  std::cout << "Check if a non existent EventId is valid: ";
  if (state->IsValidEvent(count))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  //deleting state and checking if transitions are deleted
  state = NULL;
  std::cout << "Delete state and check if state and transitions are deleted: ";
  if (stateWatcher->GetReferenceCount() != 0 ||
    firstTransitionWatcher->GetReferenceCount() != 0 ||
    secondTransitionWatcher->GetReferenceCount() != 0 )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;


  //well done!!! Passed!
  std::cout<<"[All PASSED]"<<std::endl;

  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
