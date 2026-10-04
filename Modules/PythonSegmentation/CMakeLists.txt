mitk_create_module(
  DEPENDS
    MitkPython
    MitkPythonHelper
    MitkSegmentation
)

if(TARGET ${MODULE_TARGET})
  if(BUILD_TESTING)
    add_subdirectory(test)
  endif()
endif()
