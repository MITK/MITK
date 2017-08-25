set(_options NO_INIT GCC_DEFAULT_VISIBILITY)
if(MITK_WIN32_FORCE_STATIC)
  list(APPEND _options FORCE_STATIC)
endif()

set(_additional_libs)
if(UNIX)
  list(APPEND _additional_libs m)
endif(UNIX)

mitk_create_module(
  DEPENDS PUBLIC MitkIpPic
  ${_options}
  C_MODULE
  WARNINGS_NO_ERRORS
)

