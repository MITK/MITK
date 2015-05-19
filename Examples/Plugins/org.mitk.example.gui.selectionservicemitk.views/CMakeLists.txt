project(org_mitk_example_gui_selectionservicemitk_views)

mitk_create_plugin(
  EXPORT_DIRECTIVE org_mitk_example_gui_selectionservicemitk_views_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgets
  NO_INSTALL
  )
