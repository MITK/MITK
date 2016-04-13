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
#include "mitkTestingMacros.h"
#include "mitkToolManager.h"
#include "mitkStandaloneDataStorage.h"
#include "mitkCoreObjectFactory.h"

class mitkToolManagerTestClass { public:

static void TestToolManagerWithOutTools(mitk::ToolManager::Pointer toolManager)
{
  MITK_TEST_CONDITION( toolManager->GetTools().size() == 0, "Get empty tool list" )
  MITK_TEST_CONDITION( toolManager->GetToolById(0) == NULL, "Get empty tool by id" )
}

static void TestToolManagerWithTools(mitk::ToolManager::Pointer toolManager)
{
  MITK_TEST_CONDITION( toolManager->GetTools().size() > 0, "Get tool list with size 1" )
  MITK_TEST_CONDITION( toolManager->GetToolById(0) != NULL, "Test GetToolById() method" )
  MITK_TEST_CONDITION( toolManager->ActivateTool(0) == true, "Activate tool" )
  MITK_TEST_CONDITION( toolManager->GetActiveToolID() == 0, "Check for right tool id" )

  mitk::Tool* tool = toolManager->GetActiveTool();
  MITK_TEST_CONDITION( tool != NULL, "Check for right tool" )
}

static void TestSetterMethods(mitk::ToolManager::Pointer toolManager)
{
  MITK_TEST_CONDITION( toolManager->GetReferenceData().size() == 0, "Get reference data size (0)" )

  mitk::DataNode::Pointer nodeEmpty = mitk::DataNode::New();
  toolManager->SetReferenceData(nodeEmpty);
  MITK_TEST_CONDITION( toolManager->GetReferenceData().size() == 1, "Get reference data size (1)" )
  MITK_TEST_CONDITION( toolManager->GetReferenceData(0) == nodeEmpty, "Check if it is the right reference data" )
  MITK_TEST_CONDITION( toolManager->GetReferenceData()[0] == nodeEmpty, "Check if it is the right reference data vector" )

  mitk::DataNode::Pointer nodeEmpty2 = mitk::DataNode::New();
  toolManager->SetWorkingData(nodeEmpty2);
  MITK_TEST_CONDITION( toolManager->GetWorkingData().size() == 1, "Get working data size (1)" )
  MITK_TEST_CONDITION( toolManager->GetWorkingData(0) == nodeEmpty2, "Check if it is the right working data" )
  MITK_TEST_CONDITION( toolManager->GetWorkingData()[0] == nodeEmpty2, "Check if it is the right working data vector" )
}

};

int mitkToolManagerTest(int, char* [])
{
  MITK_TEST_BEGIN("ToolManager")

  mitk::StandaloneDataStorage::Pointer dataStorage = mitk::StandaloneDataStorage::New();
  MITK_TEST_CONDITION_REQUIRED(dataStorage.IsNotNull(), "StandaloneDataManager instantiation")

  MITK_INFO << "Instantiate ToolManager...";
  mitk::ToolManager::Pointer toolManager = mitk::ToolManager::New(dataStorage.GetPointer());
  MITK_TEST_CONDITION_REQUIRED(toolManager.IsNotNull(), "ToolManager instantiation")

  MITK_INFO << "Start ToolManager tests...";
  mitkToolManagerTestClass::TestToolManagerWithTools(toolManager);
  mitkToolManagerTestClass::TestSetterMethods(toolManager);

  MITK_TEST_END()
}
