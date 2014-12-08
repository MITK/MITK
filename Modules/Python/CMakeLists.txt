if( MITK_USE_Python )
  if(NOT MITK_USE_SYSTEM_PYTHON)
    add_definitions( -DUSE_MITK_BUILTIN_PYTHON )
  endif()

  set(OpenCV_DEP )
  if(MITK_USE_OpenCV)
    set(OpenCV_DEP OpenCV)
  endif()

  # workaround until testing has a package depends
  set(SimpleITK_DEP )
  if(BUILD_TESTING AND NOT APPLE)
    set(SimpleITK_DEP SimpleITK)
  endif()


  MITK_CREATE_MODULE(
    DEPENDS MitkCore MitkQtWidgets
    EXPORT_DEFINE MITK_PYTHON_EXPORT
    PACKAGE_DEPENDS Qt4|QtGui Qt5|Widgets CTK|CTKScriptingPythonCore+CTKScriptingPythonWidgets PythonLibs VTK|vtkPython+vtkWrappingPythonCore Numpy ${SimpleITK_DEP} ${OpenCV_DEP}
  )
  configure_file(PythonPath.h.in "${CMAKE_CURRENT_BINARY_DIR}/PythonPath.h" @ONLY)

  add_subdirectory(Testing)
endif()
