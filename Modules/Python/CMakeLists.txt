if(MITK_USE_Python)

set(OpenCV_DEP )
if(MITK_USE_OpenCV)
  set(OpenCV_DEP OpenCV)
endif()

mitk_create_module(
  DEPENDS MitkCore MitkQtWidgets
  #AUTOLOAD_WITH MitkCore
  PACKAGE_DEPENDS
    PUBLIC Qt4|QtGui Qt5|Widgets CTK|CTKScriptingPythonCore+CTKScriptingPythonWidgets
    PRIVATE PythonLibs VTK|vtkPython+vtkWrappingPythonCore ${OpenCV_DEP}
)

if(TARGET ${MODULE_TARGET})
  configure_file(PythonPath.h.in "${CMAKE_CURRENT_BINARY_DIR}/PythonPath.h" @ONLY)
  if(NOT MITK_USE_SYSTEM_PYTHON)
    target_compile_definitions(${MODULE_TARGET} PRIVATE -DUSE_MITK_BUILTIN_PYTHON)
  endif()
endif()

# temporarily disabled, bug-19153
if(UNIX)
  add_subdirectory(Testing)
endif()

endif()
