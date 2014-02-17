if( MITK_USE_Python )

  MITK_CREATE_MODULE(mitkPython
    DEPENDS Mitk
    EXPORT_DEFINE MITK_PYTHON_EXPORT
    PACKAGE_DEPENDS Qt4|QtGui CTK PythonLibs
  )
  configure_file(PythonPath.h.in
    "${CMAKE_CURRENT_BINARY_DIR}/PythonPath.h" @ONLY)

  add_subdirectory(Testing)
endif()
