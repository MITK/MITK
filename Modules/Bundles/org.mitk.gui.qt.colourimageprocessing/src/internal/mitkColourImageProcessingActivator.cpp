#include "mitkColourImageProcessingActivator.h"
#include "QmitkColourImageProcessingView.h"

#include <QtPlugin>

namespace mitk {

void mitkColourImageProcessingActivator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(QmitkColourImageProcessingView, context)
}

void mitkColourImageProcessingActivator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

}

Q_EXPORT_PLUGIN2(org_mitk_qt_colourimageprocessing, mitk::mitkColourImageProcessingActivator)