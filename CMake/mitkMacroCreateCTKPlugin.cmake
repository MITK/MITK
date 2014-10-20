macro(MACRO_CREATE_MITK_CTK_PLUGIN)

  MACRO_PARSE_ARGUMENTS(_PLUGIN "EXPORT_DIRECTIVE;EXPORTED_INCLUDE_SUFFIXES;MODULE_DEPENDENCIES;MODULE_DEPENDS;PACKAGE_DEPENDS;SUBPROJECTS" "TEST_PLUGIN;NO_INSTALL" ${ARGN})

  mitk_check_module_dependencies(MODULES MitkCore ${_PLUGIN_MODULE_DEPENDENCIES} ${_PLUGIN_MODULE_DEPENDS}
                                 PACKAGES ${_PLUGIN_PACKAGE_DEPENDS}
                                 MISSING_DEPENDENCIES_VAR _missing_deps
                                 MODULE_DEPENDENCIES_VAR _module_deps
                                 PACKAGE_DEPENDENCIES_VAR _package_deps)

  if(_PLUGIN_MODULE_DEPENDENCIES)
    message(WARNING "The MODULE_DEPENDENCIES argument is deprecated since 2014.03. Please use MODULE_DEPENDS instead.")
  endif()

  if(NOT _missing_deps)

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

    _link_directories_for_packages(${_package_deps})

    MACRO_CREATE_CTK_PLUGIN(EXPORT_DIRECTIVE ${_PLUGIN_EXPORT_DIRECTIVE}
                            EXPORTED_INCLUDE_SUFFIXES ${_PLUGIN_EXPORTED_INCLUDE_SUFFIXES}
                            DOXYGEN_TAGFILES ${_PLUGIN_DOXYGEN_TAGFILES}
                            MOC_OPTIONS -DBOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED
                            ${is_test_plugin} ${plugin_no_install})

    mitk_use_modules(TARGET ${PLUGIN_TARGET}
      MODULES MitkCore ${_PLUGIN_MODULE_DEPENDENCIES} ${_PLUGIN_MODULE_DEPENDS}
      PACKAGES ${_PLUGIN_PACKAGE_DEPENDS}
     )

    set_property(TARGET ${PLUGIN_TARGET} APPEND PROPERTY COMPILE_DEFINITIONS US_MODULE_NAME=${PLUGIN_TARGET})
    set_property(TARGET ${PLUGIN_TARGET} PROPERTY US_MODULE_NAME ${PLUGIN_TARGET})

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
      foreach(_dependency ${_module_deps})
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

  else()
    if(NOT MITK_BUILD_ALL_PLUGINS)
      message(SEND_ERROR "${PROJECT_NAME} is missing requirements and won't be built. Missing: ${_missing_deps}")
    else()
      message(STATUS "${PROJECT_NAME} is missing requirements and won't be built. Missing: ${_missing_deps}")
    endif()
  endif()
endmacro()

