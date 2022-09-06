if(MITK_USE_Python3)
  mitk_create_module(
    DEPENDS MitkCore
    PACKAGE_DEPENDS PUBLIC Python3|Python
    )

  add_subdirectory(autoload/PythonService)
endif()
