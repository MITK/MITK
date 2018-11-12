project(org_mitk_gui_qt_application)

mitk_create_plugin(
  EXPORT_DIRECTIVE MITK_QT_APP
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgets MitkQtWidgetsExt
  PACKAGE_DEPENDS Qt5|OpenGL+Xml
  SUBPROJECTS MITK-CoreUI
  )
