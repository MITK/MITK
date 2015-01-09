MITK_CREATE_MODULE(DEPENDS MitkDataTypesExt
                   PACKAGE_DEPENDS
                     PUBLIC ITK|ITKThresholding
                     PRIVATE ANN
                   WARNINGS_AS_ERRORS
                  )

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
