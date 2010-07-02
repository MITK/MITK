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


#include <mitkGlobalInteraction.h>
#include <mitkPointSetInteractor.h>

#include "mitkTestingMacros.h"

#include <fstream>
int mitkGlobalInteractionTest(int /*argc*/, char* /*argv*/[])
{
  MITK_TEST_BEGIN("GlobalInteraction")
 
  //get static instance of globalInteraction
  mitk::GlobalInteraction::Pointer globalInteraction = mitk::GlobalInteraction::GetInstance();
  MITK_TEST_CONDITION_REQUIRED(globalInteraction.IsNotNull(),"testing singleton initialization: ") 
  MITK_TEST_CONDITION_REQUIRED(globalInteraction->IsInitialized() == false ,"testing initial initialization: not initialized") 

  // Initialize with default values
  MITK_TEST_CONDITION_REQUIRED(globalInteraction->Initialize("global"),"testing to initialize: ")

  //testing initialization
  MITK_TEST_CONDITION_REQUIRED(globalInteraction->IsInitialized(),"testing initial initialization: initialized") 
  
  MITK_TEST_CONDITION_REQUIRED(globalInteraction->Initialize("global") == false ,"testing double initialization") 

  MITK_TEST_CONDITION_REQUIRED(globalInteraction->IsInitialized(),"still initialized: ") 

  MITK_TEST_CONDITION_REQUIRED(globalInteraction.IsNotNull() ,"Testing 'instantiation' of 'global' static GlobalInteraction") 

  //testing creating non singleton instance
  mitk::GlobalInteraction::Pointer myGlobalInteraction = mitk::GlobalInteraction::New();
  MITK_TEST_CONDITION_REQUIRED(myGlobalInteraction.IsNotNull(),"testing non singleton initialization: ") 
  MITK_TEST_CONDITION_REQUIRED(myGlobalInteraction->IsInitialized() == false ,"testing initial initialization of non singleton: not initialized") 
  MITK_TEST_CONDITION_REQUIRED(myGlobalInteraction->Initialize("global"),"testing to initialize non singleton: ")
  MITK_TEST_CONDITION_REQUIRED(myGlobalInteraction->IsInitialized() ,"testing initialization of non singleton: initialized") 

  //testing if it really is a different instance than singleton
  MITK_TEST_CONDITION_REQUIRED(myGlobalInteraction != globalInteraction ,"Testing whether new instance equals the global satic one (must not be!)") 
  
  //getting singleton over non singleton instance: bug or feature???
  MITK_TEST_CONDITION_REQUIRED(myGlobalInteraction->GetInstance() == globalInteraction ,"Testing singleton from non singleton instance") 

  //destroy non singleton class
  myGlobalInteraction = NULL;
  
  //now test adding and removing of Interactors and Listeners
  //create Interactors
  mitk::PointSetInteractor::Pointer firstInteractor = mitk::PointSetInteractor::New("pointsetinteractor", NULL, 1);
  mitk::PointSetInteractor::Pointer secondInteractor = mitk::PointSetInteractor::New("pointsetinteractor", NULL, 10);

  globalInteraction->AddInteractor(firstInteractor);
  MITK_TEST_CONDITION_REQUIRED(globalInteraction->InteractorRegistered(firstInteractor),"Add first interactor to globalInteraction and check if is is registered: ") 
  globalInteraction->AddInteractor(secondInteractor);
  MITK_TEST_CONDITION_REQUIRED(globalInteraction->InteractorRegistered(secondInteractor),"Add second interactor to globalInteraction and check if is is registered: ") 

  //remove Interactor
  MITK_TEST_CONDITION_REQUIRED(globalInteraction->RemoveInteractor(firstInteractor),"Remove the first Interactor: ") 
  MITK_TEST_CONDITION_REQUIRED(!globalInteraction->RemoveInteractor(firstInteractor),"Remove the first Interactor a second time: ") 

  //check if really removed
  MITK_TEST_CONDITION_REQUIRED(!globalInteraction->InteractorRegistered(firstInteractor),"Check if the first is not registered: ") 
  //check if second still present
  MITK_TEST_CONDITION_REQUIRED(globalInteraction->InteractorRegistered(secondInteractor),"Check if the second is still registered: ") 
  
  //remove the second too
  MITK_TEST_CONDITION_REQUIRED(globalInteraction->RemoveInteractor(secondInteractor),"Remove the second Interactor: ") 
  MITK_TEST_CONDITION_REQUIRED(!globalInteraction->RemoveInteractor(secondInteractor),"Remove the second Interactor a second time: ") 
  MITK_TEST_CONDITION_REQUIRED(!globalInteraction->InteractorRegistered(secondInteractor),"Check if the second is not registered: ") 
  

  //------------------
  //now check the same with Listener
  std::cout << "Check the addition of a Listener the same way: ";
  std::cout << "Add two interactors as listeners (usually you add statemachines, not interactors). Are they both registered?: ";
  std::cout << "Add listener1 (ITK popup with warning should come up: ";
  globalInteraction->AddListener(firstInteractor);
  MITK_TEST_CONDITION_REQUIRED(globalInteraction->ListenerRegistered(firstInteractor),"registered listener1: ") 
  MITK_TEST_CONDITION_REQUIRED(!globalInteraction->ListenerRegistered(secondInteractor),"not yet registered listener2: ") 
  std::cout << "Add listener2: ";
  globalInteraction->AddListener(secondInteractor);
  MITK_TEST_CONDITION_REQUIRED(globalInteraction->ListenerRegistered(secondInteractor),"registered listener2: ") 
  MITK_TEST_CONDITION_REQUIRED(globalInteraction->ListenerRegistered(firstInteractor),"listener1 still registered: ") 
  
  //remove Listener
  MITK_TEST_CONDITION_REQUIRED(globalInteraction->RemoveListener(secondInteractor),"Remove listener2: ") 
  MITK_TEST_CONDITION_REQUIRED(!globalInteraction->ListenerRegistered(secondInteractor),"listener2 not registered anymore: ") 
  MITK_TEST_CONDITION_REQUIRED(globalInteraction->ListenerRegistered(firstInteractor),"but listener1: ") 
  //remove first too
  MITK_TEST_CONDITION_REQUIRED(globalInteraction->RemoveListener(firstInteractor),"Remove listener1: ") 
  MITK_TEST_CONDITION_REQUIRED(!globalInteraction->ListenerRegistered(firstInteractor),"listener1 not registered anymore: ") 
  MITK_TEST_CONDITION_REQUIRED(!globalInteraction->ListenerRegistered(secondInteractor),"listener2 also not registered: ") 

  
  //now add them as interactors again
  std::cout << "Now add the two interactors as interactors again: ";
  globalInteraction->AddInteractor(firstInteractor);
  globalInteraction->AddInteractor(secondInteractor);
  
  //releasing smartpointer of interactors; should be kept in GlobalInteraction
  firstInteractor = NULL;
  secondInteractor = NULL;
  globalInteraction = NULL;

  // always end with this!
  MITK_TEST_END();
}
