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
#include <mitkTestingMacros.h>
#include <sofa/core/visual/VisualParams.h>
#include <sofa/simulation/common/UpdateContextVisitor.h>
#include <sofa/simulation/tree/GNode.h>
#include <sstream>

template <typename T>
static T lexical_cast(const std::string& string)
{
  std::istringstream sstream(string);
  T value;

  sstream >> value;

  if (sstream.fail())
  {
    MITK_ERROR << "Lexical cast failed for '" << string << "'!";
    exit(EXIT_FAILURE);
  }

  return value;
}

static mitk::Simulation::Pointer LoadSimulation(const std::string& filename)
{
  mitk::DataNode::Pointer dataNode = mitk::IOUtil::LoadDataNode(filename);
  mitk::Simulation::Pointer simulation = dynamic_cast<mitk::Simulation*>(dataNode->GetData());

  if (simulation.IsNull())
    mitkThrow() << "Could not load '" << filename << "'!";

  return simulation;
}

static bool IsActiveSimulation(mitk::Simulation::Pointer simulation)
{
  if (simulation.IsNull())
    mitkThrow() << "Invalid argument (null pointer)!";

  sofa::simulation::Simulation::SPtr sofaSimulation = simulation->GetSimulation();
  mitk::SimulationDrawTool* drawTool = simulation->GetDrawTool();

  sofa::simulation::Simulation* activeSimulation = sofa::simulation::getSimulation();
  sofa::core::visual::DrawTool*& activeDrawTool = sofa::core::visual::VisualParams::defaultInstance()->drawTool();

  return sofaSimulation == activeSimulation && drawTool == activeDrawTool;
}

static bool NoActiveSimulation()
{
  sofa::simulation::Simulation* activeSimulation = sofa::simulation::getSimulation();
  sofa::core::visual::DrawTool*& activeDrawTool = sofa::core::visual::VisualParams::defaultInstance()->drawTool();

  return activeSimulation == NULL && activeDrawTool == NULL;
}

static void DrawSimulation(mitk::Simulation::Pointer simulation, bool updateContext = true)
{
  if (simulation.IsNull())
    mitkThrow() << "Invalid argument (null pointer)!";

  sofa::simulation::Node::SPtr rootNode = simulation->GetRootNode();

  if (!rootNode)
    mitkThrow() << "Invalid argument (simulation is not initialized)!";

  simulation->SetAsActiveSimulation();

  if (updateContext)
    rootNode->execute<sofa::simulation::UpdateContextVisitor>(sofa::core::ExecParams::defaultInstance());

  sofa::simulation::Simulation::SPtr sofaSimulation = simulation->GetSimulation();

  sofaSimulation->updateVisual(rootNode.get());
  sofaSimulation->draw(sofa::core::visual::VisualParams::defaultInstance(), rootNode.get());
}

static vtkIdType GetNumberOfPolys(mitk::Surface::Pointer surface, unsigned int t = 0)
{
  vtkIdType numPolys = 0;

  if (surface.IsNotNull())
  {
    vtkPolyData* polyData = surface->GetVtkPolyData();

    if (polyData != NULL)
      numPolys = polyData->GetNumberOfPolys();
  }

  return numPolys;
}

static void SetActiveSimulation_InputIsNull_NoActiveSimulation()
{
  mitk::Simulation::SetActiveSimulation(NULL);
  MITK_TEST_CONDITION(NoActiveSimulation(), "SetActiveSimulation_InputIsNull_NoActiveSimulation")
}

static void SetAsActiveSimulation_NotInitialized_IsActiveSimulation()
{
  mitk::Simulation::Pointer simulation = mitk::Simulation::New();
  simulation->SetAsActiveSimulation();
  MITK_TEST_CONDITION(IsActiveSimulation(simulation), "SetAsActiveSimulation_NotInitialized_IsActiveSimulation")
}

static void SetAsActiveSimulation_Initialized_IsActiveSimulation(const std::string& filename)
{
  mitk::Simulation::Pointer simulation = LoadSimulation(filename);
  simulation->SetAsActiveSimulation();
  MITK_TEST_CONDITION(IsActiveSimulation(simulation), "SetAsActiveSimulation_Initialized_IsActiveSimulation")
}

static void GetRootNode_NotInitialized_ReturnsNull()
{
  mitk::Simulation::Pointer simulation = mitk::Simulation::New();
  MITK_TEST_CONDITION(!simulation->GetRootNode(), "GetRootNode_NotInitialized_ReturnsNull")
}

static void GetRootNode_Initialized_ReturnsRootNode(const std::string& filename)
{
  mitk::Simulation::Pointer simulation = LoadSimulation(filename);
  MITK_TEST_CONDITION(simulation->GetRootNode(), "GetRootNode_Initialized_ReturnsRootNode")
}

static void GetDefaultDT_NotInitialized_ReturnsZero()
{
  mitk::Simulation::Pointer simulation = mitk::Simulation::New();
  MITK_TEST_CONDITION(mitk::Equal(simulation->GetDefaultDT(), 0.0), "GetDefaultDT_NotInitialized_ReturnsZero")
}

static void GetDefaultDT_Initialized_ReturnsDefaultDT(const std::string& filename, double dt)
{
  mitk::Simulation::Pointer simulation = LoadSimulation(filename);
  MITK_TEST_CONDITION(mitk::Equal(simulation->GetDefaultDT(), dt), "GetDefaultDT_Initialized_ReturnsDefaultDT")
}

static void GetDrawTool_Always_ReturnsDrawTool()
{
  mitk::Simulation::Pointer simulation = mitk::Simulation::New();
  MITK_TEST_CONDITION(simulation->GetDrawTool() != NULL, "GetDrawTool_Always_ReturnsDrawTool")
}

static void GetSimulation_Always_ReturnsSimulation()
{
  mitk::Simulation::Pointer simulation = mitk::Simulation::New();
  MITK_TEST_CONDITION(simulation->GetSimulation(), "GetSimulation_Always_ReturnsSimulation")
}

static void SetRootNode_InputIsNull_RootNodeIsNull()
{
  mitk::Simulation::Pointer simulation = mitk::Simulation::New();
  simulation->SetRootNode(NULL);
  MITK_TEST_CONDITION(!simulation->GetRootNode(), "SetRootNode_InputIsNull_RootNodeIsNull")
}

static void SetRootNode_InputIsValid_RootNodeEqualsInput()
{
  mitk::Simulation::Pointer simulation = mitk::Simulation::New();
  sofa::simulation::Node::SPtr rootNode = sofa::core::objectmodel::SPtr_dynamic_cast<sofa::simulation::Node>(sofa::core::objectmodel::New<sofa::simulation::tree::GNode>());
  simulation->SetRootNode(rootNode.get());
  MITK_TEST_CONDITION(simulation->GetRootNode() == rootNode, "SetRootNode_InputIsValid_RootNodeEqualsInput")
}

static void SetDefaultDT_InputIsPositive_DefaultDTEqualsInput()
{
  mitk::Simulation::Pointer simulation = mitk::Simulation::New();
  const double dt = 0.1;
  simulation->SetDefaultDT(dt);
  MITK_TEST_CONDITION(mitk::Equal(simulation->GetDefaultDT(), dt), "SetDefaultDT_InputIsPositive_DefaultDTEqualsInput")
}

static void SetDefaultDT_InputIsNegative_DefaultDTIsZero()
{
  mitk::Simulation::Pointer simulation = mitk::Simulation::New();
  simulation->SetDefaultDT(-0.1);
  MITK_TEST_CONDITION(mitk::Equal(simulation->GetDefaultDT(), 0.0), "SetDefaultDT_InputIsNegative_DefaultDTIsZero")
}

static void TakeSnapshot_NotInitialized_ReturnsNull()
{
  mitk::Simulation::Pointer simulation = mitk::Simulation::New();
  MITK_TEST_CONDITION(simulation->TakeSnapshot().IsNull(), "TakeSnapshot_SimulationIsNotInitialized_ReturnsNull");
}

static void TakeSnapshot_SimulationWasDrawn_ReturnsSurface(const std::string& filename, vtkIdType numPolys)
{
  mitk::Simulation::Pointer simulation = LoadSimulation(filename);
  DrawSimulation(simulation);
  mitk::Surface::Pointer snapshot = simulation->TakeSnapshot();
  MITK_TEST_CONDITION(GetNumberOfPolys(snapshot) == numPolys, "TakeSnapshot_SimulationWasDrawn_ReturnsSurface");
}

static void AppendSnapshot_NotInitialized_ReturnsFalse()
{
  mitk::Simulation::Pointer simulation = mitk::Simulation::New();
  mitk::Surface::Pointer record = mitk::Surface::New();
  MITK_TEST_CONDITION(!simulation->AppendSnapshot(record), "AppendSnapshot_NotInitialized_ReturnsFalse");
}

static void AppendSnapshot_InputIsNull_ReturnsFalse(const std::string& filename)
{
  mitk::Simulation::Pointer simulation = mitk::Simulation::New();
  MITK_TEST_CONDITION(!simulation->AppendSnapshot(NULL), "AppendSnapshot_InputIsNull_ReturnsFalse");
}

static void AppendSnapshot_InputIsEmpty_AppendsSnapshotAndReturnsTrue(const std::string& filename, vtkIdType numPolys)
{
  mitk::Simulation::Pointer simulation = LoadSimulation(filename);
  DrawSimulation(simulation);
  mitk::Surface::Pointer record = mitk::Surface::New();
  MITK_TEST_CONDITION(simulation->AppendSnapshot(record) && GetNumberOfPolys(record) == numPolys, "AppendSnapshot_InputIsEmpty_AppendsSnapshotAndReturnsTrue")
}

static void AppendSnapshot_InputIsNotEmpty_AppendsSnapshotAndReturnsTrue(const std::string& filename, vtkIdType numPolys)
{
  mitk::Simulation::Pointer simulation = LoadSimulation(filename);
  DrawSimulation(simulation);
  mitk::Surface::Pointer record = mitk::Surface::New();
  simulation->AppendSnapshot(record);
  MITK_TEST_CONDITION(simulation->AppendSnapshot(record) && GetNumberOfPolys(record, 1) == numPolys, "AppendSnapshot_InputIsNotEmpty_AppendsSnapshotAndReturnsTrue")
}

int mitkSimulationTest(int argc, char* argv[])
{
  if (argc != 4)
  {
    MITK_ERROR << "Invalid argument count!\n"
               << "Usage: mitkSimulationTest <filename> <dt> <numPolys>\n"
               << "  <filename> Path to simulation scene\n"
               << "  <dt>       Default time step\n"
               << "  <numPolys> Total polygon count of all visual models";

    return EXIT_FAILURE;
  }

  const std::string filename = argv[1];
  const double dt = lexical_cast<double>(argv[2]);
  const vtkIdType numPolys = lexical_cast<vtkIdType>(argv[3]);

  mitk::RegisterSimulationObjectFactory();

  MITK_TEST_BEGIN("mitkSimulationTest")

    SetActiveSimulation_InputIsNull_NoActiveSimulation();
    SetAsActiveSimulation_NotInitialized_IsActiveSimulation();
    SetAsActiveSimulation_Initialized_IsActiveSimulation(filename);

    GetRootNode_NotInitialized_ReturnsNull();
    GetRootNode_Initialized_ReturnsRootNode(filename);

    GetDefaultDT_NotInitialized_ReturnsZero();
    GetDefaultDT_Initialized_ReturnsDefaultDT(filename, dt);

    GetDrawTool_Always_ReturnsDrawTool();

    GetSimulation_Always_ReturnsSimulation();

    SetRootNode_InputIsNull_RootNodeIsNull();
    SetRootNode_InputIsValid_RootNodeEqualsInput();

    SetDefaultDT_InputIsPositive_DefaultDTEqualsInput();
    SetDefaultDT_InputIsNegative_DefaultDTIsZero();

    TakeSnapshot_NotInitialized_ReturnsNull();
    TakeSnapshot_SimulationWasDrawn_ReturnsSurface(filename, numPolys);

    AppendSnapshot_NotInitialized_ReturnsFalse();
    AppendSnapshot_InputIsNull_ReturnsFalse(filename);
    AppendSnapshot_InputIsEmpty_AppendsSnapshotAndReturnsTrue(filename, numPolys);
    AppendSnapshot_InputIsNotEmpty_AppendsSnapshotAndReturnsTrue(filename, numPolys);

  MITK_TEST_END()
}
