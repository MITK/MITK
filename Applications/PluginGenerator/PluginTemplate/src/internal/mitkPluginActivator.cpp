$(license)

#include "mitkPluginActivator.h"

#include <QtPlugin>

#include "$(view-file-name).h"

namespace mitk {

void PluginActivator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS($(view-class-name), context)
}

void PluginActivator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

}

Q_EXPORT_PLUGIN2($(plugin-target), mitk::PluginActivator)
