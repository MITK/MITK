#include "mitkPluginActivator.h"

#include <QtPlugin>

#include "QmitkUGVisualizationView.h"

namespace mitk {

void PluginActivator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(QmitkUGVisualizationView, context)
}

void PluginActivator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

}

Q_EXPORT_PLUGIN2(org_mitk_gui_qt_ugvisualization, mitk::PluginActivator)
