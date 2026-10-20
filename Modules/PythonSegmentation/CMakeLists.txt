mitk_create_module(
  DEPENDS
    MitkPython
    MitkPythonHelper
    MitkPythonWorker
    MitkSegmentation
)

if(TARGET ${MODULE_TARGET})
  if(BUILD_TESTING)
    add_subdirectory(test)
  endif()
endif()
