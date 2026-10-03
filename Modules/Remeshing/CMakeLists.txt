mitk_create_module(
  DEPENDS PUBLIC MitkCore
  PACKAGE_DEPENDS ACVD|Surface
)

if(TARGET ${MODULE_TARGET})
  # ACVD's clustering is header-only and picks its parallel implementation
  # at compile time. Every translation unit of this module that includes
  # ACVD has to agree on it, so it is set here rather than in a source file.
  target_compile_definitions(${MODULE_TARGET} PRIVATE DOmultithread)
endif()

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
