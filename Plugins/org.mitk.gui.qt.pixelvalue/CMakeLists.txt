project(org_mitk_gui_qt_pixelvalue)

mitk_create_plugin(
  EXPORT_DIRECTIVE MITK_QT_PIXELVALUE
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgets
)
