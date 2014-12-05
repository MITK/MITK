MITK_CREATE_MODULE(DEPENDS MitkDataTypesExt MitkGLMappers
                   PACKAGE_DEPENDS ITK|ITKThresholding ANN
                   WARNINGS_AS_ERRORS
                  )

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
