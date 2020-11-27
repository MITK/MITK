mitkFunctionCheckCompilerFlags("-Wno-cpp" CMAKE_CXX_FLAGS)

mitk_create_module(PythonService
  INCLUDE_DIRS
    PRIVATE src/PythonService
  DEPENDS PUBLIC MitkPython
  PACKAGE_DEPENDS
    PUBLIC Qt5|Widgets CTK|CTKScriptingPythonCore+CTKScriptingPythonWidgets
    PRIVATE Python3|NumPy
  AUTOLOAD_WITH MitkPython
)

if(TARGET ${MODULE_TARGET})
  configure_file(PythonPath.h.in "${CMAKE_CURRENT_BINARY_DIR}/PythonPath.h" @ONLY)
endif()
