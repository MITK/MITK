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

#ifndef mitkSimulationService_h
#define mitkSimulationService_h

#include <mitkISimulationService.h>

namespace mitk
{
  class SimulationService : public ISimulationService
  {
  public:
    SimulationService();
    ~SimulationService();

    Simulation::Pointer GetActiveSimulation() const;
    void SetActiveSimulation(Simulation::Pointer activeSimulation);

  private:
    Simulation::Pointer m_ActiveSimulation;
  };
}

#endif
