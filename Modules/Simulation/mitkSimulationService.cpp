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

static void SwitchSimuluationContext(mitk::Simulation::Pointer activeSimulation)
{
  if (activeSimulation.IsNull())
  {
    sofa::simulation::setSimulation(NULL);
    sofa::core::visual::VisualParams::defaultInstance()->drawTool() = NULL;
  }
  else
  {
    sofa::simulation::Simulation::SPtr sofaSimulation = activeSimulation->GetSOFASimulation();

    if (sofaSimulation != sofa::simulation::getSimulation())
    {
      sofa::simulation::setSimulation(sofaSimulation.get());
      sofa::core::visual::VisualParams::defaultInstance()->drawTool() = activeSimulation->GetDrawTool();
    }
  }
}

mitk::SimulationService::SimulationService()
  : m_Scheduler(SchedulingAlgorithm::WeightedRoundRobin)
{
}

mitk::SimulationService::~SimulationService()
{
}

mitk::Simulation::Pointer mitk::SimulationService::GetActiveSimulation() const
{
  return m_ActiveSimulation;
}

void mitk::SimulationService::SetActiveSimulation(Simulation::Pointer activeSimulation)
{
  SwitchSimuluationContext(activeSimulation);
  m_ActiveSimulation = activeSimulation;
}

mitk::Scheduler* mitk::SimulationService::GetScheduler()
{
  return &m_Scheduler;
}
