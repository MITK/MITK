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


#include <mitkStateMachineFactory.h>
#include <mitkState.h>

#include <fstream>
int mitkStateMachineFactoryTest(int /*argc*/, char* /*argv*/[])
{
  //create statemachinefactory
  mitk::StateMachineFactory* statemachineFactory = new mitk::StateMachineFactory();
  
  //load standard behavior 
  std::cout << "Testing LoadStandardBehavior(): ";
  if (!mitk::StateMachineFactory::LoadStandardBehavior())
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;
  
  //get the first state of the statemachine "global" (mitkGlobalInteraction)
  mitk::State::Pointer state = statemachineFactory->GetStartState("global");
  std::cout << "Testing GetStartState() of GlobalInteraction StateMachinePattern: ";
  if (state.IsNull())
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  statemachineFactory->Delete();

  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
