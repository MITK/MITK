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

#include <fstream>
int mitkGlobalInteractionTest(int /*argc*/, char* /*argv*/[])
{
  //get static instance of globalInteraction
  mitk::GlobalInteraction::Pointer globalInteraction = mitk::GlobalInteraction::GetInstance();

  //create Interactors
  mitk::PointSetInteractor::Pointer firstInteractor = mitk::PointSetInteractor::New("pointsetinteractor", NULL, 1);
  mitk::PointSetInteractor::Pointer secondInteractor = mitk::PointSetInteractor::New("pointsetinteractor", NULL, 10);

  globalInteraction->AddInteractor(firstInteractor);
  globalInteraction->AddInteractor(secondInteractor);
  std::cout << "Add two interactors to globalInteraction and check if they were registered: ";
  if ( !globalInteraction->InteractorRegistered(firstInteractor) ||
       !globalInteraction->InteractorRegistered(secondInteractor) )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  //remove Interactor
  std::cout << "Remove the first Interactor: ";
  if ( !globalInteraction->RemoveInteractor(firstInteractor) )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  //check if really removed
  std::cout << "Check if the first is still registered: ";
  if ( globalInteraction->InteractorRegistered(firstInteractor) )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  //still registered
  std::cout << "Check if the second is still registered: ";
  if ( !globalInteraction->InteractorRegistered(secondInteractor) )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  //remove second too
  std::cout << "Remove the second, too: ";
  if ( !globalInteraction->RemoveInteractor(secondInteractor) )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  
  //check if empty
  std::cout << "Check one of the two Interactors is registered: ";
  if ( globalInteraction->InteractorRegistered(firstInteractor) ||
       globalInteraction->InteractorRegistered(secondInteractor) )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  //------------------
  //now check the same with Listener
  std::cout << "Check the addition of a Listener the same way: ";
  std::cout << "Add two listeners. Are they both registered?: ";
  globalInteraction->AddListener(firstInteractor);
  globalInteraction->AddListener(secondInteractor);
  if ( !globalInteraction->ListenerRegistered(firstInteractor) ||
       !globalInteraction->ListenerRegistered(secondInteractor) )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  //remove Listener
  std::cout << "Remove second L.: ";
  if ( !globalInteraction->RemoveListener(secondInteractor) )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  //check if really removed
  std::cout << "Check if second is still registered: ";
  if ( globalInteraction->ListenerRegistered(secondInteractor) )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  //still registered
  std::cout << "Check if the first is still registered: ";
  if ( !globalInteraction->ListenerRegistered(firstInteractor) )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  //remove first too
  std::cout << "Remove the first Listener, too: ";
  if ( !globalInteraction->RemoveListener(firstInteractor) )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;
  
  //check if empty
  std::cout << "Is one of them still registered?: ";
  if ( globalInteraction->ListenerRegistered(firstInteractor) ||
       globalInteraction->ListenerRegistered(secondInteractor) )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Now add the two interactors as interactors again and end: ";
  globalInteraction->AddInteractor(firstInteractor);
  globalInteraction->AddInteractor(secondInteractor);
  
  //well done!!! Passed!
  std::cout<<"[ALL PASSED]"<<std::endl;

  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
