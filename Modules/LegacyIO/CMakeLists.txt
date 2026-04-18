MITK_CREATE_MODULE(
  DEPENDS MitkCore
  PACKAGE_DEPENDS tinyxml2
  DEPRECATED_SINCE 2014.10
)

if(MODULE_IS_ENABLED)
  # MitkLegacyIO itself is DEPRECATED_SINCE 2014.10 and intentionally keeps
  # using its own deprecated API internally. Silence the "deprecated" warnings
  # for this module only; the suppression stays until the module is removed.
  if(MSVC)
    target_compile_options(${MODULE_TARGET} PRIVATE /wd4996)
  else()
    target_compile_options(${MODULE_TARGET} PRIVATE -Wno-deprecated-declarations)
  endif()
endif()
