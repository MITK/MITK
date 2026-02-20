mitk_create_module(
  DEPENDS PUBLIC MitkCore MitkSceneSerializationBase
  PACKAGE_DEPENDS
    PUBLIC nlohmann_json
    PRIVATE httplib
)

add_subdirectory(autoload/RESTAPIAutoload)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()
