#include "org_mitk_gui_qt_undoredo_Activator.h"
#include "QmitkUndoRedoView.h"
#include "QmitkUndoRedoPreferencePage.h"

void org_mitk_gui_qt_undoredo_Activator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(QmitkUndoRedoView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkUndoRedoPreferencePage, context)
}

void org_mitk_gui_qt_undoredo_Activator::stop(ctkPluginContext*)
{
}
