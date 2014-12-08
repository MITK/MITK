project(org_mitk_gui_qt_remeshing)

include_directories(${CTK_INCLUDE_DIRS})

MACRO_CREATE_MITK_CTK_PLUGIN(
  EXPORT_DIRECTIVE REMESHING_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgets MitkRemeshing
  PACKAGE_DEPENDS Qt4|QtCore Qt5|OpenGL+Xml
)
