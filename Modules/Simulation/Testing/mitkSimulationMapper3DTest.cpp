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

#include <mitkNodePredicateDataType.h>
#include <mitkRenderingTestHelper.h>
#include <mitkSimulation.h>
#include <mitkSimulationObjectFactory.h>
#include <mitkTestingMacros.h>
#include <sofa/simulation/common/UpdateContextVisitor.h>

using namespace mitk;
using namespace std;

int mitkSimulationMapper3DTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("mitkSimulationMapper3DTest")

  MITK_TEST_CONDITION_REQUIRED(argc == 4, "Check if command line has correct number of arguments.")

  MITK_TEST_OUTPUT(<< "Register SimulationObjectFactory.")
  RegisterSimulationObjectFactory();

  MITK_TEST_OUTPUT(<< "Create RenderingTestHelper.")
  mitkRenderingTestHelper renderingTestHelper(1024, 768, argc, argv);
  renderingTestHelper.SetMapperIDToRender3D();

  DataNode* simulationNode = renderingTestHelper.GetDataStorage()->GetNode(NodePredicateDataType::New("Simulation"));
  MITK_TEST_CONDITION_REQUIRED(simulationNode != NULL, "Check if simulation scene was loaded correctly.");

  MITK_TEST_OUTPUT(<< "Initialize simulation.")
  Simulation::Pointer simulation = dynamic_cast<Simulation*>(simulationNode->GetData());
  simulation->SetAsActiveSimulation();
  simulation->GetRootNode()->execute<sofa::simulation::UpdateContextVisitor>(sofa::core::ExecParams::defaultInstance());

  MITK_TEST_OUTPUT(<< "Set camera.")
  renderingTestHelper.SetViewDirection(mitk::SliceNavigationController::Frontal);
  renderingTestHelper.GetVtkRenderer()->ResetCamera();

  // renderingTestHelper.SaveReferenceScreenShot(".../CMakeExternals/Source/MITK-Data/Simulation/mapper1024x768REF.png");
  MITK_TEST_CONDITION(renderingTestHelper.CompareRenderWindowAgainstReference(argc, argv) == true, "Compare visual models with reference image.")

  simulationNode->SetBoolProperty("Simulation.Options.Normals", true);

  // renderingTestHelper.SaveReferenceScreenShot(".../CMakeExternals/Source/MITK-Data/Simulation/mapper1024x768REF_1.png");
  MITK_TEST_CONDITION(renderingTestHelper.CompareRenderWindowAgainstReference(argc, argv) == true, "Compare visual model (normals) with reference image.")

  simulationNode->SetBoolProperty("Simulation.Options.Normals", false);
  simulationNode->SetBoolProperty("Simulation.Options.Wire Frame", true);

  // renderingTestHelper.SaveReferenceScreenShot(".../CMakeExternals/Source/MITK-Data/Simulation/mapper1024x768REF_2.png");
  MITK_TEST_CONDITION(renderingTestHelper.CompareRenderWindowAgainstReference(argc, argv) == true, "Compare visual model (wire frame) with reference image.")

  MITK_TEST_END()
}
