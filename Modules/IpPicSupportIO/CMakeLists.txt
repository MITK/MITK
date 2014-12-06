#mitkFunctionCheckCompilerFlags("-Wno-deprecated-declarations" CMAKE_CXX_FLAGS)

MITK_CREATE_MODULE(
  DEPENDS MitkIpPic MitkLegacyAdaptors MitkLegacyIO
  AUTOLOAD_WITH MitkCore
  )

if(BUILD_TESTING)
  #add_subdirectory(Testing)
endif()
