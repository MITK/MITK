#mitkFunctionCheckCompilerFlags("-Wno-deprecated-declarations" CMAKE_CXX_FLAGS)

MITK_CREATE_MODULE( IpPicSupportIO
  DEPENDS Mitk mitkIpPic LegacyAdaptors
  AUTOLOAD_WITH Mitk
  )

if(BUILD_TESTING)
  #add_subdirectory(Testing)
endif()
