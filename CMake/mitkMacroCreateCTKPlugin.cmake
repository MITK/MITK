macro(MACRO_CREATE_MITK_CTK_PLUGIN)

  MACRO_PARSE_ARGUMENTS(_PLUGIN "EXPORT_DIRECTIVE;EXPORTED_INCLUDE_SUFFIXES;MODULE_DEPENDENCIES;SUBPROJECTS" "TEST_PLUGIN;NO_INSTALL" ${ARGN})

  set(MODULE_QT4_MODULES ${PLUGIN_QT4_MODULES})
  if (MITK_USE_Qt4 AND MODULE_QT4_MODULES)
    list(APPEND _PLUGIN_MODULE_DEPENDENCIES Qt4)
  endif()
  set(MODULE_QT5_MODULES ${PLUGIN_QT5_MODULES})
  if (MITK_USE_Qt5 AND MODULE_QT5_MODULES)
    list(APPEND _PLUGIN_MODULE_DEPENDENCIES Qt5)
  endif()

  MITK_CHECK_MODULE(_MODULE_CHECK_RESULT Mitk ${_PLUGIN_MODULE_DEPENDENCIES})
  if(NOT _MODULE_CHECK_RESULT)

    MITK_USE_MODULE(Mitk ${_PLUGIN_MODULE_DEPENDENCIES})

    link_directories(${ALL_LIBRARY_DIRS})
    include_directories(${ALL_INCLUDE_DIRECTORIES})

    if(_PLUGIN_TEST_PLUGIN)
      set(is_test_plugin "TEST_PLUGIN")
      set(_PLUGIN_NO_INSTALL 1)
    else()
      set(is_test_plugin)
    endif()

    if(_PLUGIN_NO_INSTALL)
      set(plugin_no_install "NO_INSTALL")
    else()
      set(plugin_no_install)
    endif()

    MACRO_CREATE_CTK_PLUGIN(EXPORT_DIRECTIVE ${_PLUGIN_EXPORT_DIRECTIVE}
                            EXPORTED_INCLUDE_SUFFIXES ${_PLUGIN_EXPORTED_INCLUDE_SUFFIXES}
                            DOXYGEN_TAGFILES ${_PLUGIN_DOXYGEN_TAGFILES}
                            MOC_OPTIONS -DBOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED
                            ${is_test_plugin} ${plugin_no_install})

    target_link_libraries(${PLUGIN_TARGET} ${ALL_LIBRARIES})

    if(ALL_META_DEPENDENCIES)
      add_dependencies(${PLUGIN_TARGET} ${ALL_META_DEPENDENCIES})
    endif()

    if(MITK_DEFAULT_SUBPROJECTS AND NOT MY_SUBPROJECTS)
      set(MY_SUBPROJECTS ${MITK_DEFAULT_SUBPROJECTS})
    endif()

    if(MY_SUBPROJECTS)
      set_property(TARGET ${PLUGIN_TARGET} PROPERTY LABELS ${MY_SUBPROJECTS})
      foreach(subproject ${MY_SUBPROJECTS})
        add_dependencies(${subproject} ${PLUGIN_TARGET})
      endforeach()
    endif()

    #------------------------------------------------------------#
    #------------------ Installer support -----------------------#
    if(NOT _PLUGIN_NO_INSTALL)

      set(_autoload_targets )
      foreach(_dependency ${ALL_DEPENDENCIES})
        get_target_property(_dep_autoloads ${_dependency} MITK_AUTOLOAD_TARGETS)
        if (_dep_autoloads)
          list(APPEND _autoload_targets ${_dep_autoloads})
        endif()
      endforeach()

      # The MITK_AUTOLOAD_TARGETS property is used in the mitkFunctionInstallAutoLoadModules
      # macro which expects a list of plug-in targets.
      if (_autoload_targets)
        list(REMOVE_DUPLICATES _autoload_targets)
        set_target_properties(${PLUGIN_TARGET} PROPERTIES MITK_AUTOLOAD_TARGETS "${_autoload_targets}")
      endif()

    endif()

  else(NOT _MODULE_CHECK_RESULT)
    if(NOT MITK_BUILD_ALL_PLUGINS)
      message(SEND_ERROR "${PROJECT_NAME} is missing requirements and won't be built. Missing: ${_MODULE_CHECK_RESULT}")
    else()
      message(STATUS "${PROJECT_NAME} is missing requirements and won't be built. Missing: ${_MODULE_CHECK_RESULT}")
    endif()
  endif(NOT _MODULE_CHECK_RESULT)
endmacro()

