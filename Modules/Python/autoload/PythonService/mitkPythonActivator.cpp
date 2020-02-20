/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkPythonActivator_h
#define mitkPythonActivator_h

// Microservices
#include <usModuleActivator.h>
#include "usModuleContext.h"
#include "mitkPythonService.h"
#include <usServiceRegistration.h>

namespace mitk
{
    ///
    /// installs the PythonService
    /// runs all initial commands (setting env paths etc)
    ///
    class PythonActivator : public us::ModuleActivator
    {
    public:

        void Load(us::ModuleContext* context) override
        {
          MITK_DEBUG << "PythonActivator::Load";
          // Registering PythonService as MicroService
          m_PythonService = itk::SmartPointer<mitk::PythonService>(new PythonService());

          us::ServiceProperties _PythonServiceProps;
          _PythonServiceProps["Name"] = std::string("PythonService");

          m_PythonServiceRegistration = context->RegisterService<mitk::IPythonService>(m_PythonService.GetPointer(), _PythonServiceProps);
        }

        void Unload(us::ModuleContext*) override
        {
          MITK_DEBUG("PythonActivator") << "PythonActivator::Unload";
          MITK_DEBUG("PythonActivator") << "m_PythonService GetReferenceCount " << m_PythonService->GetReferenceCount();
          m_PythonServiceRegistration.Unregister();
          m_PythonService->Delete();
          MITK_DEBUG("PythonActivator") << "m_PythonService GetReferenceCount " << m_PythonService->GetReferenceCount();
        }

        ~PythonActivator() override
        {
        }

    private:
        itk::SmartPointer<mitk::PythonService> m_PythonService;
        us::ServiceRegistration<PythonService> m_PythonServiceRegistration;
    };
}

US_EXPORT_MODULE_ACTIVATOR(mitk::PythonActivator)
#endif
