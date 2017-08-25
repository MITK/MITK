set(_options NO_INIT GCC_DEFAULT_VISIBILITY)
if(MITK_WIN32_FORCE_STATIC)
  list(APPEND _options FORCE_STATIC)
endif()

mitk_create_module(
  DEPENDS PUBLIC MitkIpPic
  ${_options}
  C_MODULE
  WARNINGS_NO_ERRORS
)

if(UNIX)
  target_link_libraries(${MODULE_TARGET} PRIVATE m)
endif(UNIX)
