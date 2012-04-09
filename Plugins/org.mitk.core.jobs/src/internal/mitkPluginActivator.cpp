#include "mitkPluginActivator.h"

#include <QtPlugin>

namespace mitk {

void PluginActivator::start(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

void PluginActivator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

}

Q_EXPORT_PLUGIN2(org_mitk_core_jobs, mitk::PluginActivator)
