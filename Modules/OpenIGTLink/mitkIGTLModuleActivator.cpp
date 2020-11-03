/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
