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


#include <mitkTransition.h>
#include <mitkAction.h>
#include <mitkState.h>

#include <fstream>
int mitkTransitionTest(int /*argc*/, char* /*argv*/[])
{
  int nextStateId = 10;
  int eventId = 100;
  
  //Create Transition
  itk::WeakPointer<mitk::Transition> transition = new mitk::Transition("transitionname", nextStateId, eventId);

  //check nextStateId
  std::cout << "Check StateId: ";
  if (transition->GetNextStateId()!=nextStateId)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  //check EventId
  std::cout << "Check EventId: ";
  if (transition->GetEventId()!=eventId || !transition->IsEvent(eventId))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  unsigned int count = 1;
  //Create Action
  mitk::Action::Pointer firstAction = mitk::Action::New(count);
  transition->AddAction(firstAction);

  //check ActionCount
  std::cout << "Check ActionCount after first addition of an action: ";
  if (transition->GetActionCount() != count)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;
  
  ++count;

  //Create Action
  mitk::Action::Pointer secondAction = mitk::Action::New(count);
  transition->AddAction(secondAction);

  //check ActionCount
  std::cout << "check action Count after second addition of an action: ";
  if (transition->GetActionCount() != count)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  count = 1;
  
  //check ActionIterators
  std::cout << "Check ActionIterators: ";
  mitk::Transition::ActionVectorIterator iter = transition->GetActionBeginIterator();
  mitk::Transition::ActionVectorConstIterator end = transition->GetActionEndIterator();
  while (iter != end)
  {
    if((*iter)->GetActionId()!=(int)count)
    {
      std::cout<<"[FAILED]"<<std::endl;
      return EXIT_FAILURE;
    }
    ++count;
    ++iter;
  }
  std::cout<<"[PASSED]"<<std::endl;

  //check setting State
  std::cout << "Check setting a statepointer and reading it again: ";
  int stateId = 10000;
  mitk::State::Pointer state = mitk::State::New("statename", stateId);
  transition->SetNextState(state);
  if (transition->GetNextState()!=state)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  //delete the instanciated transition
  delete transition;

  //well done!!! Passed!
  std::cout<<"[ALL PASSED]"<<std::endl;

  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
