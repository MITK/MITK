MITK_CREATE_MODULE( ImageDenoising
  DEPENDS Mitk
  WARNINGS_AS_ERRORS
)

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif(BUILD_TESTING)
