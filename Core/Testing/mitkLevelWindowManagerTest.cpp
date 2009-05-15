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

#include "mitkLevelWindowManager.h"
#include "mitkStandaloneDataStorage.h"

int mitkLevelWindowManagerTest(int, char* [])
{
  mitk::LevelWindowManager::Pointer manager;
  std::cout << "Testing mitk::LevelWindowManager::New(): ";
  manager = mitk::LevelWindowManager::New();
  if (manager.IsNull()) 
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;
   
  std::cout << "Creating DataStorage: ";
  mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindowManager SetDataStorage ";
  manager->SetDataStorage(ds);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindowManager GetDataStorage ";
  if (ds != manager->GetDataStorage())
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindowManager SetLevelWindowProperty ";
  mitk::LevelWindowProperty::Pointer levelWindowProperty = mitk::LevelWindowProperty::New();

  manager->SetLevelWindowProperty(levelWindowProperty);
  std::cout<<"[PASSED]"<<std::endl;
  std::cout << "Testing mitk::LevelWindowManager GetLevelWindowProperty ";
  if (levelWindowProperty != manager->GetLevelWindowProperty())
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;
  std::cout << "Testing mitk::LevelWindowManager isAutoTopMost ";
  if (manager->isAutoTopMost())
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing mitk::LevelWindowManager GetLevelWindow ";
  try
  {
    const mitk::LevelWindow levelWindow = manager->GetLevelWindow();
    std::cout<<"[PASSED]"<<std::endl;

    std::cout << "Testing mitk::LevelWindowManager SetLevelWindow ";
    manager->SetLevelWindow(levelWindow);
    std::cout<<"[PASSED]"<<std::endl;
  }
  catch (...)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "Testing mitk::LevelWindowManager SetAutoTopMostImage ";
  manager->SetAutoTopMostImage(true);
  std::cout<<"[PASSED]"<<std::endl;
  std::cout << "Testing mitk::LevelWindowManager isAutoTopMost ";
  if (!(manager->isAutoTopMost()))
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
