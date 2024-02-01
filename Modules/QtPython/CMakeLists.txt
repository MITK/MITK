if(MITK_USE_Python3)
  mitkFunctionCheckCompilerFlags("/wd4273" CMAKE_CXX_FLAGS)

  if(CTKScriptingPythonCore_INCLUDE_DIRS AND CTKScriptingPythonWidgets_INCLUDE_DIRS)
    mitk_create_module(
      DEPENDS MitkCore MitkQtWidgets MitkPython
      PACKAGE_DEPENDS
      PUBLIC Qt6|Widgets CTK|CTKScriptingPythonCore+CTKScriptingPythonWidgets
    )
  endif()

  if(BUILD_TESTING AND TARGET ${MODULE_TARGET})
    add_subdirectory(Testing)
  endif()
endif()
