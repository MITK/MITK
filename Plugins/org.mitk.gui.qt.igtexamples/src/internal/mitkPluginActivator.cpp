#include "mitkPluginActivator.h"
#include "QmitkIGTTutorialView.h"
#include "QmitkIGTTrackingLabView.h"

#include <QtPlugin>

namespace mitk {

void PluginActivator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(QmitkIGTTutorialView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkIGTTrackingLabView, context)
}

void PluginActivator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

}

Q_EXPORT_PLUGIN2(org_mitk_gui_qt_igtexamples, mitk::PluginActivator)
