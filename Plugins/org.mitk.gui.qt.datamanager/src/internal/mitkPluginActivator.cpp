#include "mitkPluginActivator.h"

#include <QtPlugin>

#include "QmitkPropertyListView.h"
#include "../QmitkDataManagerView.h"
#include "../QmitkDataManagerPreferencePage.h"
#include "../QmitkDataManagerHotkeysPrefPage.h"

namespace mitk {

void PluginActivator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(QmitkDataManagerView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkPropertyListView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkDataManagerPreferencePage, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkDataManagerHotkeysPrefPage, context)
}

void PluginActivator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

}

Q_EXPORT_PLUGIN2(org_mitk_gui_qt_datamanager, mitk::PluginActivator)
