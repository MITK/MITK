project(org_mitk_example_gui_customviewer)

include_directories(${CTK_INCLUDE_DIRS})

#MITK_INSTALL(PROGRAMS ${VAR_STORESCP})

MACRO_CREATE_MITK_CTK_PLUGIN(
  EXPORT_DIRECTIVE org_mitk_example_gui_customviewer_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  #MODULE_DEPENDENCIES org.mitk.gui.qt.datamanager
  )

#target_link_libraries(${PLUGIN_TARGET} ${CTK_LIBRARIES})