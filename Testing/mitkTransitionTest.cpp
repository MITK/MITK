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


#include <mitkTransition.h>
#include <mitkAction.h>
#include <mitkState.h>

#include <fstream>
int mitkTransitionTest(int argc, char* argv[])
{
  int nextStateId = 10;
  int eventId = 100;
  
  //Create Transition
  mitk::Transition* transition = new mitk::Transition("transitionname", nextStateId, eventId);

  //check nextStateId
  std::cout << "Check StateId";
  if (transition->GetNextStateId()!=nextStateId)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }

  //check EventId
  std::cout << "Check EventId";
  if (transition->GetEventId()!=eventId || !transition->IsEvent(eventId))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }

  int count = 1;
  //Create Action
  mitk::Action* firstAction = new mitk::Action(count);
  transition->AddAction(firstAction);

  //check ActionCount
  std::cout << "Check ActionCount after first addition of an action";
  if (transition->GetActionCount()!=count)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  
  ++count;

  //Create Action
  mitk::Action* secondAction = new mitk::Action(count);
  transition->AddAction(secondAction);

  //check ActionCount
  std::cout << "check action Count after second addition of an action";
  if (transition->GetActionCount()!=count)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }

  count = 1;
  
  //check ActionIterators
  std::cout << "check ActionIterators";
  std::vector<mitk::Action*>::iterator iter = transition->GetActionBeginIterator();
  const std::vector<mitk::Action*>::iterator end = transition->GetActionEndIterator();
  while (iter != end)
  {
    if((*iter)->GetActionId()!=count)
    {
      std::cout<<"[FAILED]"<<std::endl;
      return EXIT_FAILURE;
    }
    ++count;
    ++iter;
  }

  //check setting State
  std::cout << "check setting a statepointer and reading it again";
  int stateId = 10000;
  mitk::State * state = new mitk::State("statename", stateId);
  transition->SetNextState(state);
  if (transition->GetNextState()!=state)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }


  //well done!!! Passed!
  std::cout<<"[PASSED]"<<std::endl;

  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
