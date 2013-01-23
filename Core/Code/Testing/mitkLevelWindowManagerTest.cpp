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

#include "mitkLevelWindowManager.h"
#include "mitkStandaloneDataStorage.h"
#include <mitkTestingMacros.h>
#include <mitkIOUtil.h>

class mitkLevelWindowManagerTestClass
{
public:

  static void TestInstantiation()
  {
  mitk::LevelWindowManager::Pointer manager;
  manager = mitk::LevelWindowManager::New();
  MITK_TEST_CONDITION_REQUIRED(manager.IsNotNull(),"Testing mitk::LevelWindowManager::New()");
  }

  static void TestSetGetDataStorage()
  {
  mitk::LevelWindowManager::Pointer manager;
  manager = mitk::LevelWindowManager::New();
  MITK_TEST_OUTPUT(<< "Creating DataStorage: ");
  mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();

  bool success = true;
  try
    {
    manager->SetDataStorage(ds);
    }
  catch(std::exception e)
    {
    success = false;
    MITK_ERROR << "Exception: " << e.what();
    }
  MITK_TEST_CONDITION_REQUIRED(success,"Testing mitk::LevelWindowManager SetDataStorage() ");
  MITK_TEST_CONDITION_REQUIRED(ds == manager->GetDataStorage(),"Testing mitk::LevelWindowManager GetDataStorage ");

  }

  static void TestMethodsWithInvalidParameters()
  {
  mitk::LevelWindowManager::Pointer manager;
  manager = mitk::LevelWindowManager::New();
  mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();
  manager->SetDataStorage(ds);

  bool success = false;
  mitk::LevelWindowProperty::Pointer levelWindowProperty = mitk::LevelWindowProperty::New();
  try
    {
    manager->SetLevelWindowProperty(levelWindowProperty);
    }
  catch(mitk::Exception e)
    {
    success = true;
    }
  MITK_TEST_CONDITION(success,"Testing mitk::LevelWindowManager SetLevelWindowProperty with invalid parameter");
  }

  static void TestOtherMethods()
  {
  mitk::LevelWindowManager::Pointer manager;
  manager = mitk::LevelWindowManager::New();
  mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();
  manager->SetDataStorage(ds);

  MITK_TEST_CONDITION(manager->isAutoTopMost(),"Testing mitk::LevelWindowManager isAutoTopMost");

  bool success = true;
  try
  {
    mitk::LevelWindow levelWindow = manager->GetLevelWindow();
    manager->SetLevelWindow(levelWindow);
  }
  catch (...)
  {
    success == false;
  }
  MITK_TEST_CONDITION(success,"Testing mitk::LevelWindowManager GetLevelWindow() and SetLevelWindow()");

  manager->SetAutoTopMostImage(true);
  MITK_TEST_CONDITION(manager->isAutoTopMost(),"Testing mitk::LevelWindowManager isAutoTopMost()");
  }

  static void TestRemoveObserver()
  {
  mitk::LevelWindowManager::Pointer manager;
  manager = mitk::LevelWindowManager::New();
  mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();
  manager->SetDataStorage(ds);
  MITK_INFO << "Number of observers: " << manager->GetNumberOfObservers();
  //add multiple objects to the data storage => multiple observers should be created
  mitk::Image::Pointer image1 = mitk::IOUtil::LoadImageA("D:/prg/MITK-02-bin/CMakeExternals/Source/MITK-Data/Pic3D.nrrd");
  mitk::DataNode::Pointer node1 = mitk::DataNode::New();
  node1->SetData(image1);
  mitk::Image::Pointer image2 = mitk::IOUtil::LoadImageA("D:/prg/MITK-02-bin/CMakeExternals/Source/MITK-Data/Pic3D.nrrd");
  mitk::DataNode::Pointer node2 = mitk::DataNode::New();
  node2->SetData(image2);
  ds->Add(node1);
  ds->Add(node2);
  MITK_INFO << "Number of observers: " << manager->GetNumberOfObservers();
  MITK_TEST_CONDITION_REQUIRED(manager->GetRelevantNodes()->size() == 2, "Test if nodes have been added");

  ds->Remove(node1);
  ds->Remove(node2);
  //remove them
  MITK_INFO << "Number of observers: " << manager->GetNumberOfObservers();
  }

};

int mitkLevelWindowManagerTest(int, char* [])
{
  MITK_TEST_BEGIN("mitkLevelWindowManager");
  mitkLevelWindowManagerTestClass::TestInstantiation();
  mitkLevelWindowManagerTestClass::TestSetGetDataStorage();
  mitkLevelWindowManagerTestClass::TestMethodsWithInvalidParameters();
  mitkLevelWindowManagerTestClass::TestOtherMethods();
  //mitkLevelWindowManagerTestClass::TestRemoveObserver();

  MITK_TEST_END();
}
