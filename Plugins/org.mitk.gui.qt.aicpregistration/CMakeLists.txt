project(org_mitk_gui_qt_aicpregistration)

mitk_create_plugin(
  EXPORT_DIRECTIVE AICPREGISTRATION_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt MitkAlgorithmsExt
)
