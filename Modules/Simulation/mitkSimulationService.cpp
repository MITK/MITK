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

#include "mitkSimulationService.h"
#include <sofa/core/visual/VisualParams.h>

mitk::SimulationService::SimulationService()
{
}

mitk::SimulationService::~SimulationService()
{
}

mitk::Simulation::Pointer mitk::SimulationService::GetSimulation() const
{
  return m_Simulation;
}

void mitk::SimulationService::SetSimulation(Simulation::Pointer simulation)
{
  if (simulation.IsNull())
  {
    sofa::simulation::setSimulation(NULL);
    sofa::core::visual::VisualParams::defaultInstance()->drawTool() = NULL;
  }
  else
  {
    sofa::simulation::Simulation::SPtr sofaSimulation = simulation->GetSimulation();

    if (sofaSimulation != sofa::simulation::getSimulation())
    {
      sofa::simulation::setSimulation(sofaSimulation.get());
      sofa::core::visual::VisualParams::defaultInstance()->drawTool() = simulation->GetDrawTool();
    }
  }

  m_Simulation = simulation;
}
