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
#include <sofa/helper/system/SetDirectory.h>

using namespace mitk;
using namespace std;

int mitkSimulationTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("mitkSimulationTest")

  MITK_TEST_CONDITION_REQUIRED(argc == 2, "Test if command line has argument.")

  MITK_TEST_CONDITION(Simulation::ScaleFactor > 0.0f, "Check if simulation scale factor is greater than zero.")

  Simulation::Pointer simulation = Simulation::New();
  MITK_TEST_CONDITION_REQUIRED(simulation.IsNotNull(), "Create simulation.")

  double defaultDT = simulation->GetDefaultDT();
  MITK_TEST_CONDITION(defaultDT == 0.0, "Get default time step.")

  SimulationDrawTool* drawTool = simulation->GetDrawTool();
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
  Simulation::SetActiveSimulation(simulation);
  activeSimulation = sofa::simulation::getSimulation();
  MITK_TEST_CONDITION(activeSimulation == sofaSimulation.get(), "Set simulation as active simulation (static version).");

  Surface::Pointer snapshot = simulation->TakeSnapshot();
  MITK_TEST_CONDITION(snapshot.IsNull(), "Try to take snapshot.")

  Surface::Pointer record;
  bool boolResult = simulation->AppendSnapshot(record);
  MITK_TEST_CONDITION(boolResult == false && record.IsNull(), "Try to append snapshot to NULL surface.")

  record = Surface::New();
  boolResult = simulation->AppendSnapshot(record);
  MITK_TEST_CONDITION(boolResult == false && record->GetSizeOfPolyDataSeries() == 1 && record->IsEmptyTimeStep(0), "Try to append snapshot to empty surface.")

  string path = sofa::helper::system::SetDirectory::GetParentDir(argv[1]);
  sofa::helper::system::DataRepository.addFirstPath(path);
  rootNode = sofa::core::objectmodel::SPtr_dynamic_cast<sofa::simulation::Node>(sofaSimulation->load(argv[1]));
  MITK_TEST_CONDITION_REQUIRED(rootNode.get() != NULL, "Load simulation scene.");

  simulation->SetRootNode(rootNode.get());
  sofa::simulation::Node::SPtr rootNode2 = simulation->GetRootNode();
  MITK_TEST_CONDITION_REQUIRED(rootNode.get() == rootNode2.get(), "Set root node.")

  defaultDT = rootNode->getDt();
  simulation->SetDefaultDT(defaultDT);
  MITK_TEST_CONDITION(defaultDT == simulation->GetDefaultDT(), "Set default time step.")

  sofaSimulation->init(rootNode.get());
  sofa::helper::system::DataRepository.removePath(path);

  simulation->SetAsActiveSimulation();
  activeSimulation = sofa::simulation::getSimulation();
  MITK_TEST_CONDITION_REQUIRED(sofaSimulation.get() == activeSimulation, "Set simulation as active simulation.")

  record = simulation->TakeSnapshot();
  MITK_TEST_CONDITION(record.IsNotNull() && record->GetSizeOfPolyDataSeries() == 1 && record->IsEmptyTimeStep(0) == false, "Take snapshot.")

  boolResult = simulation->AppendSnapshot(record);
  MITK_TEST_CONDITION(boolResult == true && record->GetSizeOfPolyDataSeries() == 2 && record->IsEmptyTimeStep(1) == false, "Append snapshot.")

  MITK_TEST_END()
}
