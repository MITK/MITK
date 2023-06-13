mitk_create_module(
  DEPENDS MitkCore
  TARGET_DEPENDS PRIVATE LZ4::lz4_shared
)

add_subdirectory(test)
