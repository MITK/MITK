mitk_create_module(
  DEPENDS MitkCore
  PACKAGE_DEPENDS PRIVATE ITK|ZLIB
)

add_subdirectory(test)
