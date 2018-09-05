mitkFunctionCheckCompilerFlags("-Wno-cpp" CMAKE_CXX_FLAGS)

MITK_CREATE_MODULE( PythonService
  INCLUDE_DIRS
    PRIVATE src/PythonService
  DEPENDS PUBLIC MitkPython
  PACKAGE_DEPENDS
    PUBLIC Qt5|Widgets CTK|CTKScriptingPythonCore+CTKScriptingPythonWidgets
    AUTOLOAD_WITH MitkPython
)

if(TARGET ${MODULE_TARGET})
  configure_file(PythonPath.h.in "${CMAKE_CURRENT_BINARY_DIR}/PythonPath.h" @ONLY)
endif()
