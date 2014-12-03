project(BlueBerryExampleLauncher)

set(_source_files
  BlueBerryExampleLauncher.cpp
  BlueBerryExampleLauncherDialog.cpp
)

set(_source_moc_h_files
  BlueBerryExampleLauncherDialog.h
)

set(_source_ui_files
  BlueBerryExampleLauncherDialog.ui
)

# this is a workaround for Visual Studio. The relative include paths in the generated
# moc files can get very long and can't be resolved by the MSVC compiler.
foreach(_moc_src ${_source_moc_h_files})
  if (DESIRED_QT_VERSION MATCHES "5")
    qt5_wrap_cpp(_source_files ${_moc_src} OPTIONS -f${_moc_src})
  else()
    qt4_wrap_cpp(_source_files ${_moc_src} OPTIONS -f${_moc_src})
  endif()
endforeach()

if (DESIRED_QT_VERSION MATCHES "5")
  qt5_wrap_ui(_source_files ${_source_ui_files})
else()
  qt4_wrap_ui(_source_files ${_source_ui_files})
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                   )

#-----------------------------------------------------------------------------
# Create provisioning files
#-----------------------------------------------------------------------------

set(_plugins_for_all_demos
  org.blueberry.compat
)

file(GLOB _demo_configurations Configurations/*.cmake)

set(ALL_REQUIRED_PLUGINS ${_plugins_for_all_demos})
foreach(_demo_config_file ${_demo_configurations})
  set(REQUIRED_PLUGINS )
  set(DESCRIPTION )
  include(${_demo_config_file})
  get_filename_component(_name ${_demo_config_file} NAME_WE)
  FunctionCreateProvisioningFile(FILE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}_${_name}.provisioning
                                 PLUGINS ${REQUIRED_PLUGINS} ${_plugins_for_all_demos}
                                 NO_INSTALL
                                )
  if(DESCRIPTION)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/BlueBerryExampleDescription.txt
                   ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}_${_name}.txt @ONLY)
  endif()
  list(APPEND ALL_REQUIRED_PLUGINS ${REQUIRED_PLUGINS})
endforeach()

list(REMOVE_DUPLICATES ALL_REQUIRED_PLUGINS)
set(ALL_REQUIRED_PLUGIN_TARGETS )
foreach(req_plugin ${ALL_REQUIRED_PLUGINS})
  string(REPLACE "." "_" _plugin_target ${req_plugin})
  if(TARGET ${_plugin_target})
    list(APPEND ALL_REQUIRED_PLUGIN_TARGETS ${_plugin_target})
  endif()
endforeach()

#-----------------------------------------------------------------------------
# Create the example launcher
#-----------------------------------------------------------------------------

FunctionCreateBlueBerryApplication(
  NAME ${PROJECT_NAME}
  DESCRIPTION "MITK Application Framework Demo"
  SOURCES ${_source_files}
#  PLUGINS ${ALL_REQUIRED_PLUGIN_TARGETS}
  NO_PROVISIONING
  NO_INSTALL
  SHOW_CONSOLE
)
mitk_use_modules(TARGET ${PROJECT_NAME} PACKAGES Qt4|QtGui Qt5|Widgets)

add_dependencies(${PROJECT_NAME} ${ALL_REQUIRED_PLUGIN_TARGETS})

# subproject support
set_property(TARGET ${PROJECT_NAME} PROPERTY LABELS ${MITK_DEFAULT_SUBPROJECTS})
foreach(subproject ${MITK_DEFAULT_SUBPROJECTS})
  add_dependencies(${subproject} ${PROJECT_NAME})
endforeach()
