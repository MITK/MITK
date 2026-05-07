mitk_create_module(
  DEPENDS
    PUBLIC MitkCore MitkSceneSerializationBase
    PRIVATE MitkQtWidgets #[[ Virtual dependency, i.e., we only require it here
                              as we auto-load MitkRESTAPIAutoload with
                              MitkQtWidgets. ]]
  PACKAGE_DEPENDS
    PUBLIC nlohmann_json
    PRIVATE httplib OpenSSL
)

add_subdirectory(autoload/RESTAPIAutoload)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()
