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


#include <mitkStateMachineFactory.h>
#include <mitkState.h>
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

#include <fstream>
int mitkStateMachineFactoryTest(int /*argc*/, char* /*argv*/[])
{
  //create statemachinefactory
  mitk::StateMachineFactory* statemachineFactory = mitk::StateMachineFactory::New();
  
  //load standard behavior 
  std::cout << "Testing LoadStandardBehavior(): ";
  if (!statemachineFactory->LoadStandardBehavior())
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
