MITK_CREATE_MODULE(
  DEPENDS MitkCore
)

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif(BUILD_TESTING)
