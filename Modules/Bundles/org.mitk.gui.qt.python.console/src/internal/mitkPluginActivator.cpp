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
