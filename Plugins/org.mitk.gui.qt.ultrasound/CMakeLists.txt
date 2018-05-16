project(org_mitk_gui_qt_ultrasound)

mitk_create_plugin(
  EXPORT_DIRECTIVE ULTRASOUND_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt MitkUS MitkUSUI
)
