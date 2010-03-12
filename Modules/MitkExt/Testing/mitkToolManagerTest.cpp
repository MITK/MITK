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
#include "mitkTestingMacros.h"
#include "mitkToolManager.h"
#include "mitkStandaloneDataStorage.h"
#include "mitkCoreObjectFactory.h"
#include "mitkAddContourTool.h"
#include "mitkGlobalInteraction.h"

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

int mitkToolManagerTest(int /* argc */, char* /*argv*/[])
{
  // always start with this!
  MITK_TEST_BEGIN("ToolManager")

  // Global interaction must(!) be initialized if used
  mitk::GlobalInteraction::GetInstance()->Initialize("global");

  // instantiation
  mitk::StandaloneDataStorage::Pointer dataStorage = mitk::StandaloneDataStorage::New();
  mitk::ToolManager::Pointer toolManager = mitk::ToolManager::New(dataStorage.GetPointer());

  // first test: did this work?
  // using MITK_TEST_CONDITION_REQUIRED makes the test stop after failure, since
  // it makes no sense to continue without an object.
  MITK_TEST_CONDITION_REQUIRED(toolManager.IsNotNull(),"Testing instantiation") 

  // write your own tests here and use the macros from mitkTestingMacros.h !!!
  // do not write to std::cout and do not return from this function yourself!
  //mitkToolManagerTestClass::TestToolManagerWithOutTools(toolManager);

  //now we add one tool
  toolManager = mitk::ToolManager::New(dataStorage.GetPointer());
 
  //start test with tool
  mitkToolManagerTestClass::TestToolManagerWithTools(toolManager);
  
  //now the setter methods
  mitkToolManagerTestClass::TestSetterMethods(toolManager);

  // always end with this!
  MITK_TEST_END()
}
