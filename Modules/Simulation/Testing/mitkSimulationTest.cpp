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

#include <mitkSimulation.h>
#include <mitkSimulationObjectFactory.h>
#include <mitkTestingMacros.h>

static void GetRootNode_SimulationNotInitialized_ReturnsNullPointer()
{
  mitk::Simulation::Pointer simulation = mitk::Simulation::New();
  sofa::simulation::Node::SPtr rootNode = simulation->GetRootNode();
  MITK_TEST_CONDITION(!rootNode, "GetRootNode_SimulationNotInitialized_ReturnsNullPointer")
}

int mitkSimulationTest(int, char* [])
{
  mitk::RegisterSimulationObjectFactory();

  MITK_TEST_BEGIN("mitkSimulationTest")

  GetRootNode_SimulationNotInitialized_ReturnsNullPointer();

  MITK_TEST_END()
}
