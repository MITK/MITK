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
#include <mitkSimulation.h>
#include <mitkSimulationObjectFactory.h>
#include <mitkSimulationModel.h>
#include <mitkTestingMacros.h>
#include <sofa/core/visual/VisualParams.h>
#include <sofa/simulation/common/UpdateContextVisitor.h>

int mitkSimulationTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("mitkSimulationTest")

  MITK_TEST_CONDITION_REQUIRED(argc == 2, "Test if command line has argument.")

  MITK_TEST_OUTPUT(<< "Register SimulationObjectFactory.")
  mitk::RegisterSimulationObjectFactory();

  MITK_TEST_CONDITION(mitk::Simulation::ScaleFactor > 0.0f, "Check if simulation scale factor is greater than zero.")

  mitk::Simulation::Pointer simulation = mitk::Simulation::New();
  MITK_TEST_CONDITION_REQUIRED(simulation.IsNotNull(), "Create simulation.")

  double defaultDT = simulation->GetDefaultDT();
  MITK_TEST_CONDITION(defaultDT == 0.0, "Get default time step.")

  mitk::SimulationDrawTool* drawTool = simulation->GetDrawTool();
  MITK_TEST_CONDITION(drawTool != NULL, "Get draw tool.")

  sofa::simulation::Node::SPtr rootNode = simulation->GetRootNode();
  MITK_TEST_CONDITION(rootNode.get() == NULL, "Get root node.")

  double dt = 0.42;
  simulation->SetDefaultDT(dt);
  defaultDT = simulation->GetDefaultDT();
  MITK_TEST_CONDITION(defaultDT == dt, "Set default time step.");

  sofa::simulation::Simulation* activeSimulation = sofa::simulation::getSimulation();
  MITK_TEST_CONDITION(activeSimulation == NULL, "Get active simulation.");

  sofa::simulation::Simulation::SPtr sofaSimulation = simulation->GetSimulation();
  MITK_TEST_CONDITION_REQUIRED(sofaSimulation != NULL, "Get default constructed SOFA simulation.")

  simulation->SetAsActiveSimulation();
  activeSimulation = sofa::simulation::getSimulation();
  MITK_TEST_CONDITION(activeSimulation == sofaSimulation.get(), "Set simulation as active simulation.")

  sofa::simulation::setSimulation(NULL);
  mitk::Simulation::SetActiveSimulation(simulation);
  activeSimulation = sofa::simulation::getSimulation();
  MITK_TEST_CONDITION(activeSimulation == sofaSimulation.get(), "Set simulation as active simulation (static version).");

  mitk::Surface::Pointer snapshot = simulation->TakeSnapshot();
  MITK_TEST_CONDITION(snapshot.IsNull(), "Try to take snapshot.")

  mitk::Surface::Pointer record;
  bool boolResult = simulation->AppendSnapshot(record);
  MITK_TEST_CONDITION(!boolResult && record.IsNull(), "Try to append snapshot to NULL surface.")

  record = mitk::Surface::New();
  boolResult = simulation->AppendSnapshot(record);
  MITK_TEST_CONDITION(!boolResult && record->GetSizeOfPolyDataSeries() == 1 && record->IsEmptyTimeStep(0), "Try to append snapshot to empty surface.")

  mitk::DataNode::Pointer dataNode = mitk::IOUtil::LoadDataNode(argv[1]);
  mitk::Simulation* simulation2 = dynamic_cast<mitk::Simulation*>(dataNode->GetData());
  sofa::simulation::Simulation::SPtr sofaSimulation2 = simulation2->GetSimulation();
  sofa::simulation::Node::SPtr rootNode2 = simulation2->GetRootNode();
  MITK_TEST_CONDITION_REQUIRED(rootNode2.get() != NULL, "Load simulation scene.");

  simulation2->SetAsActiveSimulation();
  activeSimulation = sofa::simulation::getSimulation();
  MITK_TEST_CONDITION_REQUIRED(sofaSimulation2.get() == activeSimulation, "Set simulation as active simulation.")

  MITK_TEST_OUTPUT(<< "Draw simulation scene.")
  rootNode2->execute<sofa::simulation::UpdateContextVisitor>(sofa::core::ExecParams::defaultInstance());
  sofaSimulation2->updateVisual(rootNode2.get());
  sofaSimulation2->draw(sofa::core::visual::VisualParams::defaultInstance(), rootNode2.get());

  record = simulation2->TakeSnapshot();
  MITK_TEST_CONDITION(record.IsNotNull() && record->GetSizeOfPolyDataSeries() == 1 && record->IsEmptyTimeStep(0) == false, "Take snapshot.")

  boolResult = simulation2->AppendSnapshot(record);
  MITK_TEST_CONDITION(boolResult && record->GetSizeOfPolyDataSeries() == 2 && record->IsEmptyTimeStep(1) == false, "Append snapshot.")

  MITK_TEST_END()
}
