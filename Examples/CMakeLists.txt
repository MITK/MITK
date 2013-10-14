
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

  include("${CMAKE_CURRENT_SOURCE_DIR}/Plugins/PluginList.cmake")
  set(mitk_example_plugins_fullpath )
  foreach(mitk_example_plugin ${MITK_EXAMPLE_PLUGINS})
    list(APPEND mitk_example_plugins_fullpath Plugins/${mitk_example_plugin})
  endforeach()

  ctkMacroSetupPlugins(${mitk_example_plugins_fullpath}
                       BUILD_OPTION_PREFIX MITK_BUILD_
                       COMPACT_OPTIONS)

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
  MbiLog
  QtFreeRender
#  Overlays# depends on QmitkExt..
)

if(MITK_USE_QT)
  # some examples depend on Qt 4
  if (MITK_USE_Qt4)
    list(APPEND _example_dirs
      Tutorial
      QtAppExample
    )
  endif()
  # some examples depend on Qt 5
  if (MITK_USE_Qt5)
    list(APPEND _example_dirs
      QuickRender
      )
    endif()
endif()

if(MITK_USE_BLUEBERRY)
  list(APPEND _example_dirs
    BlueBerryExampleLauncher
  )
endif()

foreach(_example_dir ${_example_dirs})
  add_subdirectory(${_example_dir})
endforeach()
