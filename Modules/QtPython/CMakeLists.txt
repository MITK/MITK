if(MITK_USE_Python3)
  mitkFunctionCheckCompilerFlags("/wd4273" CMAKE_CXX_FLAGS)

  mitk_create_module(
    DEPENDS MitkCore MitkQtWidgets MitkPython
    PACKAGE_DEPENDS
    PUBLIC Qt5|Widgets CTK|CTKScriptingPythonCore+CTKScriptingPythonWidgets
    )

  if(BUILD_TESTING)
    add_subdirectory(Testing)
  endif()
endif()
