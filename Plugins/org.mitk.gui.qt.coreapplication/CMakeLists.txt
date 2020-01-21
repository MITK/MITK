project(org_mitk_gui_qt_coreapplication)

mitk_create_plugin(
  EXPORT_DIRECTIVE MITK_QT_COREAPP
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgets
  SUBPROJECTS MITK-CoreUI
  )
