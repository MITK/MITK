#include "mitkPluginActivator.h"

#include <QtPlugin>

#include "../QmitkQtCommonTestSuite.h"
#include "../api/QmitkMockFunctionality.h"

namespace mitk {

void PluginActivator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(QmitkQtCommonTestSuite, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkMockFunctionality, context)
}

void PluginActivator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

}

Q_EXPORT_PLUGIN2(org_mitk_gui_qt_common.tests, mitk::PluginActivator)
