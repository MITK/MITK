#include "mitkNavigationDataPlayerPluginActivator.h"

#include <QtPlugin>

#include "QmitkNavigationDataPlayerView.h"

namespace mitk {

void NavigationDataPlayerPluginActivator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(QmitkNavigationDataPlayerView, context)

}

void NavigationDataPlayerPluginActivator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

}

Q_EXPORT_PLUGIN2(org_mitk_gui_qt_navigationdataplayer, mitk::NavigationDataPlayerPluginActivator)
