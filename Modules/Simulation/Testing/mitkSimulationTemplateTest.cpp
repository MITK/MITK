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

#include <mitkIOUtil.h>
#include <mitkSimulationObjectFactory.h>
#include <mitkSimulationTemplate.h>
#include <mitkTestingMacros.h>

int mitkSimulationTemplateTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("mitkSimulationTemplateTest")

  MITK_TEST_CONDITION_REQUIRED(argc == 2, "Test if command line has argument.")

  MITK_TEST_OUTPUT(<< "Register SimulationObjectFactory.")
  mitk::RegisterSimulationObjectFactory();

  mitk::SimulationTemplate::Pointer simulationTemplate = mitk::SimulationTemplate::New();
  MITK_TEST_CONDITION_REQUIRED(simulationTemplate.IsNotNull(), "Create simulation template.")

  std::string contents = simulationTemplate->CreateSimulation();
  MITK_TEST_CONDITION(contents.empty(), "Try to create simulation from template.")

  bool boolResult = simulationTemplate->SetProperties(NULL);
  MITK_TEST_CONDITION(!boolResult, "Try to set properties of non-existent data node.")

  boolResult = simulationTemplate->SetProperties(mitk::DataNode::New());
  MITK_TEST_CONDITION(!boolResult, "Try to set properties of empty data node.")

  boolResult = simulationTemplate->Parse("");
  MITK_TEST_CONDITION(boolResult, "Parse empty simulation template.");

  boolResult = simulationTemplate->Parse("");
  MITK_TEST_CONDITION(!boolResult, "Try to parse already initialized simulation template.")

  boolResult = simulationTemplate->SetProperties(mitk::DataNode::New());
  MITK_TEST_CONDITION(!boolResult, "Try to set properties of empty data node again.")

  contents = simulationTemplate->CreateSimulation();
  MITK_TEST_CONDITION(contents.empty(), "Create empty simulation.")

  simulationTemplate = mitk::SimulationTemplate::New();
  MITK_TEST_CONDITION_REQUIRED(simulationTemplate.IsNotNull(), "Create another simulation template.")

  contents = "<!-- -->";
  boolResult = simulationTemplate->Parse(contents);
  MITK_TEST_CONDITION(boolResult, "Parse static simulation template.")

  std::string contents2 = simulationTemplate->CreateSimulation();
  MITK_TEST_CONDITION(contents == contents2, "Create simulation.")

  simulationTemplate = mitk::SimulationTemplate::New();
  MITK_TEST_CONDITION_REQUIRED(simulationTemplate.IsNotNull(), "Create another simulation template.")

  contents = "{'F'}|{id='A'}|{id = 'B' type='string'}|{id ='C' type=\t\t\n\t'int'}|{id= 'D' type='float'}|{id='E' default='E'}|{id='F' type='int', default='1'}|{id='G' type='float' default='0.5'}|{'E'}";
  boolResult = simulationTemplate->Parse(contents);
  MITK_TEST_CONDITION(boolResult, "Parse simulation template.")

  mitk::DataNode::Pointer dataNode = mitk::DataNode::New();
  dataNode->SetData(simulationTemplate);
  boolResult = simulationTemplate->SetProperties(dataNode);
  MITK_TEST_CONDITION(boolResult, "Set properties of corresponding data node.")

  dataNode->SetStringProperty("A", "A");
  dataNode->SetIntProperty("F", 2);
  contents = simulationTemplate->CreateSimulation();
  MITK_TEST_CONDITION(contents == "2|A||0|0|E|2|0.5|E", "Create Simulation")

  // TODOs
  // - Ambiguous IDs
  // - Syntax errors

  MITK_TEST_END()
}
