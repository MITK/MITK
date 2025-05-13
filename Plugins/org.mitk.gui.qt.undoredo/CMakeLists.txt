project(org_mitk_gui_qt_undoredo)

mitk_create_plugin(
  EXPORT_DIRECTIVE UNDO_REDO_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt
)
