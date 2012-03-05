#include "mitkImageNavigatorPluginActivator.h"

#include "QmitkImageNavigatorView.h"

#include <QtPlugin>

namespace mitk {

  void ImageNavigatorPluginActivator::start(ctkPluginContext* context)
  {
    BERRY_REGISTER_EXTENSION_CLASS(QmitkImageNavigatorView, context)
  }

  void ImageNavigatorPluginActivator::stop(ctkPluginContext* context)
  {
    Q_UNUSED(context)
  }

}

Q_EXPORT_PLUGIN2(org_mitk_gui_qt_imagenavigator, mitk::ImageNavigatorPluginActivator)