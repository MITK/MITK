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
#include "mitkTestingMacros.h"
#include <mitkStandardFileLocations.h>
int mitkStateMachineFactoryTest(int /*argc*/, char* /*argv*/[])
{
  MITK_TEST_BEGIN("StateMachineFactory")
  //create statemachinefactory
  mitk::StateMachineFactory* statemachineFactory = mitk::StateMachineFactory::New();
  
  //load standard behavior 
  MITK_TEST_CONDITION_REQUIRED(statemachineFactory->LoadStandardBehavior(),"Testing LoadStandardBehavior(): ") 
  
  //get the first state of the statemachine "global" (mitkGlobalInteraction)
  mitk::State::Pointer state = statemachineFactory->GetStartState("global");
  MITK_TEST_CONDITION_REQUIRED(state.IsNotNull(),"Testing GetStartState() of GlobalInteraction state machine pattern: ") 

  std::string xmlFileName1( mitk::StandardFileLocations::GetInstance()->FindFile("TestStateMachine1.xml", "Core/Code/Testing/Data") );
  MITK_TEST_CONDITION_REQUIRED(!xmlFileName1.empty(),"Getting xml file 1: ") 
  MITK_TEST_CONDITION_REQUIRED(statemachineFactory->LoadBehavior(xmlFileName1),"Parsing xml file 1: ") 
  state = statemachineFactory->GetStartState("test1");
  MITK_TEST_CONDITION_REQUIRED(state.IsNotNull(),"Testing GetStartState() of test1 state machine pattern: ") 

  //global still accessible?
  state = statemachineFactory->GetStartState("global");
  MITK_TEST_CONDITION_REQUIRED(state.IsNotNull(),"Testing if previous loaded state machine patterns are still accessible: ") 
  
  std::string xmlFileName2( mitk::StandardFileLocations::GetInstance()->FindFile("TestStateMachine2.xml", "Core/Code/Testing/Data") );
  MITK_TEST_CONDITION_REQUIRED(!xmlFileName2.empty(),"Getting xml file 2: ") 
  MITK_TEST_CONDITION_REQUIRED(statemachineFactory->LoadBehavior(xmlFileName2),"Parsing xml file 2. Schould throw a fatal error due to already existing pattern name: ") 
  state = statemachineFactory->GetStartState("test4");
  MITK_TEST_CONDITION_REQUIRED(state.IsNotNull(),"Testing GetStartState() of test4 state machine pattern: ")
  
  state = statemachineFactory->GetStartState("global");
  MITK_TEST_CONDITION_REQUIRED(state.IsNotNull(),"Testing if previous loaded state machine pattern (global) is still accessible: ") 
  state = statemachineFactory->GetStartState("test1");
  MITK_TEST_CONDITION_REQUIRED(state.IsNotNull(),"Testing if previous loaded state machine pattern (test1) is still accessible: ") 

  statemachineFactory->Delete();

  // always end with this!
  MITK_TEST_END();
}
