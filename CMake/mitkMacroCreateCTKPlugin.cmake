macro(MACRO_CREATE_MITK_CTK_PLUGIN)

  MACRO_PARSE_ARGUMENTS(_PLUGIN "EXPORT_DIRECTIVE;EXPORTED_INCLUDE_SUFFIXES;MODULE_DEPENDENCIES;SUBPROJECTS" "TEST_PLUGIN" ${ARGN})

  MITK_CHECK_MODULE(_MODULE_CHECK_RESULT Mitk ${_PLUGIN_MODULE_DEPENDENCIES})
  if(NOT _MODULE_CHECK_RESULT)
    MITK_USE_MODULE(Mitk ${_PLUGIN_MODULE_DEPENDENCIES})
   
    link_directories(${ALL_LIBRARY_DIRS})
    include_directories(${ALL_INCLUDE_DIRECTORIES})

    if(_PLUGIN_TEST_PLUGIN)
      set(is_test_plugin "TEST_PLUGIN")
    else()
      set(is_test_plugin)
    endif()

    set(_mitk_tagfile )

    if(EXISTS ${MITK_DOXYGEN_TAGFILE_NAME})
      # Todo: Point to stable documentations for stable builds
      set(_mitk_tagfile "${MITK_DOXYGEN_TAGFILE_NAME}=http://docs.mitk.org/nightly-qt4/")
    endif()

    MACRO_CREATE_CTK_PLUGIN(EXPORT_DIRECTIVE ${_PLUGIN_EXPORT_DIRECTIVE}
                            EXPORTED_INCLUDE_SUFFIXES ${_PLUGIN_EXPORTED_INCLUDE_SUFFIXES}
                            DOXYGEN_TAGFILES ${_PLUGIN_DOXYGEN_TAGFILES} ${_mitk_tagfile}
                            ${is_test_plugin})

    target_link_libraries(${PLUGIN_TARGET} ${ALL_LIBRARIES})

    if(MITK_DEFAULT_SUBPROJECTS AND NOT MY_SUBPROJECTS)
      set(MY_SUBPROJECTS ${MITK_DEFAULT_SUBPROJECTS})
    endif()

    if(MY_SUBPROJECTS)
      set_property(TARGET ${PLUGIN_TARGET} PROPERTY LABELS ${MY_SUBPROJECTS})
      foreach(subproject ${MY_SUBPROJECTS})
        add_dependencies(${subproject} ${PLUGIN_TARGET})
      endforeach()
    endif()
  else(NOT _MODULE_CHECK_RESULT)
    if(NOT MITK_BUILD_ALL_PLUGINS)
      message(SEND_ERROR "${PLUGIN_TARGET} is missing requirements and won't be built. Missing: ${_MODULE_CHECK_RESULT}")
    else()
      message(STATUS "${PLUGIN_TARGET} is missing requirements and won't be built. Missing: ${_MODULE_CHECK_RESULT}")
    endif()
  endif(NOT _MODULE_CHECK_RESULT)
endmacro()

