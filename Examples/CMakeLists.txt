
set(MITK_DEFAULT_SUBPROJECTS MITK-Examples)

#-----------------------------------------------------------------------------
# Set-up example plugins
#-----------------------------------------------------------------------------

if(MITK_USE_BLUEBERRY)

  # Specify which plug-ins belong to this project
  macro(GetMyTargetLibraries all_target_libraries varname)
    set(re_ctkplugin_mitk "^org_mitk_example_[a-zA-Z0-9_]+$")
    set(_tmp_list)
    list(APPEND _tmp_list ${all_target_libraries})
    ctkMacroListFilter(_tmp_list re_ctkplugin_mitk OUTPUT_VARIABLE ${varname})
  endmacro()

  set(MITK_EXAMPLE_PLUGIN_TARGETS )
  foreach(mitk_example_plugin ${MITK_EXAMPLE_PLUGINS})
    ctkFunctionExtractOptionNameAndValue(${mitk_example_plugin} plugin_name plugin_value)
    string(REPLACE "." "_" _plugin_target ${plugin_name})
    list(APPEND MITK_EXAMPLE_PLUGIN_TARGETS ${_plugin_target})
    mark_as_advanced(${${_plugin_target}_option_name})
  endforeach()

endif()

#-----------------------------------------------------------------------------
# Add example executables
#-----------------------------------------------------------------------------

set(MITK_DIR ${PROJECT_BINARY_DIR})
set(MITK_EXPORTS_FILE_INCLUDED 1)

set(_example_dirs
  Dump
  MbiLog
  QtFreeRender
  Tutorial
#  Overlays# depends on MitkQtWidgetsExt..
)

# some examples depend on Qt 5
if (MITK_USE_Qt5)
  list(APPEND _example_dirs
    QuickRender
    )
elseif (MITK_USE_Qt4)
  list(APPEND _example_dirs
    QtAppExample
    DumpDir
    )
endif()

if(MITK_USE_BLUEBERRY)
  list(APPEND _example_dirs
    BlueBerryExampleLauncher
  )
endif()

foreach(_example_dir ${_example_dirs})
  add_subdirectory(${_example_dir})
endforeach()
