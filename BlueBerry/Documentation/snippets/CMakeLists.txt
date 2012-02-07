if(BUILD_TESTING)
  include(FunctionCompileSnippets)

  include_directories(${CTKPluginFramework_INCLUDE_DIRS})

  set(_link_libs CTKPluginFramework)
  FunctionCompileSnippets("${CMAKE_CURRENT_SOURCE_DIR}" ${_link_libs})
endif()
