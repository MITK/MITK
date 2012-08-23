project(org_mitk_example_gui_testapplication)

include_directories(${CTK_INCLUDE_DIRS})

#MITK_INSTALL(PROGRAMS ${VAR_STORESCP})

MACRO_CREATE_MITK_CTK_PLUGIN(
  EXPORT_DIRECTIVE org_mitk_example_gui_testapplication_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  )

#target_link_libraries(${PLUGIN_TARGET} ${CTK_LIBRARIES})