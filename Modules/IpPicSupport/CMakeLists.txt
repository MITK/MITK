mitkFunctionCheckCompilerFlags("-Wno-deprecated-declarations" CMAKE_CXX_FLAGS)

MITK_CREATE_MODULE(
  DEPENDS MitkLegacyAdaptors MitkLegacyIO MitkIpPic
  DEPRECATED_SINCE 2013.12
  )

add_subdirectory(Testing)
