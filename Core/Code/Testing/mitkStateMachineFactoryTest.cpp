/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#include <mitkStateMachineFactory.h>
#include <mitkState.h>
#include "mitkTestingMacros.h"
#include <mitkInteractionConst.h>
int mitkStateMachineFactoryTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("StateMachineFactory")
  //create statemachinefactory
  mitk::StateMachineFactory* statemachineFactory = mitk::StateMachineFactory::New();

  //load standard behavior
  MITK_TEST_CONDITION_REQUIRED(statemachineFactory->LoadStandardBehavior(),"Testing LoadStandardBehavior(): ")

  //get the first state of the statemachine "global" (mitkGlobalInteraction)
  mitk::State::Pointer state = statemachineFactory->GetStartState("global");
  MITK_TEST_CONDITION_REQUIRED(state.IsNotNull(),"Testing GetStartState() of GlobalInteraction state machine pattern: ")

  MITK_TEST_CONDITION_REQUIRED(argc>2, "Testing correct test invocation");
  std::string xmlFileName1 = argv[1];
  MITK_TEST_CONDITION_REQUIRED(!xmlFileName1.empty(),"Getting xml file 1: ")
  MITK_TEST_CONDITION_REQUIRED(statemachineFactory->LoadBehavior(xmlFileName1),"Parsing xml file 1: ")
  state = statemachineFactory->GetStartState("test1");
  MITK_TEST_CONDITION_REQUIRED(state.IsNotNull(),"Testing GetStartState() of test1 state machine pattern: ")

  //global still accessible?
  state = statemachineFactory->GetStartState("global");
  MITK_TEST_CONDITION_REQUIRED(state.IsNotNull(),"Testing if previous loaded state machine patterns are still accessible: ")

  std::string xmlFileName2 = argv[2];
  MITK_TEST_CONDITION_REQUIRED(!xmlFileName2.empty(),"Getting xml file 2: ")
  MITK_TEST_CONDITION_REQUIRED(statemachineFactory->LoadBehavior(xmlFileName2),"Parsing xml file 2. Schould throw a fatal error due to already existing pattern name: ")
  state = statemachineFactory->GetStartState("test4");
  MITK_TEST_CONDITION_REQUIRED(state.IsNotNull(),"Testing GetStartState() of test4 state machine pattern: ")

  state = statemachineFactory->GetStartState("global");
  MITK_TEST_CONDITION_REQUIRED(state.IsNotNull(),"Testing if previous loaded state machine pattern (global) is still accessible: ")
  state = statemachineFactory->GetStartState("test1");
  MITK_TEST_CONDITION_REQUIRED(state.IsNotNull(),"Testing if previous loaded state machine pattern (test1) is still accessible: ")


  //manually create a state machine pattern and add it to factory
  std::string patternName("manuallyCreatedStateMachine");
  mitk::StateMachineFactory::StateMachineMapType* allStatesMap = new mitk::StateMachineFactory::StateMachineMapType();

  mitk::State::Pointer state1 = mitk::State::New("firstState", 1);
  mitk::Transition* transition1 = new mitk::Transition("goto2", 2, mitk::EIDNULLEVENT);
  mitk::Action::Pointer action1 = mitk::Action::New(mitk::AcDONOTHING);
  transition1->AddAction(action1);
  state1->AddTransition(transition1);
  allStatesMap->insert(mitk::StateMachineFactory::StateMachineMapType::value_type(state1->GetId(), state1));

  mitk::State::Pointer state2 = mitk::State::New("secondState", 2);
  transition1->SetNextState(state2);
  mitk::Transition* transition2 = new mitk::Transition("goto1", 1, mitk::EIDNULLEVENT);
  mitk::Action::Pointer action2 = mitk::Action::New(mitk::AcDONOTHING);
  transition2->AddAction(action2);
  transition2->SetNextState(state1);
  state2->AddTransition(transition2);
  allStatesMap->insert(mitk::StateMachineFactory::StateMachineMapType::value_type(state2->GetId(), state2));

  //now add to factory
  statemachineFactory->AddStateMachinePattern(patternName.c_str(), state1, allStatesMap);

  //check if it is accessable
  state = statemachineFactory->GetStartState("global");
  MITK_TEST_CONDITION_REQUIRED(state.IsNotNull(),"Testing if previous loaded state machine pattern (global) is still accessible: ")
  state = statemachineFactory->GetStartState("test1");
  MITK_TEST_CONDITION_REQUIRED(state.IsNotNull(),"Testing if previous loaded state machine pattern (test1) is still accessible: ")
  state = statemachineFactory->GetStartState(patternName.c_str());
  MITK_TEST_CONDITION_REQUIRED(state.IsNotNull(),"Testing if manually created and added state machine pattern is accessible: ")

  statemachineFactory->Delete();
  //states, transitions and actions are freed in StateMachineFactory

  // always end with this!
  MITK_TEST_END();
}
