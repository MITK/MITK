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


#include <mitkGlobalInteraction.h>
#include <mitkPointSetInteractor.h>

#include <fstream>
int mitkGlobalInteractionTest(int argc, char* argv[])
{
  //create Interactors
  mitk::PointSetInteractor * firstInteractor = new mitk::PointSetInteractor("pointsetinteractor", NULL, 1);
  mitk::PointSetInteractor * secondInteractor = new mitk::PointSetInteractor("pointsetinteractor", NULL, 10);
  
  //Create GlobalInteraction
  mitk::GlobalInteraction* globalInteraction = new mitk::GlobalInteraction("globalinteraction");

  globalInteraction->AddInteractor(firstInteractor);
  globalInteraction->AddInteractor(secondInteractor);
  if ( !globalInteraction->InteractorRegistered(firstInteractor) ||
       !globalInteraction->InteractorRegistered(secondInteractor) )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }

  //remove Interactor
  if ( !globalInteraction->RemoveInteractor(secondInteractor) )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }

  //check if really removed
  if ( globalInteraction->InteractorRegistered(secondInteractor) )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  
  //still registered
  if ( !globalInteraction->InteractorRegistered(firstInteractor) )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }

  //remove first too
  if ( !globalInteraction->RemoveInteractor(firstInteractor) )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  
  //check if empty
  if ( globalInteraction->InteractorRegistered(firstInteractor) ||
       globalInteraction->InteractorRegistered(secondInteractor) )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }

  //------------------
  //now check the same with Listener
  globalInteraction->AddListener(firstInteractor);
  globalInteraction->AddListener(secondInteractor);
  if ( !globalInteraction->ListenerRegistered(firstInteractor) ||
       !globalInteraction->ListenerRegistered(secondInteractor) )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }

  //remove Interactor
  if ( !globalInteraction->RemoveListener(secondInteractor) )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }

  //check if really removed
  if ( globalInteraction->ListenerRegistered(secondInteractor) )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  
  //still registered
  if ( !globalInteraction->ListenerRegistered(firstInteractor) )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }

  //remove first too
  if ( !globalInteraction->RemoveListener(firstInteractor) )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  
  //check if empty
  if ( globalInteraction->ListenerRegistered(firstInteractor) ||
       globalInteraction->ListenerRegistered(secondInteractor) )
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }

  //well done!!! Passed!
  std::cout<<"[PASSED]"<<std::endl;

  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
