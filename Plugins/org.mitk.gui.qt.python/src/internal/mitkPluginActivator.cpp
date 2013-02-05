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
#include "mitkPluginActivator.h"
#include <QtPlugin>
#include "QmitkPythonView.h"
#include "PythonPath.h"
#include <ctkAbstractPythonManager.h>

namespace mitk
{

    void PluginActivator::start(ctkPluginContext* context)
    {
      MITK_INFO << "registering python paths";

      QString itkLibDirs(PYTHONPATH_ITK_LIBRARY_DIRS);
      MITK_INFO << "itkLibDirs" << itkLibDirs.toStdString();
      QString itkWrapItkDir(PYTHONPATH_WRAP_ITK_DIR);
      MITK_INFO << "itkWrapItkDir" << itkWrapItkDir.toStdString();

      QString basecommand = "sys.path.append('%1');";
      QString pythonCommand;
      if( ! itkWrapItkDir.isEmpty() )
      {
         pythonCommand = basecommand.arg(itkLibDirs);
         MITK_INFO << "issuing command " << pythonCommand.toStdString();
         GetPythonManager()->executeString(pythonCommand, ctkAbstractPythonManager::SingleInput );

         pythonCommand = basecommand.arg(itkWrapItkDir);
         MITK_INFO << "issuing command " << pythonCommand.toStdString();
         GetPythonManager()->executeString(pythonCommand, ctkAbstractPythonManager::SingleInput );
      }
      else
      {
          MITK_WARN << "No wrapping directory set for WrapITK. ITK Python Wrapping not available!";
      }

      BERRY_REGISTER_EXTENSION_CLASS(QmitkPythonView, context)
    }

    void PluginActivator::stop(ctkPluginContext* context)
    {
        Q_UNUSED(context)
    }

    ctkAbstractPythonManager *PluginActivator::GetPythonManager()
    {
        static ctkAbstractPythonManager PythonManager;
        return &PythonManager;
    }

}

Q_EXPORT_PLUGIN2(org_mitk_gui_qt_python, mitk::PluginActivator)
