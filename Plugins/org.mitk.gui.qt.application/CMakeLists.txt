project(org_mitk_gui_qt_application)

mitk_create_plugin(
  EXPORT_DIRECTIVE MITK_QT_APP
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgets
  PACKAGE_DEPENDS Qt4|QtCore Qt5|OpenGL+Xml
  SUBPROJECTS MITK-CoreUI
  )
