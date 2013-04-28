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

static mitk::DataNode::Pointer CreateDataNode(mitk::SimulationTemplate::Pointer simulationTemplate)
{
  mitk::DataNode::Pointer dataNode = mitk::DataNode::New();
  dataNode->SetData(simulationTemplate);
  return dataNode;
}

static void CreateSimulation_NotInitialized_ReturnsEmptyString()
{
  mitk::SimulationTemplate::Pointer simulationTemplate = mitk::SimulationTemplate::New();
  MITK_TEST_CONDITION(simulationTemplate->CreateSimulation().empty(), "CreateSimulation_NotInitialized_ReturnsEmptyString")
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

int mitkSimulationTemplateTest(int, char* [])
{
  mitk::RegisterSimulationObjectFactory();

  MITK_TEST_BEGIN("mitkSimulationTemplateTest")

    CreateSimulation_NotInitialized_ReturnsEmptyString();

    SetProperties_InputIsNull_ReturnsFalse();
    SetProperties_NotInitialized_ReturnsFalse();
    SetProperties_ContainsTemplateAndReference_SetsPropertyAndReturnsTrue();

  MITK_TEST_END()
}
