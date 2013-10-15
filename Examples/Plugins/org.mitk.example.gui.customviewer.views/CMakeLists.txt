project(org_mitk_example_gui_customviewer_views)

MACRO_CREATE_MITK_CTK_PLUGIN(
  EXPORT_DIRECTIVE org_mitk_example_gui_customviewer_views_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDENCIES QmitkExt mitkDicomUI
  NO_INSTALL
  )
