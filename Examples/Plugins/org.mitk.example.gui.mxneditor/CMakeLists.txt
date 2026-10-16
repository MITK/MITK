project(org_mitk_example_gui_mxneditor)

mitk_create_plugin(
  EXPORT_DIRECTIVE MXNEDITOREXAMPLE_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgets
  NO_INSTALL
)
