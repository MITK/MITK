#include "mitkMaterialEditorPluginActivator.h"

#include "QmitkMITKSurfaceMaterialEditorView.h"

#include <QtPlugin>

namespace mitk {

  void MaterialEditorPluginActivator::start(ctkPluginContext* context)
  {
    BERRY_REGISTER_EXTENSION_CLASS(QmitkMITKSurfaceMaterialEditorView, context)
  }

  void MaterialEditorPluginActivator::stop(ctkPluginContext* context)
  {
    Q_UNUSED(context)
  }

}

Q_EXPORT_PLUGIN2(org_mitk_gui_qt_materialeditor, mitk::MaterialEditorPluginActivator)