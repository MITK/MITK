
set(_options NO_INIT GCC_DEFAULT_VISIBILITY)
if(MITK_WIN32_FORCE_STATIC)
  list(APPEND _options FORCE_STATIC)
endif()

set(_additional_libs)
if(USE_ITKZLIB)
  list(APPEND _additional_libs itkzlib)
else()
  list(APPEND _additional_libs z)
endif(USE_ITKZLIB)

mitk_create_module(
  ADDITIONAL_LIBS ${_additional_libs}
  ${_options}
  C_MODULE
)

target_compile_definitions(${MODULE_TARGET} PRIVATE -DMITK_IPPIC_COMPILE)

