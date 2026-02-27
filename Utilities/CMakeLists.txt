# Suppress warnings for third-party code
if(MSVC)
  add_compile_options(/W0)
else()
  add_compile_options(-w)
endif()

set(MITK_MODULE_NAME_REGEX_MATCH )
set(MITK_MODULE_NAME_REGEX_NOT_MATCH )
set(MITK_MODULE_NAME_PREFIX )
add_subdirectory(qtsingleapplication)
