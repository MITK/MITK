project(org_mitk_gui_qt_rdftriplestore)

mitk_create_plugin(
  EXPORT_DIRECTIVE RDFTRIPLESTORE_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkRDF
  PACKAGE_DEPENDS Qt4|QtCore Qt5|OpenGL
)
