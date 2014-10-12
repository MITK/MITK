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

#include <mitkSimulationService.h>
#include <usModuleActivator.h>
#include <usModuleContext.h>
#include <memory>

namespace mitk
{
  class SimulationActivator : public us::ModuleActivator
  {
  public:
    SimulationActivator()
    {
    }

    ~SimulationActivator()
    {
    }

    void Load(us::ModuleContext* context)
    {
      m_SimulationService.reset(new SimulationService);
      context->RegisterService<ISimulationService>(m_SimulationService.get());
    }

    void Unload(us::ModuleContext*)
    {
      m_SimulationService.reset(NULL);
    }

  private:
    SimulationActivator(const SimulationActivator&);
    SimulationActivator& operator=(const SimulationActivator&);

    std::auto_ptr<SimulationService> m_SimulationService;
  };
}

US_EXPORT_MODULE_ACTIVATOR(mitk::SimulationActivator);
