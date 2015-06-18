project(org_mitk_gui_qt_eventrecorder)

mitk_create_plugin(
  EXPORT_DIRECTIVE EVENTRECORDER_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt
)
