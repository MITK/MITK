#include "mitkPluginActivator.h"

#include "QmitkPointBasedRegistrationView.h"

#include <QtPlugin>

namespace mitk {

void PluginActivator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(QmitkPointBasedRegistrationView, context)
  Q_UNUSED(context)
}

void PluginActivator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

}

Q_EXPORT_PLUGIN2(org_mitk_gui_qt_pointbasedregistration, mitk::PluginActivator)
