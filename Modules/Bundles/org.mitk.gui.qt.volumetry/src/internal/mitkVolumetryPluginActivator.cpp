#include "mitkVolumetryPluginActivator.h"

#include "QmitkVolumetryView.h"

#include <QtPlugin>

namespace mitk {

  void VolumetryPluginActivator::start(ctkPluginContext* context)
  {
    BERRY_REGISTER_EXTENSION_CLASS(QmitkVolumetryView, context)
  }

  void VolumetryPluginActivator::stop(ctkPluginContext* context)
  {
    Q_UNUSED(context)
  }

}

Q_EXPORT_PLUGIN2(org_mitk_gui_qt_volumetry, mitk::VolumetryPluginActivator)