if(MITK_WRAP_PYTHON_ENABLED)
  mitkFunctionCheckCompilerFlags("-Wno-cpp" CMAKE_CXX_FLAGS)
  mitk_create_module(
    DEPENDS MitkCore
    PACKAGE_DEPENDS PUBLIC Python3|Python
  )

  #if(TARGET ${MODULE_TARGET} AND BUILD_TESTING)
  #  add_subdirectory(test)
  #endif()
  if(TARGET ${MODULE_TARGET})
  configure_file(PythonPath.h.in "${CMAKE_CURRENT_BINARY_DIR}/PythonPath.h" @ONLY)
  endif()
endif()
