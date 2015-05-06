project(org_mitk_gui_qt_geometrytools)

mitk_create_plugin(
  EXPORT_DIRECTIVE GEOMETRYTOOLS_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt
)
