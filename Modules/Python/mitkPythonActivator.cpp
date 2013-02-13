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
#ifndef mitkPythonActivator_h
#define mitkPythonActivator_h

// Microservices
#include <usModuleActivator.h>
#include "mitkModuleContext.h"
#include "mitkPythonService.h"
#include <usServiceRegistration.h>

namespace mitk
{
    ///
    /// installs the PythonService
    /// runs all initial commands (setting env paths etc)
    ///
    class PythonActivator : public mitk::ModuleActivator
    {
    public:

        void Load(mitk::ModuleContext* context)
        {
          // Registering PythonService as MicroService
          m_PythonService = itk::SmartPointer<mitk::PythonService>(new PythonService());

          ServiceProperties _PythonServiceProps;
          _PythonServiceProps["Name"] = std::string("PythonService");

          context->RegisterService<mitk::IPythonService>(m_PythonService, _PythonServiceProps);
        }

        void Unload(mitk::ModuleContext* context)
        {
          m_PythonService = 0;
        }

        ~PythonActivator()
        {
        }

    private:
        itk::SmartPointer<mitk::PythonService> m_PythonService;
    };
}

US_EXPORT_MODULE_ACTIVATOR(mitkPython, mitk::PythonActivator)
#endif
