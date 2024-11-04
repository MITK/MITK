if(MITK_WRAP_PYTHON_ENABLED)
  mitkFunctionCheckCompilerFlags("-Wno-cpp" CMAKE_CXX_FLAGS)

  mitk_create_module(PythonService
    DEPENDS PRIVATE MitkPython
    AUTOLOAD_WITH MitkCore
  )

endif()
