MITK_CREATE_MODULE(
  DEPENDS MitkCore
    PRIVATE MitkDICOM
  PACKAGE_DEPENDS
    PRIVATE Poco nlohmann_json
)

add_subdirectory(autoload/IO)
add_subdirectory(test)
