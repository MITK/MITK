project(org_mitk_example_gui_pcaexample)

mitk_create_plugin(
  EXPORT_DIRECTIVE PCAEXAMPLE_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt
)
