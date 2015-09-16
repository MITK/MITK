project(org_mitk_gui_qt_spectrocamrecorder)

mitk_create_plugin(
  EXPORT_DIRECTIVE SPECTROCAMRECORDER_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt MitkSpectroCam
)
