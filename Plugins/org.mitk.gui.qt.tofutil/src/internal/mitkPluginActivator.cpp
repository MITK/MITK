#include "mitkPluginActivator.h"

#include <QtPlugin>
#include "QmitkToFUtilView.h"

namespace mitk {

void PluginActivator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(QmitkToFUtilView, context)
}

void PluginActivator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

}

Q_EXPORT_PLUGIN2(org_mitk_gui_qt_tofutil, mitk::PluginActivator)