if(BUILD_TESTING)
  include(mitkFunctionCompileSnippets)

  include_directories(${CTKPluginFramework_INCLUDE_DIRS})

  set(_link_libs CTKPluginFramework)
  mitkFunctionCompileSnippets("${CMAKE_CURRENT_SOURCE_DIR}" ${_link_libs})
endif()
