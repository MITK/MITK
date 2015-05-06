project(org_mitk_example_gui_customviewer)

include_directories(${CTK_INCLUDE_DIRS})

mitk_create_plugin(
  EXPORT_DIRECTIVE org_mitk_example_gui_customviewer_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  NO_INSTALL
  )
