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
#include <mitkServiceInterface.h>
#include <MitkSimulationExports.h>

namespace mitk
{
  class Scheduler;

  class MitkSimulation_EXPORT ISimulationService
  {
  public:
    virtual Simulation::Pointer GetActiveSimulation() const = 0;
    virtual void SetActiveSimulation(Simulation::Pointer activeSimulation) = 0;
    virtual Scheduler* GetScheduler() = 0;

  protected:
    ISimulationService();
    virtual ~ISimulationService();

  private:
    ISimulationService(const ISimulationService&);
    ISimulationService& operator=(const ISimulationService&);
  };
}

MITK_DECLARE_SERVICE_INTERFACE(mitk::ISimulationService, "org.mitk.ISimulationService");

#endif
