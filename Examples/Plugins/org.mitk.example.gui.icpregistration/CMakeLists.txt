project(org_mitk_example_gui_icpregistration)

mitk_create_plugin(
  EXPORT_DIRECTIVE ICPREGISTRATION_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt
)
