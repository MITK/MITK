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

#include <mitkSimulationObjectFactory.h>
#include <mitkSimulationTemplate.h>
#include <mitkTestingMacros.h>

#define _MITK_TEST_FOR_EXCEPTION(STATEMENT, EXCEPTION, MESSAGE) \
  MITK_TEST_OUTPUT_NO_ENDL(<< MESSAGE) \
  try \
  { \
    STATEMENT; \
    MITK_TEST_OUTPUT(<< " [FAILED]") \
    mitk::TestManager::GetInstance()->TestFailed(); \
  } \
  catch (const EXCEPTION& e) \
  { \
    MITK_TEST_OUTPUT(<< "\n  " << e.GetDescription() << " [PASSED]") \
    mitk::TestManager::GetInstance()->TestPassed(); \
  }

static mitk::DataNode::Pointer CreateDataNode(mitk::SimulationTemplate::Pointer simulationTemplate, bool setProperties = false)
{
  if (simulationTemplate.IsNull())
    mitkThrow() << "Invalid argument (null pointer!)";

  mitk::DataNode::Pointer dataNode = mitk::DataNode::New();
  dataNode->SetData(simulationTemplate);

  if (setProperties)
    simulationTemplate->SetProperties(dataNode);

  return dataNode;
}

static void Parse_InputIsEmpty_ReturnsTrue()
{
  mitk::SimulationTemplate::Pointer simulationTemplate = mitk::SimulationTemplate::New();
  MITK_TEST_CONDITION(simulationTemplate->Parse(""), "Parse_InputIsEmpty_ReturnsTrue")
}

static void Parse_AlreadyInitialized_ReturnsFalse()
{
  mitk::SimulationTemplate::Pointer simulationTemplate = mitk::SimulationTemplate::New();
  simulationTemplate->Parse("");
  MITK_TEST_CONDITION(!simulationTemplate->Parse(""), "Parse_AlreadyInitialized_ReturnsFalse")
}

static void Parse_EOFBeforeClosingBrace_ThrowsException()
{
  mitk::SimulationTemplate::Pointer simulationTemplate = mitk::SimulationTemplate::New();
  _MITK_TEST_FOR_EXCEPTION(simulationTemplate->Parse("{id='A' default='1'"), mitk::Exception, "Parse_EOFBeforeClosingBrace_ThrowsException")
}

static void Parse_OpeningBraceBeforeClosingBrace_ThrowsException()
{
  mitk::SimulationTemplate::Pointer simulationTemplate = mitk::SimulationTemplate::New();
  _MITK_TEST_FOR_EXCEPTION(simulationTemplate->Parse("{id='A' default='1' {id='B' default='2'}}"), mitk::Exception, "Parse_OpeningBraceBeforeClosingBrace_ThrowsException")
}

static void SetProperties_InputIsNull_ReturnsFalse()
{
  mitk::SimulationTemplate::Pointer simulationTemplate = mitk::SimulationTemplate::New();
  MITK_TEST_CONDITION(!simulationTemplate->SetProperties(NULL), "SetProperties_InputIsNull_ReturnsFalse")
}

static void SetProperties_NotInitialized_ReturnsFalse()
{
  mitk::SimulationTemplate::Pointer simulationTemplate = mitk::SimulationTemplate::New();
  mitk::DataNode::Pointer dataNode = CreateDataNode(simulationTemplate);
  MITK_TEST_CONDITION(!simulationTemplate->SetProperties(dataNode), "SetProperties_NotInitialized_ReturnsFalse")
}

static void SetProperties_InputIsNotOwner_ReturnsFalse()
{
  mitk::SimulationTemplate::Pointer simulationTemplate1 = mitk::SimulationTemplate::New();
  simulationTemplate1->Parse("{id='A' default='1'}");
  mitk::SimulationTemplate::Pointer simulationTemplate2 = mitk::SimulationTemplate::New();
  simulationTemplate2->Parse("{id='B' default='2'}");
  mitk::DataNode::Pointer dataNode1 = CreateDataNode(simulationTemplate1);
  MITK_TEST_CONDITION(!simulationTemplate2->SetProperties(dataNode1), "SetProperties_InputIsNotOwner_ReturnsFalse")
}

static void SetProperties_ContainsTemplateAndReference_SetsPropertyAndReturnsTrue()
{
  mitk::SimulationTemplate::Pointer simulationTemplate = mitk::SimulationTemplate::New();
  simulationTemplate->Parse("{id='Atoll' default='Bora'} {'Atoll'}");
  mitk::DataNode::Pointer dataNode = CreateDataNode(simulationTemplate);
  bool boolResult = simulationTemplate->SetProperties(dataNode);
  std::size_t numProperties = dataNode->GetPropertyList()->GetMap()->size();
  std::string stringResult;
  dataNode->GetStringProperty("Atoll", stringResult);
  MITK_TEST_CONDITION(boolResult && numProperties == 1 && stringResult == "Bora", "SetProperties_ContainsTemplateAndReference_SetsPropertyAndReturnsTrue")
}

static void CreateSimulation_NotInitialized_ReturnsEmptyString()
{
  mitk::SimulationTemplate::Pointer simulationTemplate = mitk::SimulationTemplate::New();
  MITK_TEST_CONDITION(simulationTemplate->CreateSimulation().empty(), "CreateSimulation_NotInitialized_ReturnsEmptyString")
}

static void CreateSimulation_ContainsInvalidReference_ReturnsEmptyString()
{
  mitk::SimulationTemplate::Pointer simulationTemplate = mitk::SimulationTemplate::New();
  simulationTemplate->Parse("{'Invalid'}");
  MITK_TEST_CONDITION(simulationTemplate->CreateSimulation().empty(), "CreateSimulation_ContainsInvalidReference_ReturnsEmptyString")
}

static void CreateSimulation_ContainsTemplateAndReference_ReturnsExpectedString()
{
  mitk::SimulationTemplate::Pointer simulationTemplate = mitk::SimulationTemplate::New();
  simulationTemplate->Parse("{id='Atoll' default='Bora'} {'Atoll'}");
  MITK_TEST_CONDITION(simulationTemplate->CreateSimulation() == "Bora Bora", "CreateSimulation_ContainsTemplateAndReference_ReturnsExpectedString")
}

int mitkSimulationTemplateTest(int, char* [])
{
  mitk::RegisterSimulationObjectFactory();

  MITK_TEST_BEGIN("mitkSimulationTemplateTest")

    Parse_InputIsEmpty_ReturnsTrue();
    Parse_AlreadyInitialized_ReturnsFalse();
    Parse_EOFBeforeClosingBrace_ThrowsException();
    Parse_OpeningBraceBeforeClosingBrace_ThrowsException();

    SetProperties_InputIsNull_ReturnsFalse();
    SetProperties_NotInitialized_ReturnsFalse();
    SetProperties_InputIsNotOwner_ReturnsFalse();
    SetProperties_ContainsTemplateAndReference_SetsPropertyAndReturnsTrue();

    CreateSimulation_NotInitialized_ReturnsEmptyString();
    CreateSimulation_ContainsInvalidReference_ReturnsEmptyString();
    CreateSimulation_ContainsTemplateAndReference_ReturnsExpectedString();

  MITK_TEST_END()
}
