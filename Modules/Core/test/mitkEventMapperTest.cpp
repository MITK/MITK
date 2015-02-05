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


#include <mitkEventMapper.h>
#include <mitkEvent.h>
#include <mitkInteractionConst.h>
#include "mitkTestingMacros.h"
#include <mitkStateEvent.h>
int mitkEventMapperTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("EventMapper");

  MITK_TEST_CONDITION_REQUIRED(argc >= 3, "Test if a file to load has been specified");

  //construct IDs to be checked
  mitk::Event* mouseButtonPressEvent = new mitk::Event(NULL, mitk::Type_MouseButtonPress, mitk::BS_LeftButton, mitk::BS_NoButton, mitk::Key_none);
  const int mouseButtonPressID = 1;
  //there is no combination Type_MouseButtonPress and Keys, so use this to be sure to have unique events
  mitk::Event* uniqueEventFile1 = new mitk::Event(NULL, mitk::Type_MouseButtonPress, mitk::BS_NoButton, mitk::BS_NoButton, mitk::Key_R);
  const int uniqueEventIDFile1 = 13000;
  mitk::Event* uniqueEventFile2 = new mitk::Event(NULL, mitk::Type_MouseButtonPress, mitk::BS_NoButton, mitk::BS_NoButton, mitk::Key_N);
  const int uniqueEventIDFile2 = 13001;


  //create statemachinefactory
  mitk::EventMapper* eventMapper = mitk::EventMapper::New();

  //load standard behavior
  MITK_TEST_CONDITION_REQUIRED(eventMapper->LoadStandardBehavior(),"Testing LoadStandardBehavior(): ")
  std::cout<<"StyleName: " << eventMapper->GetStyleName()<<"\n";

  mitk::StateEvent stateEvent;
  stateEvent.Set(0, mouseButtonPressEvent);
  eventMapper->RefreshStateEvent(&stateEvent);
  MITK_TEST_CONDITION_REQUIRED(stateEvent.GetId() == mouseButtonPressID,"Testing event mapping of standard xml-file: ")

//  std::string xmlFileName1( "TestStateMachine1.xml" );
  std::string xmlFileName1( argv[1] );
  MITK_TEST_CONDITION_REQUIRED(!xmlFileName1.empty(),"Getting xml file 1: ")
  MITK_TEST_CONDITION_REQUIRED(eventMapper->LoadBehavior(xmlFileName1),"Parsing xml file 1 should throw warning: ")
  //test dubicate file loading
  MITK_TEST_CONDITION_REQUIRED(eventMapper->LoadBehavior(xmlFileName1) == true,"Double parsing should be avoided and not throw warnings.")

  stateEvent.Set(0, uniqueEventFile1);
  eventMapper->RefreshStateEvent(&stateEvent);
  MITK_TEST_CONDITION_REQUIRED(stateEvent.GetId() == uniqueEventIDFile1,"Testing event mapping of first additionally loaded xml-file: ")

  //global still accessible?
  stateEvent.Set(0, mouseButtonPressEvent);
  eventMapper->RefreshStateEvent(&stateEvent);
  MITK_TEST_CONDITION_REQUIRED(stateEvent.GetId() == mouseButtonPressID,"Testing if standard information still available: ")


//  std::string xmlFileName2( "TestStateMachine2.xml" );
  std::string xmlFileName2( argv[2] );
  MITK_TEST_CONDITION_REQUIRED(!xmlFileName2.empty(),"Getting xml file 2: ")
  MITK_TEST_CONDITION_REQUIRED(eventMapper->LoadBehavior(xmlFileName2),"Parsing xml file 2. Warning of double entry should be thrown: ")

  stateEvent.Set(0, uniqueEventFile2);
  eventMapper->RefreshStateEvent(&stateEvent);
  MITK_TEST_CONDITION_REQUIRED(stateEvent.GetId() == uniqueEventIDFile2,"Testing event mapping of second additionally loaded xml-file: ")

  //global still accessible?
  stateEvent.Set(0, mouseButtonPressEvent);
  eventMapper->RefreshStateEvent(&stateEvent);
  MITK_TEST_CONDITION_REQUIRED(stateEvent.GetId() == mouseButtonPressID,"Testing if standard information still available: ")

  eventMapper->Delete();

  // always end with this!
  MITK_TEST_END();
}
