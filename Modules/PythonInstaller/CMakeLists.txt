if(MITK_USE_Python3)
  mitk_create_module(
    DEPENDS PUBLIC MitkPythonHelper
    PACKAGE_DEPENDS
      PUBLIC Qt6|Core
      PUBLIC nlohmann_json
    AUTOMOC
  )
endif()
