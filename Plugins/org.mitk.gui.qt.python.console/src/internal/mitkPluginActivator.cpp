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

#include "QmitkPythonConsoleView.h"
#include "QmitkPythonVariableStack.h"
#include "QmitkPythonCommandHistory.h"
#include "QmitkPythonEditor.h"
#include "QmitkPythonSnippets.h"
#include "QmitkPythonPerspective.h"

#include <QtPlugin>

namespace mitk {

  void PluginActivator::start(ctkPluginContext* context)
  {
    BERRY_REGISTER_EXTENSION_CLASS(QmitkPythonConsoleView, context)
    BERRY_REGISTER_EXTENSION_CLASS(QmitkPythonVariableStack, context)
    BERRY_REGISTER_EXTENSION_CLASS(QmitkPythonCommandHistory, context)
    BERRY_REGISTER_EXTENSION_CLASS(QmitkPythonEditor, context)
    BERRY_REGISTER_EXTENSION_CLASS(QmitkPythonSnippets, context)
    BERRY_REGISTER_EXTENSION_CLASS(QmitkPythonPerspective, context)
  }

  void PluginActivator::stop(ctkPluginContext* context)
  {
    Q_UNUSED(context)
  }

}

Q_EXPORT_PLUGIN2(org_mitk_gui_qt_imagenavigator, mitk::PluginActivator)
