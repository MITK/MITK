if( MITK_USE_Python )
  if(NOT MITK_USE_SYSTEM_PYTHON)
    add_definitions( -DUSE_MITK_BUILTIN_PYTHON )
    set(NUMPY_INCLUDE_DIR "${Numpy_DIR}/core/include")
  endif()

  MESSAGE("numpy: " ${Numpy_DIR})

  set(OpenCV_DEP )
  if(MITK_USE_OpenCV)
    set(OpenCV_DEP OpenCV)
  endif()

  MITK_CREATE_MODULE(
    DEPENDS MitkCore
    EXPORT_DEFINE MITK_PYTHON_EXPORT
    PACKAGE_DEPENDS Qt4|QtGui CTK|CTKScriptingPythonCore+CTKScriptingPythonWidgets PythonLibs VTK|vtkPython+vtkWrappingPythonCore SimpleITK ${OpenCV_DEP}
    INCLUDE_DIRS ${NUMPY_INCLUDE_DIR}
  )
  configure_file(PythonPath.h.in "${CMAKE_CURRENT_BINARY_DIR}/PythonPath.h" @ONLY)

  add_subdirectory(Testing)
endif()
