mitk_create_module(
  AUTOMOC
  DEPENDS
    PUBLIC MitkCore MitkRESTAPI
  PACKAGE_DEPENDS
    PUBLIC nlohmann_json Qt6|Core
)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()
