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

#include "mitkSimulationPropAssemblyVisitor.h"
#include "mitkSimulationModel.h"
#include <vtkActor.h>
#include <vtkPropAssembly.h>

mitk::SimulationPropAssemblyVisitor::SimulationPropAssemblyVisitor(vtkPropAssembly* propAssembly, const sofa::core::ExecParams* params)
  : Visitor(params),
    m_PropAssembly(propAssembly)
{
}

mitk::SimulationPropAssemblyVisitor::~SimulationPropAssemblyVisitor()
{
}

sofa::simulation::Visitor::Result mitk::SimulationPropAssemblyVisitor::processNodeTopDown(sofa::simulation::Node* node)
{
  typedef sofa::simulation::Node::Sequence<sofa::core::visual::VisualModel>::const_iterator VisualModelIterator;
  VisualModelIterator end = node->visualModel.end();

  for (VisualModelIterator visualModel = node->visualModel.begin(); visualModel != end; ++visualModel)
  {
    SimulationModel* simulationModel = dynamic_cast<SimulationModel*>(*visualModel);

    if (simulationModel != NULL)
    {
      std::vector<vtkActor*> actors = simulationModel->GetActors();

      for (std::vector<vtkActor*>::const_iterator actor = actors.begin(); actor != actors.end(); ++actor)
        m_PropAssembly->AddPart(*actor);
    }
  }

  return RESULT_CONTINUE;
}
