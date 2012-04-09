#include "mitkImageCropperPluginActivator.h"
#include "QmitkImageCropper.h"

#include <QtPlugin>

namespace mitk {

void ImageCropperPluginActivator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS( QmitkImageCropper, context )
}

void ImageCropperPluginActivator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

}

Q_EXPORT_PLUGIN2(org_mitk_gui_qt_imagecropper, mitk::ImageCropperPluginActivator)