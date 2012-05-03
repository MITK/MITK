#include "mitkPluginActivator.h"

#include <QtPlugin>

#include "QmitkMeshDecimationView.h"

namespace mitk {

void PluginActivator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(QmitkMeshDecimationView, context)
  //BERRY_REGISTER_EXTENSION_CLASS(QmitkHexVolumeMesherView, context)
}

void PluginActivator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

}

Q_EXPORT_PLUGIN2(org_mbi_gui_qt_meshdecimation, mitk::PluginActivator)
