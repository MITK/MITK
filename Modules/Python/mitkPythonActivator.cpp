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
#include "PythonPath.h"

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

            MITK_DEBUG("PythonActivator") << "registering python paths";

            QString _PYTHONPATH_ITK_LIBRARY_DIRS(PYTHONPATH_ITK_LIBRARY_DIRS);
            MITK_DEBUG("PythonActivator") << "_PYTHONPATH_ITK_LIBRARY_DIRS" << _PYTHONPATH_ITK_LIBRARY_DIRS.toStdString();

            QString _PYTHONPATH_WRAP_ITK_DIR(PYTHONPATH_WRAP_ITK_DIR);
            MITK_DEBUG("PythonActivator") << "_PYTHONPATH_WRAP_ITK_DIR" << _PYTHONPATH_WRAP_ITK_DIR.toStdString();

            QString _PYTHONPATH_VTK_LIBRARY_DIRS(PYTHONPATH_VTK_LIBRARY_DIRS);
            MITK_DEBUG("PythonActivator") << "_PYTHONPATH_VTK_LIBRARY_DIRS" << _PYTHONPATH_VTK_LIBRARY_DIRS.toStdString();

            QString _PYTHONPATH_VTK_PYTHON_WRAPPING_DIR(PYTHONPATH_VTK_PYTHON_WRAPPING_DIR);
            MITK_DEBUG("PythonActivator") << "_PYTHONPATH_VTK_PYTHON_WRAPPING_DIR" << _PYTHONPATH_VTK_PYTHON_WRAPPING_DIR.toStdString();

            QString _PYTHONPATH_OPEN_CV_LIBRARY_DIRS(PYTHONPATH_OPEN_CV_LIBRARY_DIRS);
            MITK_DEBUG("PythonActivator") << "_PYTHONPATH_OPEN_CV_LIBRARY_DIRS" << _PYTHONPATH_OPEN_CV_LIBRARY_DIRS.toStdString();

            QString basecommand = "sys.path.append('%1');";
            QString pythonCommand;

            pythonCommand = basecommand.arg(_PYTHONPATH_ITK_LIBRARY_DIRS);
            MITK_DEBUG("PythonActivator") << "issuing command " << pythonCommand.toStdString();
            m_PythonService->Execute(pythonCommand, mitk::IPythonService::SINGLE_LINE_COMMAND );

            pythonCommand = basecommand.arg(_PYTHONPATH_WRAP_ITK_DIR);
            MITK_DEBUG("PythonActivator") << "issuing command " << pythonCommand.toStdString();
            m_PythonService->Execute(pythonCommand, mitk::IPythonService::SINGLE_LINE_COMMAND );

            pythonCommand = basecommand.arg(_PYTHONPATH_VTK_LIBRARY_DIRS);
            MITK_DEBUG("PythonActivator") << "issuing command " << pythonCommand.toStdString();
            m_PythonService->Execute(pythonCommand, mitk::IPythonService::SINGLE_LINE_COMMAND );

            pythonCommand = basecommand.arg(_PYTHONPATH_VTK_PYTHON_WRAPPING_DIR);
            MITK_DEBUG("PythonActivator") << "issuing command " << pythonCommand.toStdString();
            m_PythonService->Execute(pythonCommand, mitk::IPythonService::SINGLE_LINE_COMMAND );

            pythonCommand = basecommand.arg(_PYTHONPATH_OPEN_CV_LIBRARY_DIRS);
            MITK_DEBUG("PythonActivator") << "issuing command " << pythonCommand.toStdString();
            m_PythonService->Execute(pythonCommand, mitk::IPythonService::SINGLE_LINE_COMMAND );
        }

        void Unload(mitk::ModuleContext* )
        {
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
