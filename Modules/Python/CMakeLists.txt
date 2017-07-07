if(MITK_USE_Python)
  set(OpenCV_DEP )
  if(MITK_USE_OpenCV)
    set(OpenCV_DEP OpenCV)
  endif()

  mitk_create_module(
    DEPENDS MitkCore
    PACKAGE_DEPENDS PUBLIC PythonLibs
    WARNINGS_AS_ERRORS
    )

  add_subdirectory(autoload/PythonService)
endif()
