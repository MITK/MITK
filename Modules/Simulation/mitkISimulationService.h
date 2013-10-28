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

#ifndef mitkISimulationService_h
#define mitkISimulationService_h

#include <mitkSimulation.h>
#include <usServiceInterface.h>
#include <SimulationExports.h>

namespace mitk
{
  class Simulation_EXPORT ISimulationService
  {
  public:
    virtual Simulation::Pointer GetSimulation() const = 0;
    virtual void SetSimulation(Simulation::Pointer simulation) = 0;

  protected:
    ISimulationService();
    virtual ~ISimulationService();

  private:
    ISimulationService(const ISimulationService&);
    ISimulationService& operator=(const ISimulationService&);
  };
}

US_DECLARE_SERVICE_INTERFACE(mitk::ISimulationService, "org.mitk.ISimulationService");

#endif
