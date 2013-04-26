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
#include <mitkTestingMacros.h>

static void GetRootNode_SimulationIsNotInitialized_ReturnsNullPointer()
{
  mitk::Simulation::Pointer simulation = mitk::Simulation::New();
  sofa::simulation::Node::SPtr rootNode = simulation->GetRootNode();
  MITK_TEST_CONDITION(!rootNode, "GetRootNode_SimulationIsNotInitialized_ReturnsNullPointer")
}

static void GetRootNode_SimulationIsInitialized_ReturnsValidPointer()
{
}

int mitkSimulationTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("mitkSimulationTest")

  GetRootNode_SimulationIsNotInitialized_ReturnsNullPointer();
  GetRootNode_SimulationIsInitialized_ReturnsValidPointer();

  MITK_TEST_END()
}
