/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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
//#include <mitkAction.h>

#include <fstream>
int mitkStateTest(int argc, char* argv[])
{
  int stateId = 10;
  
  //Create State
  mitk::State* state = new mitk::State("state", stateId);

  //check Id
  std::cout << "check StateId";
  if (state->GetId()!=stateId)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }

  int count = 0;
  //Create Transition
  mitk::Transition* firstTransition = new mitk::Transition("firstTransition", count, count+1);
  ++count;
  mitk::Transition* secondTransition = new mitk::Transition("secondTransition", count, count+1);

  //add transitions
  std::cout << "add two transitions";
  if (	!state->AddTransition( firstTransition ) || !state->AddTransition( secondTransition ))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }

  count = 1;
  //check getting the transitions
  std::cout << "try reading the first transition";
  if (state->GetTransition(count) != firstTransition)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  ++count;
  std::cout << "try reading the second transition";
  if (state->GetTransition(count) != secondTransition)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }

  //check valid EventIds
  std::cout << "check if the first EventId is valid";
  count = 1;
  if (!state->IsValidEvent(count))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  ++count;
  std::cout << "check if the second EventId is valid";
  if (!state->IsValidEvent(count))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  ++count;
  std::cout << "check if a non existent EventId is valid";
  if (state->IsValidEvent(count))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }

  //well done!!! Passed!
  std::cout<<"[PASSED]"<<std::endl;

  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
