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

#include "mitkStateMachineAction.h"
#include "mitkStateMachineContainer.h"
#include "mitkStateMachineState.h"
#include "mitkStateMachineTransition.h"
#include "mitkTestingMacros.h"
#include <string>

int mitkStateMachineContainerTest(int /*argc*/, char * /*argv*/ [])
{
  MITK_TEST_BEGIN("StateMachineContainer")

  /*
   * Loads a test StateMachine and checks if it is build up correctly.
   */

  mitk::StateMachineContainer *smc = mitk::StateMachineContainer::New();

  MITK_TEST_CONDITION_REQUIRED(smc->LoadBehavior("Tests/StatemachineTest.xml") == true,
                               "01 Check if file can be loaded");

  std::string sname = smc->GetStartState()->GetName();

  MITK_TEST_CONDITION_REQUIRED(sname == "STATE1", "02 Check if start state is correct");

  /*
   * Follow transitions and check if all actions have been loaded.
   */
  mitk::StateMachineTransition::Pointer s1 =
    smc->GetStartState()->GetTransition("MouseMoveEvent", "no1"); // transition exists
  mitk::StateMachineTransition::Pointer s2 =
    smc->GetStartState()->GetTransition("StdMouseNotMove", "no1"); // transition does not exist
  mitk::StateMachineState::Pointer st2 = s1->GetNextState();
  mitk::StateMachineTransition::Pointer s3 =
    st2->GetTransition("MouseMoveEvent", "no1"); // transition from state2 to state3
  typedef std::vector<mitk::StateMachineAction::Pointer> ActionCollectionType;
  ActionCollectionType actions = s3->GetActions();

  ActionCollectionType::iterator it = actions.begin();
  std::string action1 = (*it)->GetActionName();
  ++it;
  std::string action2 = (*it)->GetActionName();
  ++it; // this should now point to the end of the vector, since only two actions are given

  MITK_INFO << s2.IsNull();

  MITK_TEST_CONDITION_REQUIRED(
    s1.IsNotNull() && s2.IsNull() && st2->GetName() == "STATE2" && action1 == "doaction7" && action2 == "doaction2" &&
      (it == actions.end()),
    "03 Check if transitions and Action identifier work. \n Expected STATE2 , doaction7, doction2 got:\n"
      << st2->GetName()
      << ","
      << action1
      << ","
      << action2);

  // always end with this!
  MITK_TEST_END()
}
