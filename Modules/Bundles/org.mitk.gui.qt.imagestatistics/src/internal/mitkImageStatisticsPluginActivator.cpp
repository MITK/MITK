#include "mitkImageStatisticsPluginActivator.h"

#include <QtPlugin>

#include "QmitkImageStatisticsView.h"

namespace mitk {

  void ImageStatisticsPluginActivator::start(ctkPluginContext* context)
  {
    BERRY_REGISTER_EXTENSION_CLASS(QmitkImageStatistics, context)
  }

  void ImageStatisticsPluginActivator::stop(ctkPluginContext* context)
  {
    Q_UNUSED(context)
  }

}

Q_EXPORT_PLUGIN2(org_mitk_gui_qt_imagestatistics, mitk::ImageStatisticsPluginActivator)