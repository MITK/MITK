#include "mitkBasicImageProcessingActivator.h"
#include "QmitkBasicImageProcessingView.h"

#include <QtPlugin>

namespace mitk {

void BasicImageProcessingActivator::start(ctkPluginContext* context)
{
   BERRY_REGISTER_EXTENSION_CLASS(QmitkBasicImageProcessing, context)
}

void BasicImageProcessingActivator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

}

Q_EXPORT_PLUGIN2(org_mitk_gui_qt_basicimageprocessing, mitk::BasicImageProcessingActivator)