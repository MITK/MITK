#mitkFunctionCheckCompilerFlags("-Wno-deprecated-declarations" CMAKE_CXX_FLAGS)

MITK_CREATE_MODULE(
  DEPENDS MitkCore MitkIpPic
  AUTOLOAD_WITH MitkCore
  WARNINGS_AS_ERRORS
  )

if(BUILD_TESTING)
  #add_subdirectory(Testing)
endif()
