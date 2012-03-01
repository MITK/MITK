mitkFunctionCheckCompilerFlags("-Wno-deprecated-declarations" CMAKE_CXX_FLAGS)

MITK_CREATE_MODULE( IpPicSupport
  DEPENDS Mitk LegacyAdaptors mitkIpPic
  )

ADD_SUBDIRECTORY(Testing)
