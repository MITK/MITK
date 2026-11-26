set(MITK_REST_PYTHON_DEPENDS)

if(TARGET MitkPythonSegmentation)
  set(MITK_REST_PYTHON_DEPENDS MitkPythonSegmentation)
endif()

mitk_create_module(
  DEPENDS
    PUBLIC MitkRESTAPI MitkSegmentation ${MITK_REST_PYTHON_DEPENDS}
  PACKAGE_DEPENDS
    PUBLIC Qt6|Core nlohmann_json
)

if(TARGET ${MODULE_TARGET})
  if(BUILD_TESTING)
    add_subdirectory(test)
  endif()
endif()
