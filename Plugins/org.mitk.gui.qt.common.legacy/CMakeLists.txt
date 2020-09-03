project(org_mitk_gui_qt_common_legacy)

mitk_create_plugin(
  EXPORT_DIRECTIVE MITK_QT_COMMON_LEGACY
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgets
  )
