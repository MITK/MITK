#include "org_mitk_gui_qt_undoredo_Activator.h"
#include "QmitkUndoRedoView.h"

void org_mitk_gui_qt_undoredo_Activator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(QmitkUndoRedoView, context)
}

void org_mitk_gui_qt_undoredo_Activator::stop(ctkPluginContext*)
{
}
