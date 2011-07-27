#include "mitkPluginActivator.h"
#include "QmitkImageGuidedTherapyTutorialView.h"

#include <QtPlugin>

namespace mitk {

void PluginActivator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(QmitkImageGuidedTherapyTutorialView, context)
}

void PluginActivator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

}

Q_EXPORT_PLUGIN2(org_mitk_gui_qt_igttutorial, mitk::PluginActivator)