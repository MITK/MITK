MITK_CREATE_MODULE(MitkAlgorithmsExt
                   DEPENDS MitkDataTypesExt
                   PACKAGE_DEPENDS ITK>ITKThresholding ANN
                   WARNINGS_AS_ERRORS
                  )

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
