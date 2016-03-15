MITK_CREATE_MODULE( PythonService
  INCLUDE_DIRS
    PRIVATE src/PythonService
  DEPENDS PUBLIC MitkPython
  PACKAGE_DEPENDS
    PUBLIC Qt5|Widgets CTK|CTKScriptingPythonCore+CTKScriptingPythonWidgets
    PRIVATE VTK|vtkPython+vtkWrappingPythonCore ${OpenCV_DEP}
    AUTOLOAD_WITH MitkPython
)

if(TARGET ${MODULE_TARGET})
  configure_file(PythonPath.h.in "${CMAKE_CURRENT_BINARY_DIR}/PythonPath.h" @ONLY)
  if(NOT MITK_USE_SYSTEM_PYTHON)
    target_compile_definitions(${MODULE_TARGET} PRIVATE -DUSE_MITK_BUILTIN_PYTHON)
  endif()
endif()
