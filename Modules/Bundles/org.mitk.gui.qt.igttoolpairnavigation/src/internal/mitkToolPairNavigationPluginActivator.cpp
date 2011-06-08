#include "mitkToolPairNavigationPluginActivator.h"

#include <QtPlugin>

#include "QmitkToolPairNavigationView.h"

namespace mitk {

void ToolPairNavigationPluginActivator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(QmitkToolPairNavigationView, context)

}

void ToolPairNavigationPluginActivator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

}

Q_EXPORT_PLUGIN2(org_mitk_gui_qt_igttoolpairnavigation, mitk::ToolPairNavigationPluginActivator)
