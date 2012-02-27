#include "mitkPluginActivator.h"
#include "QmitkMeasurementView.h"
#include "QmitkImageStatisticsView.h"

#include <QtPlugin>

namespace mitk {

void PluginActivator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(QmitkMeasurementView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkImageStatisticsView, context)
}

void PluginActivator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

}

Q_EXPORT_PLUGIN2(org.mitk.gui.qt.measurementtoolbox, mitk::PluginActivator)