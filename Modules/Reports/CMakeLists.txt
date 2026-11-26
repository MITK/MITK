mitk_create_module(
  DEPENDS
    PUBLIC MitkCore
  PACKAGE_DEPENDS
    PUBLIC Qt6|Core
)

if(TARGET ${MODULE_TARGET})
  if(BUILD_TESTING)
    add_subdirectory(test)
  endif()
endif()
