MITK_CREATE_MODULE(
  DEPENDS MitkCore
  PACKAGE_DEPENDS tinyxml2
  DEPRECATED_SINCE 2014.10
)

if(MODULE_IS_ENABLED)
  if(MSVC)
    target_compile_options(${MODULE_TARGET} PRIVATE /wd4996)
  else()
    target_compile_options(${MODULE_TARGET} PRIVATE -Wno-deprecated-declarations)
  endif()
endif()
