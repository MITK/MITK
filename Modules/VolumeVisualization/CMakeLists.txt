mitk_create_module(
  INCLUDE_DIRS
    PUBLIC include
    PRIVATE src
  DEPENDS MitkCore
  PACKAGE_DEPENDS
    PRIVATE nlohmann_json
)
