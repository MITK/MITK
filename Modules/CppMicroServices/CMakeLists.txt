# Build the resource compiler first -- needed by all modules for resource embedding
add_subdirectory(tools)

mitk_create_module(
  DEPENDS PUBLIC MitkLog
  PACKAGE_DEPENDS
    PRIVATE nlohmann_json
  NO_INIT # CppMicroServices initializes itself in usModuleRegistry.cpp
)

if(TARGET ${MODULE_TARGET})
  target_compile_definitions(${MODULE_TARGET} PRIVATE
    US_MINIZ_NO_ARCHIVE_WRITING_API
    US_MINIZ_NO_ZLIB_COMPATIBLE_NAMES
  )

  # Unix needs dl for dlopen/dlsym
  if(UNIX)
    target_link_libraries(${MODULE_TARGET} PRIVATE ${CMAKE_DL_LIBS})
  endif()

  if(BUILD_TESTING)
    add_subdirectory(test)
  endif()
endif()
