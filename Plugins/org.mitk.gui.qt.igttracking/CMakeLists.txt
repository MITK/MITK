project(org_mitk_gui_qt_igttracking)

mitk_create_plugin(
  EXPORT_DIRECTIVE IGTTRACKING_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt MitkMapperExt MitkIGT MitkIGTUI
)
