/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkPythonActivator_h
#define mitkPythonActivator_h

#include <usModuleActivator.h>
#include "usModuleContext.h"
#include "mitkPythonContext.h"
#include "mitkIPythonService.h"
#include <usServiceRegistration.h>

namespace mitk
{
    class PythonActivator : public us::ModuleActivator
    {
    public:

        void Load(us::ModuleContext* context) override
        {
          MITK_INFO << "PythonActivator::Load";
          // Registering PythonService as MicroService
          m_PythonService = std::make_unique<mitk::IPythonService>();
          us::ServiceProperties _PythonServiceProps;
          _PythonServiceProps["Name"] = std::string("PythonService");
          _PythonServiceProps["service.ranking"] = int(0);
          m_PythonServiceRegistration = context->RegisterService<mitk::IPythonService>(m_PythonService.get(), _PythonServiceProps);
        }

        void Unload(us::ModuleContext*) override {}

        ~PythonActivator() override{}

    private:
      std::unique_ptr<mitk::IPythonService> m_PythonService;
      us::ServiceRegistration<mitk::IPythonService> m_PythonServiceRegistration;
    };
}

US_EXPORT_MODULE_ACTIVATOR(mitk::PythonActivator)
#endif
