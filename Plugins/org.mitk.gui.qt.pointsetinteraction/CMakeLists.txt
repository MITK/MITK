project(org_mitk_gui_qt_pointsetinteraction)

mitk_create_plugin(
  EXPORT_DIRECTIVE POINTSETINTERACTION_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt
)
