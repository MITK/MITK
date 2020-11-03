project(org_mitk_gui_qt_dosevisualization)

mitk_create_plugin(
  EXPORT_DIRECTIVE RTDOSEVISUALIZATION_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt MitkRT MitkRTUI
)
