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

  static void TestRemoveObserver(std::string testImageFile)
  {
  mitk::LevelWindowManager::Pointer manager;
  manager = mitk::LevelWindowManager::New();
  mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();
  manager->SetDataStorage(ds);

  //add multiple objects to the data storage => multiple observers should be created
  mitk::Image::Pointer image1 = mitk::IOUtil::LoadImage(testImageFile);
  mitk::DataNode::Pointer node1 = mitk::DataNode::New();
  node1->SetData(image1);
  mitk::Image::Pointer image2 = mitk::IOUtil::LoadImage(testImageFile);
  mitk::DataNode::Pointer node2 = mitk::DataNode::New();
  node2->SetData(image2);
  ds->Add(node1);
  ds->Add(node2);

  MITK_TEST_CONDITION_REQUIRED(manager->GetRelevantNodes()->size() == 2, "Test if nodes have been added");
  MITK_TEST_CONDITION_REQUIRED(manager->GetRelevantNodes()->size() == manager->GetNumberOfObservers(), "Test if number of nodes is similar to number of observers");

  mitk::Image::Pointer image3 = mitk::IOUtil::LoadImage(testImageFile);
  mitk::DataNode::Pointer node3 = mitk::DataNode::New();
  node3->SetData(image3);
  ds->Add(node3);
  MITK_TEST_CONDITION_REQUIRED(manager->GetRelevantNodes()->size() == 3, "Test if another node have been added");
  MITK_TEST_CONDITION_REQUIRED(manager->GetRelevantNodes()->size() == manager->GetNumberOfObservers(), "Test if number of nodes is similar to number of observers");

  ds->Remove(node1);
  MITK_TEST_CONDITION_REQUIRED(manager->GetRelevantNodes()->size() == 2, "Deleted node 1 (test GetRelevantNodes())");
  MITK_TEST_CONDITION_REQUIRED(manager->GetNumberOfObservers() == 2, "Deleted node 1 (test GetNumberOfObservers())");

  ds->Remove(node2);
  MITK_TEST_CONDITION_REQUIRED(manager->GetRelevantNodes()->size() == 1, "Deleted node 2 (test GetRelevantNodes())");
  MITK_TEST_CONDITION_REQUIRED(manager->GetNumberOfObservers() == 1, "Deleted node 2 (test GetNumberOfObservers())");

  ds->Remove(node3);
  MITK_TEST_CONDITION_REQUIRED(manager->GetRelevantNodes()->size() == 0, "Deleted node 3 (test GetRelevantNodes())");
  MITK_TEST_CONDITION_REQUIRED(manager->GetNumberOfObservers() == 0, "Deleted node 3 (test GetNumberOfObservers())");

  }

};

int mitkLevelWindowManagerTest(int argc, char* args[])
{
  MITK_TEST_BEGIN("mitkLevelWindowManager");

  MITK_TEST_CONDITION_REQUIRED( argc >= 2, "Testing if test file is given.");
  std::string testImage = args[1];

  mitkLevelWindowManagerTestClass::TestInstantiation();
  mitkLevelWindowManagerTestClass::TestSetGetDataStorage();
  mitkLevelWindowManagerTestClass::TestMethodsWithInvalidParameters();
  mitkLevelWindowManagerTestClass::TestOtherMethods();
  mitkLevelWindowManagerTestClass::TestRemoveObserver(testImage);

  MITK_TEST_END();
}
