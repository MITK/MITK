#include "mitkPluginActivator.h"

#include <QtPlugin>
#include "QmitkToFTutorialView.h"

namespace mitk {

void PluginActivator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(QmitkToFTutorialView, context)
}

void PluginActivator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

}

Q_EXPORT_PLUGIN2(org_mitk_gui_qt_toftutorial, mitk::PluginActivator)