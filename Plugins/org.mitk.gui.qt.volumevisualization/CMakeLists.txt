project(org_mitk_gui_qt_volumevisualization)

mitk_create_plugin(
  EXPORT_DIRECTIVE MITK_QT_VOLUMEVISUALIZATION
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt
)
