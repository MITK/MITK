mitk_create_module(
  DEPENDS
    PUBLIC MitkRESTAPI MitkSegmentation
  PACKAGE_DEPENDS
    PUBLIC Qt6|Core nlohmann_json
)

if(TARGET ${MODULE_TARGET})
  if(BUILD_TESTING)
    add_subdirectory(test)
  endif()
endif()
