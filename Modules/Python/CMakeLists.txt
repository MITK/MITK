mitk_create_module(
  DEPENDS MitkPythonHelper
  PACKAGE_DEPENDS PRIVATE Python3|Python
  TARGET_DEPENDS PRIVATE pybind11::embed
)

if(TARGET ${MODULE_TARGET})
  if(BUILD_TESTING)
    add_subdirectory(test)
  endif()
endif()
