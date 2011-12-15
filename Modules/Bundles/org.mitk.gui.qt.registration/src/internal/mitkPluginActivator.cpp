#include "mitkPluginActivator.h"

#include "src/internal/QmitkDeformableRegistrationView.h"
#include "src/internal/QmitkPointBasedRegistrationView.h"
#include "src/internal/QmitkRigidRegistrationView.h"

#include <QtPlugin>

namespace mitk {

void PluginActivator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(QmitkDeformableRegistrationView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkPointBasedRegistrationView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkRigidRegistrationView, context)
  //Q_UNUSED(context)
}

void PluginActivator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

}

Q_EXPORT_PLUGIN2(org_mitk_gui_qt_registration, mitk::PluginActivator)
