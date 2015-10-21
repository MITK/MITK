if(MITK_USE_Python)
  set(OpenCV_DEP )
  if(MITK_USE_OpenCV)
    set(OpenCV_DEP OpenCV)
  endif()

  mitk_create_module(
    DEPENDS MitkCore MitkQtWidgets MitkPython
    PACKAGE_DEPENDS
    PUBLIC Qt4|QtGui Qt5|Widgets CTK|CTKScriptingPythonCore+CTKScriptingPythonWidgets
    )

  # temporarily disabled, bug-19153
  if(UNIX)
    add_subdirectory(Testing)
  endif()
endif()
