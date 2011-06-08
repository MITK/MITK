#include "mitkPluginActivator.h"
#include "QmitkMITKIGTNavigationToolManagerView.h"

#include <QtPlugin>

namespace mitk {

void PluginActivator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(QmitkMITKIGTNavigationToolManagerView, context)
}

void PluginActivator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

}

Q_EXPORT_PLUGIN2(org.mitk.gui.qt.igtnavigationtoolmanager, mitk::PluginActivator)