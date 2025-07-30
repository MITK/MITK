mitk_create_module(
  DEPENDS MitkPythonHelper
  PACKAGE_DEPENDS PUBLIC Python3|Python
)

if(TARGET ${MODULE_TARGET})
  if(BUILD_TESTING)
    add_subdirectory(test)
  endif()
endif()
