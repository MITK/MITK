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

#include <usGetModuleContext.h>
#include <usModuleActivator.h>
#include <usModuleContext.h>
#include <usModule.h>

#include "mitkIGTLMeasurements.h"

namespace mitk
{

  /**
  \brief Registers services for IGTL module.
  */
  class IGTLModuleActivator : public us::ModuleActivator
  {
  public:

    void Load(us::ModuleContext* context) override
    {
      // First create and register a instance.
      m_Measurement = IGTLMeasurements::New();
      m_MeasurementReg = context->RegisterService<IGTLMeasurements>(m_Measurement);
    }

    void Unload(us::ModuleContext*) override
    {
      m_MeasurementReg.Unregister();
      //delete m_Measurement;
      //m_Measurement.
    }

  private:
    IGTLMeasurements::Pointer m_Measurement;
    us::ServiceRegistration<IGTLMeasurements> m_MeasurementReg;
  };
}

US_EXPORT_MODULE_ACTIVATOR(mitk::IGTLModuleActivator)
