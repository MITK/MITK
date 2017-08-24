mitk_create_module(
  DEPENDS MitkCore
  PACKAGE_DEPENDS Redland
)

add_subdirectory(Testing)

if(MSVC AND TARGET ${MODULE_TARGET})
  set_property(TARGET ${MODULE_TARGET} APPEND_STRING PROPERTY COMPILE_FLAGS " /wd4251")
endif()
