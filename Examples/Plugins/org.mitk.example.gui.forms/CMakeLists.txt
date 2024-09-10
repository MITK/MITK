project(org_mitk_example_gui_forms)

mitk_create_plugin(
  EXPORT_DIRECTIVE FORMS_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkFormsUI
  NO_INSTALL
)
